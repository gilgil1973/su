#ifndef OPTIONDLG_H
#define OPTIONDLG_H

#include <QDialog>

namespace Ui {
  class OptionDlg;
}

class OptionDlg : public QDialog
{
  Q_OBJECT

public:
  explicit OptionDlg(QWidget *parent = 0);
  ~OptionDlg();

private:
  Ui::OptionDlg *ui;
};

#endif // OPTIONDLG_H
