// ----------------------------------------------------------------------------
//
// site unblocker version 1.4
//
// http://www.gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#ifndef __BYPASS_HTTP_PROXY_H__
#define __BYPASS_HTTP_PROXY_H__

#include <VWebProxy>
#include "hostmgr.h"

// ----------------------------------------------------------------------------
// BypassWebProxy
// ----------------------------------------------------------------------------
class BypassWebProxy : public VWebProxy
{
  Q_OBJECT

public:
  BypassWebProxy(void* owner = NULL);
  virtual ~BypassWebProxy();

protected:
  virtual bool doOpen();
  virtual bool doClose();
  bool enableProxy();
  bool disableProxy();

public:
  QByteArray              blockMsg;
  HttpRequestChangePolicy defaultPolicy;
  HostMgr                 hostMgr;

public:
  virtual void load(VXml xml);
  virtual void save(VXml xml);

signals:
  void newHostDetected(HostMgr::Key key, HostMgr::Value value);

public slots:
  void onMyHttpRequestHeader(VHttpRequest*  request,  VWebProxyConnection* connection);
  void onMyHttpResponseHeader(VHttpResponse* response, VWebProxyConnection* connection);
};

#endif // __BYPASS_HTTP_PROXY_H__

