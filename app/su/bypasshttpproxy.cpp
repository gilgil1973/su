#include "bypasshttpproxy.h"

#include "../common/httptest.h"
#include <VDebugNew>

// ----------------------------------------------------------------------------
// TestThread
// ----------------------------------------------------------------------------
class TestThread : public VThread
{
public:
  HttpTest httpTest;

public:
  TestThread(void* owner, QString host, int port) : VThread(owner)
  {
    httpTest.host = host;
    httpTest.port = port;
  }

  virtual ~TestThread()
  {
    close();
  }

protected:
  virtual void run()
  {
    httpTest.test();
    HttpRequestChangePolicy policy = httpTest.bestPolicy();
    LOG_INFO("--------------------------------");
    LOG_INFO("policy for (%s:%d) is %d", httpTest.host.toLatin1().data(), httpTest.port, (int)policy);
    LOG_INFO("--------------------------------");
    HostMgr::Key key;     key.host = httpTest.host; key.port = httpTest.port;
    HostMgr::Value value; value.policy = policy;

    BypassHttpProxy* proxy = (BypassHttpProxy*)owner;
    proxy->hostMgr.lock();
    proxy->hostMgr.items.insert(key, value);
    proxy->hostMgr.unlock();

    emit proxy->newHostDetected(key, value);
  }
};

// ----------------------------------------------------------------------------
// BypassHttpProxy
// ----------------------------------------------------------------------------
BypassHttpProxy::BypassHttpProxy(void* owner) : VHttpProxy(owner)
{
  blockMsg =
    "HTTP/1.0 302 Redirect\r\n"
    "Location: http://www.warning.or.kr";

  HostMgr::Key key; HostMgr::Value value;

  key.host = "grooveshark.com"; key.port = 80; value.policy = ChangeAddLine;
  hostMgr.items.insert(key, value);

  key.host = "www.grooveshark.com"; key.port = 80; value.policy = ChangeAddLine;
  hostMgr.items.insert(key, value);

  QObject::connect(
    this, SIGNAL(beforeRequest(VHttpRequest&,VTcpSession*,VTcpClient*)),
    this, SLOT(myBeforeRequest(VHttpRequest&,VTcpSession*,VTcpClient*)), Qt::DirectConnection);
  QObject::connect(
    this, SIGNAL(beforeResponse(QByteArray&,VTcpClient*,VTcpSession*)),
    this, SLOT(myBeforeResponse(QByteArray&,VTcpClient*,VTcpSession*)), Qt::DirectConnection);
}

BypassHttpProxy::~BypassHttpProxy()
{
  close();
}

bool BypassHttpProxy::doOpen()
{
  if (!enableProxy()) return false;
  return VHttpProxy::doOpen();
}

bool BypassHttpProxy::doClose()
{
  if (!disableProxy()) return false;
  return VHttpProxy::doClose();
}

#include <Wininet.h>

bool BypassHttpProxy::enableProxy()
{
  INTERNET_PER_CONN_OPTION_LIST list;
  BOOL    bReturn;
  DWORD   dwBufSize = sizeof(list);

  list.dwSize = sizeof(list);
  list.pszConnection = NULL;
  list.dwOptionCount = 3;
  list.pOptions = new INTERNET_PER_CONN_OPTION[3];
  if(NULL == list.pOptions)
  {
    LOG_ERROR("failed to allocat memory in SetConnectionOptions");
    return false;
  }
  // Set proxy name.
  list.pOptions[0].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
  list.pOptions[0].Value.pszValue = L"http=http://127.0.0.1:8080";

  // Set flags.
  list.pOptions[1].dwOption = INTERNET_PER_CONN_FLAGS;
  list.pOptions[1].Value.dwValue = PROXY_TYPE_DIRECT | PROXY_TYPE_PROXY;

  // Set proxy override.
  list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
  list.pOptions[2].Value.pszValue = L"<local>";

  // Set the options on the connection.
  bReturn = InternetSetOption(NULL,
      INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);

  // Free the allocated memory.
  delete [] list.pOptions;
  InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
  InternetSetOption(NULL, INTERNET_OPTION_REFRESH , NULL, 0);

  LOG_DEBUG("bReturn=%d", bReturn);
  return (bool)bReturn;
  }

bool BypassHttpProxy::disableProxy()
{
  //conn_name: active connection name.
  INTERNET_PER_CONN_OPTION_LIST list;
  BOOL    bReturn;
  DWORD   dwBufSize = sizeof(list);
  // Fill out list struct.
  list.dwSize = sizeof(list);
  // NULL == LAN, otherwise connectoid name.
  list.pszConnection = NULL;
  // Set three options.
  list.dwOptionCount = 1;
  list.pOptions = new INTERNET_PER_CONN_OPTION[list.dwOptionCount];
  // Make sure the memory was allocated.
  if(NULL == list.pOptions)
  {
    // Return FALSE if the memory wasn't allocated.
    LOG_ERROR("failed to allocat memory in DisableConnectionProxy()");
    return false;
  }
  // Set flags.
  list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
  list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT  ;
  // Set the options on the connection.
  bReturn = InternetSetOption(NULL,
      INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);
  // Free the allocated memory.
  delete [] list.pOptions;
  InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
  InternetSetOption(NULL, INTERNET_OPTION_REFRESH , NULL, 0);

  LOG_DEBUG("bReturn=%d", bReturn);
  return bReturn;
}

void BypassHttpProxy::load(VXml xml)
{
  VHttpProxy::load(xml);
  blockMsg = xml.getStr("blockMsg", blockMsg).toLatin1();
  hostMgr.load(xml.gotoChild("hostMgr"));
}

void BypassHttpProxy::save(VXml xml)
{
  VHttpProxy::save(xml);
  xml.setStr("blockMsg", blockMsg);
  hostMgr.save(xml.gotoChild("hostMgr"));
}

void BypassHttpProxy::myBeforeRequest(VHttpRequest& request, VTcpSession* inSession, VTcpClient* outClient)
{
  Q_UNUSED(inSession)
  QString host = outClient->host;
  int     port = outClient->port;
  {
    VLock lock(hostMgr);
    HostMgr::Key key; key.host = host; key.port = port;
    HostMgr::HostItemMap::iterator it = hostMgr.items.find(key);
    if (it != hostMgr.items.end())
    {
      switch (it.value().policy)
      {
        case ChangeNone: break;
        case ChangeAddLine:
        {
          ChangeHttpRequestAddLine change;
          change.change(request);
          break;
        }
        case ChangeAddSpace:
        {
          ChangeHttpRequestAddSpace change;
          change.change(request);
          break;
        }
        case ChangeDummyHost:
        {
          ChangeHttpRequestDummyHost change;
          change.change(request);
          break;
        }
      }
    }
  }
  LOG_DEBUG("%s:%d", outClient->host.toLatin1().data(), outClient->port);
  //request.requestLine.method = "\r\n" + request.requestLine.method;
}

void BypassHttpProxy::myBeforeResponse(QByteArray& msg, VTcpClient* outClient, VTcpSession* inSession)
{
  Q_UNUSED(inSession)
  if (!msg.startsWith(blockMsg)) return;
  QString transHost = outClient->host + ":" + QString::number(outClient->port);
  LOG_INFO("-------------------------------------------------")
  LOG_INFO("blocked(%s)!!!", transHost.toLatin1().data());
  LOG_INFO("-------------------------------------------------");

  QString host = outClient->host;
  int     port = outClient->port;

  HostMgr::Key key; key.host = host; key.port = port;
  if (hostMgr.items.find(key) != hostMgr.items.end()) return;

  TestThread* testThread = new TestThread(this, outClient->host, outClient->port);
  testThread->freeOnTerminate = true;
  testThread->open();
}
