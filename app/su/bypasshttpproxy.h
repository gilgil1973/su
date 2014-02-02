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

#include <VHttpProxy>
#include "hostmgr.h"

// ----------------------------------------------------------------------------
// BypassHttpProxy
// ----------------------------------------------------------------------------
class BypassHttpProxy : public VHttpProxy
{
  Q_OBJECT

public:
  BypassHttpProxy(void* owner = NULL);
  virtual ~BypassHttpProxy();

protected:
  virtual bool doOpen();
  virtual bool doClose();
  bool enableProxy();
  bool disableProxy();

public:
  HostMgr hostMgr;

public:
  QByteArray blockMsg;

public:
  virtual void load(VXml xml);
  virtual void save(VXml xml);

signals:
  void newHostDetected(HostMgr::Key key, HostMgr::Value value);

public slots:
  void myBeforeRequest(VHttpRequest& request, VTcpSession* fromSession, VTcpClient* toClient);
  void myBeforeResponse(QByteArray& msg, VTcpClient* fromClient, VTcpSession* toSession);
};

#endif // __BYPASS_HTTP_PROXY_H__

