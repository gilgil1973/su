// ----------------------------------------------------------------------------
//
// site unblocker version 1.1
//
// http://www.gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#ifndef __CHANGE_HTTP_REQUEST_H__
#define __CHANGE_HTTP_REQUEST_H__

#include <VHttpRequest>
#include <VObject>

// ----------------------------------------------------------------------------
// HttpRequestChangePolicy
// ----------------------------------------------------------------------------
typedef enum
{
  ChangeNone      = 0,
  ChangeAddLine   = 1,
  ChangeAddSpace  = 2,
  ChangeDummyHost = 3,
  ChangeSslAbsPath = 4,
} HttpRequestChangePolicy;

// ----------------------------------------------------------------------------
// ChangeHttpRequest
// ----------------------------------------------------------------------------
class ChangeHttpRequest : public VObject
{
public:
  virtual void change(VHttpRequest& request)
  {
    Q_UNUSED(request)
  }
};

// ----------------------------------------------------------------------------
// ChangeHttpRequestAddLine
// ----------------------------------------------------------------------------
class ChangeHttpRequestAddLine : public ChangeHttpRequest
{
public:
  virtual void change(VHttpRequest& request);
};

// ----------------------------------------------------------------------------
// ChangeHttpRequestAddSpace
// ----------------------------------------------------------------------------
class ChangeHttpRequestAddSpace : public ChangeHttpRequest
{
public:
  virtual void change(VHttpRequest& request);
};

// ----------------------------------------------------------------------------
// ChangeHttpRequestDummyHost
// ----------------------------------------------------------------------------
class ChangeHttpRequestDummyHost : public ChangeHttpRequest
{
public:
  virtual void change(VHttpRequest& request);
};

// ----------------------------------------------------------------------------
// ChangeHttpRequestSslAbsPath
// ----------------------------------------------------------------------------
class ChangeHttpRequestSslAbsPath : public ChangeHttpRequest
{
public:
  virtual void change(VHttpRequest& request);
};

#endif // __CHANGE_HTTP_REQUEST_H__
