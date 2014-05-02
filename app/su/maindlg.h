#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QDesktopServices>

#include <VHttpRequest>
#include <VHttpResponse>
#include "bypasswebproxy.h"

// ----------------------------------------------------------------------------
// CommandEvent
// ----------------------------------------------------------------------------
class CommandEvent : public QEvent
{
public:
  typedef enum
  {
    Open,
    Close,
    Show,
    Hide,
    Option,
    Terminate
  } CommandEventType;
  CommandEventType type;

public:
  CommandEvent(CommandEventType type) : QEvent(User)
  {
    this->type = type;
  }
};

// ----------------------------------------------------------------------------
// MainDlg
// ----------------------------------------------------------------------------
namespace Ui {
  class MainDlg;
}

class MainDlg : public QDialog, public VXmlable
{
  Q_OBJECT

public:
  explicit MainDlg(QWidget *parent = 0);
  ~MainDlg();

public:
  bool autoOpen;
  bool showHomepage;

public:
  virtual void load(VXml xml);
  virtual void save(VXml xml);

public:
  BypassWebProxy bwp;

public:
  VTcpServer   tcpServer;
  virtual bool event(QEvent* event);
  virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result);

public slots:
  void run(VTcpSession* tcpSession);
  void newHostDetected(HostMgr::Key key, HostMgr::Value value);

public slots:
  void _open();
  void _close();
  void _show();
  void _hide();
  void _option();
  void _terminate();

public:
  void initializeControl();
  void finalizeControl();
  void loadControl();
  void saveControl();
  void setControl();

private slots:
  void on_btnOpen_clicked();

  void on_btnClose_clicked();

  void on_btnOption_clicked();

public:
  Ui::MainDlg *ui;
};

#endif // DIALOG_H
