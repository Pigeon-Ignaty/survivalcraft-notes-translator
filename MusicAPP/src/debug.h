#ifndef DEBUG_H
#define DEBUG_H

#include <QWidget>
#include <QCloseEvent>
#include <QTextEdit>
#include <QMainWindow>
namespace Ui {
class Debug;
}

class Debug : public QWidget
{
    Q_OBJECT

public:
    explicit Debug(QWidget *parent = nullptr);
    ~Debug();
    QTextEdit* getTextEdit() const; // Метод для доступа к QTextEdit
private:
    Ui::Debug *ui;
    QTextEdit *textEdit;
protected:
    void closeEvent(QCloseEvent *event) override;
signals:
    void signalConsoleClose();
public slots:
    void receiveText(const QString& text);
    void SlotMainWindowClose();
private slots:
    void on_ClearButton_clicked();
    void on_CopyButton_clicked();
};

#endif // DEBUG_H
