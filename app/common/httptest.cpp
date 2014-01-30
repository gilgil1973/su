#include "httptest.h"
#include "changehttprequest.h"
#include <VHttpRequest>
#include <VHttpResponse>

#include <VThread>
#include <VTcpClient>
#include <VDebugNew>

// ----------------------------------------------------------------------------
// HttpTestThread
// ----------------------------------------------------------------------------
class HttpTestThread : public VThread
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
  HttpTestThread() : VThread(NULL)
  {
    host   = "";
    port   = 0;
    result = NULL;
    change = NULL;
  }

  virtual ~HttpTestThread()
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
      LOG_INFO("%s can not connect to %s:%d", change->className().toLatin1().data(), host.toLatin1().data(), port);
      *result = HttpTest::RESULT_CANNOT_CONNECT;
      return;
    }

    //
    // make http request msg
    //
    tag = 3000; // gilgil temp
    VHttpRequest request;
    request.requestLine.method  = "GET";
    request.requestLine.path    = "/";
    request.requestLine.version = "HTTP/1.0";
    QByteArray hostValue = host.toLatin1().data();
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
      if (response.parse(msg)) break;
    }
    tag = 6000; // gilgil temp

    if (msg == "")
    {
      LOG_INFO("%s no http response message", change->className().toLatin1().data());
      *result = HttpTest::RESULT_NO_RESPONSE;
      return;
    }
    tag = 7000; // gilgil temp

    if (!response.parse(msg))
    {
      if (msg.size() > 256) msg.resize(256);
      LOG_INFO("%s response.parse return false %s", change->className().toLatin1().data(), msg.data());
      *result = HttpTest::RESULT_PARSE_ERROR;
      return;
    }
    tag = 8000; // gilgil temp

    LOG_INFO("%s response = %d %s", change->className().toLatin1().data(), response.statusLine.code, response.statusLine.text.data());
    *result = response.statusLine.code;
    tag = 9000; // gilgil temp
  }
};

// ----------------------------------------------------------------------------
// HttpTest
// ----------------------------------------------------------------------------
HttpTest::HttpTest()
{
  host = "";
  port = 0;

  resultNone      = RESULT_NONE;
  resultAddLine   = RESULT_NONE;
  resultAddSpace  = RESULT_NONE;
  resultDummyHost = RESULT_NONE;
  resultSslAbsPath = RESULT_NONE;
}

void HttpTest::test()
{
  //
  // normal
  //
  HttpTestThread noneThread;
  noneThread.host   = host;
  noneThread.port   = port;
  noneThread.result = &resultNone;
  noneThread.change = new ChangeHttpRequest;
  noneThread.open();


  //
  // addLine
  //
  HttpTestThread addLineThread;
  addLineThread.host   = host;
  addLineThread.port   = port;
  addLineThread.result = &resultAddLine;
  addLineThread.change = new ChangeHttpRequestAddLine;
  addLineThread.open();

  //
  // addSpace
  //
  HttpTestThread addSpaceThread;
  addSpaceThread.host   = host;
  addSpaceThread.port   = port;
  addSpaceThread.result = &resultAddSpace;
  addSpaceThread.change = new ChangeHttpRequestAddSpace;
  addSpaceThread.open();

  //
  // dummyHost
  //
  HttpTestThread dummyHostThread;
  dummyHostThread.host   = host;
  dummyHostThread.port   = port;
  dummyHostThread.result = &resultDummyHost;
  dummyHostThread.change = new ChangeHttpRequestDummyHost;
  dummyHostThread.open();

  //
  // sslAbsPath
  //
  HttpTestThread sslAbsPath;
  sslAbsPath.host = host;
  sslAbsPath.port = port;
  sslAbsPath.result = &resultSslAbsPath;
  sslAbsPath.change = new ChangeHttpRequestSslAbsPath;
  sslAbsPath.open();

  //
  // close
  //
  noneThread.wait(20000);      noneThread.close();
  addLineThread.wait(20000);   addLineThread.close();
  addSpaceThread.wait(20000);  addSpaceThread.close();
  dummyHostThread.wait(20000); dummyHostThread.close();
}

HttpRequestChangePolicy HttpTest::bestPolicy()
{
  // if (resultNone      == RESULT_OK) return ChangeNone;
  if (resultAddLine   >= RESULT_OK && resultAddLine  != RESULT_BAD_REQUEST) return ChangeAddLine;
  if (resultAddSpace  >= RESULT_OK && resultAddSpace != RESULT_BAD_REQUEST) return ChangeAddSpace;
  if (resultDummyHost == RESULT_OK) return ChangeDummyHost;
  LOG_ERROR("no appropriate policy for %s:%d", host.toLatin1().data(), port);
  return ChangeNone;
}
