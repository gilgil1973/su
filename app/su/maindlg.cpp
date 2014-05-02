#include "maindlg.h"
#include "ui_maindlg.h"
#include "siteunblocker.h"
#include <VDebugNew>

MainDlg::MainDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::MainDlg)
{
  LOG_DEBUG("");
  ui->setupUi(this);
  initializeControl();
  loadControl();

  if (!tcpServer.open())
  {
    LOG_ERROR("tcpServer.Open return false");
    QMessageBox::warning(NULL, "error", "can not open command server(may be already lauched)");
    QApplication::postEvent(this, new QEvent(QEvent::Close));
    return;
  }

  if (autoOpen) _open();
  if (showHomepage)
  {
    QDesktopServices::openUrl(QUrl("http://su.gilgil.net/manage/"));
  }

  setControl();
}

MainDlg::~MainDlg()
{
  LOG_DEBUG("");
  tcpServer.close();

  saveControl();
  finalizeControl();
  setControl();
  delete ui;
}

void MainDlg::load(VXml xml)
{
  {
    VXml coordXml = xml.findChild("coord");
    if (!coordXml.isNull())
    {
      QRect rect = geometry();
      rect.setLeft  ((coordXml.getInt("left",   0)));
      rect.setTop   ((coordXml.getInt("top",    0)));
      rect.setWidth ((coordXml.getInt("width",  640)));
      rect.setHeight((coordXml.getInt("height", 480)));
      setGeometry(rect);
    }
  }

  if (!xml.findChild("bwp").isNull()) bwp.load(xml.gotoChild("bwp"));
  tcpServer.load(xml.gotoChild("tcpServer"));
  autoOpen     = xml.getBool("autoOpen", autoOpen);
  showHomepage = xml.getBool("showHomepage", showHomepage);
}

void MainDlg::save(VXml xml)
{
  {
    VXml coordXml = xml.gotoChild("coord");
    QRect rect = geometry();
    coordXml.setInt("left",   rect.left());
    coordXml.setInt("top",    rect.top());
    coordXml.setInt("width",  rect.width());
    coordXml.setInt("height", rect.height());
  }

  bwp.save(xml.gotoChild("bwp"));
  tcpServer.save(xml.gotoChild("tcpServer"));
  xml.setBool("autoOpen", autoOpen);
  xml.setBool("showHomepage", showHomepage);
}

void MainDlg::initializeControl()
{
  LOG_DEBUG("");
  QString title = (QString)("site unblocker version ") + SiteUnblocker::version;
  this->setWindowTitle(title);

  Qt::WindowFlags flags = windowFlags();
  flags |= Qt::WindowMinimizeButtonHint;
  flags &= ~Qt::WindowContextHelpButtonHint;
  setWindowFlags(flags);

  move(0, 0);

  autoOpen       = false;
  showHomepage   = true;
  tcpServer.port = 8081;

  QObject::connect(&tcpServer, SIGNAL(runned(VTcpSession*)), this, SLOT(run(VTcpSession*)), Qt::DirectConnection);
  QObject::connect(&bwp, SIGNAL(newHostDetected(HostMgr::Key,HostMgr::Value)), this, SLOT(newHostDetected(HostMgr::Key,HostMgr::Value)), Qt::DirectConnection);
}

void MainDlg::finalizeControl()
{
  LOG_DEBUG("");
  bwp.close();
}

void MainDlg::loadControl()
{
  LOG_DEBUG("");
  this->loadFromDefaultDoc("mainDlg");
}

void MainDlg::saveControl()
{
  LOG_DEBUG("");
  this->saveToDefaultDoc("mainDlg");
}

void MainDlg::setControl()
{
  bool active = bwp.active();
  ui->btnOpen->setEnabled(!active);
  ui->btnClose->setEnabled(active);
  ui->btnOption->setEnabled(!active);
}

bool MainDlg::event(QEvent* event)
{
  CommandEvent *commandEvent = dynamic_cast<CommandEvent*>(event);
  if (commandEvent != NULL)
  {
    switch (commandEvent->type)
    {
      case CommandEvent::Open:      _open(); break;
      case CommandEvent::Close:     _close(); break;
      case CommandEvent::Show:      _show(); break;
      case CommandEvent::Hide:      _hide(); break;
      case CommandEvent::Option:    _option(); break;
      case CommandEvent::Terminate: _terminate(); break;
    }
    return true;
  }

  HostMgrEvent* hostMgrEvent = dynamic_cast<HostMgrEvent*>(event);
  if (hostMgrEvent != NULL)
  {
    int rowCount = ui->tableWidget->rowCount();
    rowCount++;
    ui->tableWidget->setRowCount(rowCount);
    int row = rowCount - 1;
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(hostMgrEvent->key.host));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(hostMgrEvent->key.port)));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number((int)(hostMgrEvent->value.policy))));
    ui->tableWidget->scrollToBottom();
    return true;
  }

  return QDialog::event(event);
}

bool MainDlg::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
  MSG* msg = reinterpret_cast<MSG*>(message);
  if (msg->message == WM_QUERYENDSESSION)
  {
    LOG_INFO("WM_QUERYENDSESSION received hwnd=%p lParam=%p wParam=%p", msg->hwnd, msg->lParam, msg->wParam);
    QMessageBox::warning(NULL, "warning", "close su application before shutdown");
    *result = FALSE;
    return true;
  }
  return QDialog::nativeEvent(eventType, message, result);
}

void MainDlg::run(VTcpSession* tcpSession)
{
  LOG_DEBUG("stt");
  VHttpRequest  request;
  VHttpResponse response;

  while (true)
  {
    QByteArray packet;
    int readLen = tcpSession->read(packet);
    if (readLen == VERR_FAIL) break;
    if (!request.parse(packet))
    {
      LOG_ERROR("error in parse http request");
      break;
    }
    QByteArray path = request.requestLine.path;
    QEvent* event = NULL;
    if (path == "/open")           event = new CommandEvent(CommandEvent::Open);
    else if (path == "/close")     event = new CommandEvent(CommandEvent::Close);
    else if (path == "/show")      event = new CommandEvent(CommandEvent::Show);
    else if (path == "/hide")      event = new CommandEvent(CommandEvent::Hide);
    else if (path == "/option")    event = new CommandEvent(CommandEvent::Option);
    else if (path == "/terminate") event = new CommandEvent(CommandEvent::Terminate);
    else
    {
      LOG_ERROR("invalid path %s", request.requestLine.path.data());
      break;
    }
    QApplication::postEvent(this, event);
    tcpSession->write(
      "HTTP/1.0 200 OK\r\n"\
      "\r\n"\
      "<body onload='javascript:window.history.back()'>");
    tcpSession->close();
  }

  LOG_DEBUG("end");
}

void MainDlg::newHostDetected(HostMgr::Key key, HostMgr::Value value)
{
  HostMgrEvent* event = new HostMgrEvent(key, value);
  QApplication::postEvent(this, event);
}

void MainDlg::_open()
{
  LOG_DEBUG("");
  ui->btnOpen->click();
}

void MainDlg::_close()
{
  LOG_DEBUG("");
  ui->btnClose->click();
}

void MainDlg::_show()
{
  LOG_DEBUG("");
  show();
}

void MainDlg::_hide()
{
  LOG_DEBUG("");
  hide();
}

void MainDlg::_option()
{
  LOG_DEBUG("");
  ui->btnOption->click();
}

void MainDlg::_terminate()
{
  LOG_DEBUG("");
  saveControl();
  finalizeControl();
  close();
}

void MainDlg::on_btnOpen_clicked()
{
  LOG_DEBUG("");
  if (!bwp.open())
  {
    QMessageBox::warning(NULL, "error", bwp.error.msg);
  }
  setControl();
}

void MainDlg::on_btnClose_clicked()
{
  LOG_DEBUG("");
  bwp.close();
  setControl();
}

void MainDlg::on_btnOption_clicked()
{
  LOG_DEBUG("");
  QMessageBox::information(NULL, "option", "not supported yet");
}
