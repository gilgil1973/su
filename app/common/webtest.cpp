#include "webtest.h"
#include "changehttprequest.h"
#include <VHttpRequest>
#include <VHttpResponse>

#include <VThread>
#include <VTcpClient>
#include <VDebugNew>

// ----------------------------------------------------------------------------
// WebTestThread
// ----------------------------------------------------------------------------
class WebTestThread : public VThread
{
public:
  QString host;
  int     port;
  int*    result;

public:
  ChangeHttpRequest* change;

protected:
  VTcpClient tcpClient;

public:
  WebTestThread() : VThread(NULL)
  {
    host   = "";
    port   = 0;
    result = NULL;
    change = NULL;
  }

  virtual ~WebTestThread()
  {
    close();
    SAFE_DELETE(change);
  }

  virtual bool close()
  {
    return VThread::close();
  }

protected:
  virtual void run()
  {
    tag = 1000; // gilgil temp
    tcpClient.host = host;
    tcpClient.port = port;

    //
    // connect
    //
    tag = 2000; // gilgil temp
    if (!tcpClient.open())
    {
      LOG_INFO("%s can not connect to %s:%d", qPrintable(change->className()), qPrintable(host), port);
      *result = WebTest::RESULT_CANNOT_CONNECT;
      return;
    }

    //
    // make http request msg
    //
    tag = 3000; // gilgil temp
    VHttpRequest request;
    request.requestLine.method  = "GET";
    request.requestLine.path    = "/";
    request.requestLine.version = "HTTP/1.1";
    QByteArray hostValue = qPrintable(host);
    if (port != 80) hostValue += ":" + QByteArray::number(port);
    request.header.setValue("Host", hostValue);
    request.header.setValue("User-Agent", "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)");
    if (change != NULL)
      change->change(request);
    QByteArray msg = request.toByteArray();
    tag = 4000; // gilgil temp

    //
    // send http request msg
    //
    tcpClient.tcpSession->write(msg);
    tag = 5000; // gilgil temp

    //
    // receive http response msg
    //
    msg = "";
    VHttpResponse response;
    while (true)
    {
      QByteArray oneMsg;
      int readLen = tcpClient.tcpSession->read(oneMsg);
      if (readLen == VERR_FAIL) break;
      msg += oneMsg;
      {
        QByteArray tempMsg = msg;
        if (response.parse(tempMsg)) break;
      }
    }
    tag = 6000; // gilgil temp

    if (msg == "")
    {
      LOG_INFO("%s no http response message", qPrintable(change->className()));
      *result = WebTest::RESULT_NO_RESPONSE;
      return;
    }
    tag = 7000; // gilgil temp

    if (!response.parse(msg))
    {
      if (msg.size() > 256) msg.resize(256);
      LOG_INFO("%s response.parse return false %s", qPrintable(change->className()), msg.data());
      *result = WebTest::RESULT_PARSE_ERROR;
      return;
    }
    tag = 8000; // gilgil temp

    LOG_INFO("%s response = %d %s", qPrintable(change->className()), response.statusLine.code, response.statusLine.text.data());
    *result = response.statusLine.code;
    tag = 9000; // gilgil temp
  }
};

// ----------------------------------------------------------------------------
// WebTest
// ----------------------------------------------------------------------------
WebTest::WebTest()
{
  host = "";
  port = 0;

  resultNone       = RESULT_NONE;
  resultAddLine    = RESULT_NONE;
  resultAddSpace   = RESULT_NONE;
  resultDummyHost  = RESULT_NONE;
  resultSslAbsPath = RESULT_NONE;
}

void WebTest::test()
{
  //
  // normal
  //
  WebTestThread noneThread;
  noneThread.host   = host;
  noneThread.port   = port;
  noneThread.result = &resultNone;
  noneThread.change = new ChangeHttpRequest;
  noneThread.open();


  //
  // addLine
  //
  WebTestThread addLineThread;
  addLineThread.host   = host;
  addLineThread.port   = port;
  addLineThread.result = &resultAddLine;
  addLineThread.change = new ChangeHttpRequestAddLine;
  addLineThread.open();

  //
  // addSpace
  //
  WebTestThread addSpaceThread;
  addSpaceThread.host   = host;
  addSpaceThread.port   = port;
  addSpaceThread.result = &resultAddSpace;
  addSpaceThread.change = new ChangeHttpRequestAddSpace;
  addSpaceThread.open();

  //
  // dummyHost
  //
  WebTestThread dummyHostThread;
  dummyHostThread.host   = host;
  dummyHostThread.port   = port;
  dummyHostThread.result = &resultDummyHost;
  dummyHostThread.change = new ChangeHttpRequestDummyHost;
  dummyHostThread.open();

  //
  // sslAbsPath
  //
  WebTestThread sslAbsPathThread;
  sslAbsPathThread.host = host;
  sslAbsPathThread.port = port;
  sslAbsPathThread.result = &resultSslAbsPath;
  sslAbsPathThread.change = new ChangeHttpRequestSslAbsPath;
  sslAbsPathThread.open();

  //
  // close
  //
  noneThread.wait(20000);       noneThread.close();
  addLineThread.wait(20000);    addLineThread.close();
  addSpaceThread.wait(20000);   addSpaceThread.close();
  dummyHostThread.wait(20000);  dummyHostThread.close();
  sslAbsPathThread.wait(20000); sslAbsPathThread.close();
}

HttpRequestChangePolicy WebTest::bestPolicy()
{
  // if (resultNone      == RESULT_OK) return ChangeNone;
  if (resultAddLine   >= RESULT_OK && resultAddLine  != RESULT_BAD_REQUEST) return ChangeAddLine;
  if (resultAddLine   >= RESULT_OK && resultAddLine  != RESULT_BAD_REQUEST) return ChangeAddLine;
  if (resultSslAbsPath   >= RESULT_OK && resultSslAbsPath  != RESULT_BAD_REQUEST) return ChangeSslAbsPath;
  if (resultAddSpace  >= RESULT_OK && resultAddSpace != RESULT_BAD_REQUEST) return ChangeAddSpace;
  if (resultDummyHost == RESULT_OK) return ChangeDummyHost;
  LOG_ERROR("no appropriate policy for %s:%d", qPrintable(host), port);
  return ChangeNone;
}
