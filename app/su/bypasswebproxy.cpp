#include "bypasswebproxy.h"

#include "../common/webtest.h"
#include <VDebugNew>

// ----------------------------------------------------------------------------
// TestThread
// ----------------------------------------------------------------------------
class TestThread : public VThread
{
public:
  WebTest webTest;

public:
  TestThread(void* owner, QString host, int port) : VThread(owner)
  {
    webTest.host = host;
    webTest.port = port;
  }

  virtual ~TestThread()
  {
    close();
  }

protected:
  virtual void run()
  {
    webTest.test();
    HttpRequestChangePolicy policy = webTest.bestPolicy();
    LOG_INFO("--------------------------------");
    LOG_INFO("policy for (%s:%d) is %d", webTest.host.toLatin1().data(), webTest.port, (int)policy);
    LOG_INFO("--------------------------------");
    HostMgr::Key key;     key.host = webTest.host; key.port = webTest.port;
    HostMgr::Value value; value.policy = policy;

    BypassWebProxy* proxy = (BypassWebProxy*)owner;
    proxy->hostMgr.lock();
    proxy->hostMgr.items.insert(key, value);
    proxy->hostMgr.unlock();

    emit proxy->newHostDetected(key, value);
  }
};

// ----------------------------------------------------------------------------
// BypassWebProxy
// ----------------------------------------------------------------------------
BypassWebProxy::BypassWebProxy(void* owner) : VWebProxy(owner)
{
  httpsEnabled        = false;
  maxContentCacheSize = 65536;

  blockMsg =
    "HTTP/1.0 302 Redirect\r\n"
    "Location: http://www.warning.or.kr";

  defaultPolicy = ChangeNone;

  {
    HostMgr::Key key; HostMgr::Value value;

    key.host = "grooveshark.com"; key.port = 80; value.policy = ChangeAddLine;
    hostMgr.items.insert(key, value);

    key.host = "www.grooveshark.com"; key.port = 80; value.policy = ChangeAddLine;
    hostMgr.items.insert(key, value);
  }

  VObject::connect(
    this, SIGNAL(onHttpRequestHeader(VHttpRequest*, VWebProxyConnection*)),
    this, SLOT(onMyHttpRequestHeader(VHttpRequest*, VWebProxyConnection*)), Qt::DirectConnection);

  VObject::connect(
    this, SIGNAL(onHttpResponseHeader(VHttpResponse*, VWebProxyConnection*)),
    this, SLOT(onMyHttpResponseHeader(VHttpResponse*, VWebProxyConnection*)), Qt::DirectConnection);
}

BypassWebProxy::~BypassWebProxy()
{
  close();
}

bool BypassWebProxy::doOpen()
{
  if (!enableProxy()) return false;
  return VWebProxy::doOpen();
}

bool BypassWebProxy::doClose()
{
  if (!disableProxy()) return false;
  return VWebProxy::doClose();
}

#include <Wininet.h>

bool BypassWebProxy::enableProxy()
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

bool BypassWebProxy::disableProxy()
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

void BypassWebProxy::load(VXml xml)
{
  VWebProxy::load(xml);

  blockMsg = qPrintable(xml.getStr("blockMsg", blockMsg));
  defaultPolicy = (HttpRequestChangePolicy)xml.getInt("defaultPolicy", (int)defaultPolicy);
  hostMgr.load(xml.gotoChild("hostMgr"));
}

void BypassWebProxy::save(VXml xml)
{
  VWebProxy::save(xml);

  xml.setStr("blockMsg", blockMsg);
  xml.setInt("defaultPolicy", (int)defaultPolicy);
  hostMgr.save(xml.gotoChild("hostMgr"));
}

void BypassWebProxy::onMyHttpRequestHeader(VHttpRequest*  request,  VWebProxyConnection* connection)
{
  QString host = connection->outClient->host;
  int     port = connection->outClient->port;

  HttpRequestChangePolicy policy = defaultPolicy;

  {
    VLock lock(hostMgr);
    HostMgr::Key key; key.host = host; key.port = port;
    HostMgr::HostItemMap::iterator it = hostMgr.items.find(key);
    if (it != hostMgr.items.end()) policy = it.value().policy;
  }

  switch (policy)
  {
    case ChangeNone: break;
    case ChangeAddLine:
    {
      ChangeHttpRequestAddLine change;
      change.change(*request);
      break;
    }
    case ChangeAddSpace:
    {
      ChangeHttpRequestAddSpace change;
      change.change(*request);
      break;
    }
    case ChangeDummyHost:
    {
      ChangeHttpRequestDummyHost change;
      change.change(*request);
      break;
    }
    case ChangeSslAbsPath:
    {
      ChangeHttpRequestSslAbsPath change;
      change.change(*request);
      break;
    }
    default:
    {
      LOG_FATAL("invalid policy(%d)", (int)policy);
      break;
    }
  }
  LOG_DEBUG("%s:%d", qPrintable(connection->outClient->host), connection->outClient->port);
}

void BypassWebProxy::onMyHttpResponseHeader(VHttpResponse* response, VWebProxyConnection* connection)
{
  QByteArray msg = response->toByteArray();
  if (!msg.startsWith(blockMsg)) return;
  QString transHost = connection->outClient->host + ":" + QString::number(connection->outClient->port);
  LOG_INFO("-------------------------------------------------")
  LOG_INFO("blocked(%s)!!!", transHost.toLatin1().data());
  LOG_INFO("-------------------------------------------------");

  QString host = connection->outClient->host;
  int     port = connection->outClient->port;

  HostMgr::Key key; key.host = host; key.port = port;
  if (hostMgr.items.find(key) != hostMgr.items.end()) return;

  TestThread* testThread = new TestThread(this, connection->outClient->host, connection->outClient->port);
  testThread->freeOnTerminate = true;
  testThread->open();
}
