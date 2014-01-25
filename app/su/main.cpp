#include <QApplication>
#include <VApp>
#include "siteunblocker.h"
#include "maindlg.h"
#include <VDebugNew>

int run(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainDlg dlg;

  dlg.show();
  int res = a.exec();
  return res;
}

int main(int argc, char *argv[])
{
  VApp::initialize();

  LOG_INFO("site unblocker version %s started %s ", SiteUnblocker::version, VDREAM_VERSION);
  int res = run(argc, argv);
  VApp::finalize();
  LOG_INFO("site unblocker version %s terminated", SiteUnblocker::version);
  return res;
}
