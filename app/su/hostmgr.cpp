#include "hostmgr.h"
#include <VDebugNew>

// ----------------------------------------------------------------------------
// HostMgr
// ----------------------------------------------------------------------------
HostMgr::HostMgr()
{
  clear();
}

HostMgr::~HostMgr()
{
  clear();
}

void HostMgr::clear()
{
  items.clear();
}

void HostMgr::load(VXml xml)
{
  clear();

  xml_foreach (child, xml.childs())
  {
    Key key;
    Value value;

    key.host = child.getStr("host", key.host);
    key.port = child.getInt("port", key.port);
    value.policy = (HttpRequestChangePolicy)child.getInt("policy", (int)value.policy);
    items.insert(key, value);
  }
}

void HostMgr::save(VXml xml)
{
  xml.clearChild();

  for (HostItemMap::iterator it = items.begin(); it != items.end(); it++)
  {
    VXml child = xml.addChild("host");
    Key   key   = it.key();
    Value value = it.value();

    child.setStr("host", key.host);
    child.setInt("port", key.port);
    child.setInt("policy", (int) value.policy);
  }
}
