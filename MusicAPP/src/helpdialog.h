#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QWidget>
#include <QCloseEvent>
#include <QDialog>
namespace Ui {
class HelpDialog;
}

class HelpDialog : public QWidget
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);
    ~HelpDialog();
signals:
    void signalHelpClose();
    void closed();
private:
    Ui::HelpDialog *ui;
protected:
    void closeEvent(QCloseEvent *event) override; // Переопределяем closeEvent
private slots:
};

#endif // HELPDIALOG_H
