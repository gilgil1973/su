// ----------------------------------------------------------------------------
//
// site unblocker version 1.4
//
// http://www.gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#ifndef __HOST_MGR_H__
#define __HOST_MGR_H__

#include "../common/changehttprequest.h"

// ----------------------------------------------------------------------------
// HostMgr
// ----------------------------------------------------------------------------
class HostMgr : public VLockable, public VXmlable
{
public:
  class Key
  {
  public:
    QString host;
    int     port;

  public:
    bool operator < (const Key& other) const
    {
      if (host < other.host) return true;
      if (host > other.host) return false;
      return port < other.port;
    }
  };

  class Value
  {
  public:
    HttpRequestChangePolicy policy;
  };

  typedef QMap<Key, Value> HostItemMap;
  HostItemMap items;

public:
  HostMgr();
  virtual ~HostMgr();

public:
  void clear();

public:
  virtual void load(VXml xml);
  virtual void save(VXml xml);
};

// ----------------------------------------------------------------------------
// HostMgrEvent
// ----------------------------------------------------------------------------
class HostMgrEvent : public QEvent
{
public:
  HostMgr::Key   key;
  HostMgr::Value value;
public:
  HostMgrEvent(HostMgr::Key key, HostMgr::Value value) : QEvent(User)
  {
    this->key   = key;
    this->value = value;
  }
};

#endif // HOSTMGR_H
