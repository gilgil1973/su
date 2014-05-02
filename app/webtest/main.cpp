#include <stdio.h>

#include <VApp>
#include <QCoreApplication>
#include <QDebug>
#include "../common/webtest.h"

using namespace std;

class Param
{
public:
  QString host;
  int     port;

public:
  Param()
  {
    host = "";
    port = 80;
  }

  bool parse(int argc, char* argv[])
  {
    if (argc > 1) host = argv[1];
    if (argc > 2) port = atoi(argv[2]);

    if (host == "") return false;
    return true;
  }

  static void usage()
  {
    printf("http test version 8.0\n");
    printf("Copyright (c) Gilbert Lee All rights reserved\n");
    printf("\n");
    printf("httptest <host> [<port>]\n");
    printf("\n");
    printf("example\n");
    printf("\n");
    printf("  httptest www.gilgil.net\n");
    printf("  httptest www.google.com 8080\n");
  }
};

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  VApp::initialize(true, false, "stdout");

  Param param;
  if (!param.parse(argc, argv))
  {
    Param::usage();
    return -1;
  }

  printf("testing   %s:%d\n", param.host.toLatin1().data(), param.port);
  LOG_INFO("testing   %s:%d", param.host.toLatin1().data(), param.port);
  WebTest webTest;
  webTest.host = param.host;
  webTest.port = param.port;
  webTest.test();

  printf("0:none       %d\n", webTest.resultNone);
  printf("1:addLine    %d\n", webTest.resultAddLine);
  printf("2:addSpace   %d\n", webTest.resultAddSpace);
  printf("3:dummyHost  %d\n", webTest.resultDummyHost);
  printf("4:sslAbsPath %d\n", webTest.resultSslAbsPath);

  printf("best policy= %d\n", (int)webTest.bestPolicy());

  return 0;
}
