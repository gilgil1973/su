// ----------------------------------------------------------------------------
//
// site unblocker version 1.4
//
// http://www.gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#ifndef __WEB_TEST_H__
#define __WEB_TEST_H__

#include <VHttpCommon>
#include "changehttprequest.h"

// ----------------------------------------------------------------------------
// WebTest
// ----------------------------------------------------------------------------
class WebTest
{
public:
  static const int RESULT_NONE              = 0;
  static const int RESULT_CANNOT_CONNECT    = 1;
  static const int RESULT_NO_RESPONSE       = 2;
  static const int RESULT_PARSE_ERROR       = 3;

  static const int RESULT_OK                = 200;
  static const int RESULT_MOVED_PERMANENTLY = 301;
  static const int RESULT_REDIRECT          = 302; // warning.or.kr // Redirect, Found
  static const int RESULT_BAD_REQUEST       = 400;
  static const int RESULT_NOT_FOUND         = 404;

public:
  WebTest();

public:
  QString host;
  int     port;

public:
  void test();
  HttpRequestChangePolicy bestPolicy();

public:
  int resultNone;
  int resultAddLine;
  int resultAddSpace;
  int resultDummyHost;
  int resultSslAbsPath;
};

#endif // __WEB_TEST_H__
