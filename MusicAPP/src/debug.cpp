#include "debug.h"
#include "ui_debug.h"

Debug::Debug(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Debug)
{
    ui->setupUi(this);
    ui->TextConsole->setReadOnly(true);
}

Debug::~Debug()
{
    delete ui;
}

QTextEdit* Debug::getTextEdit() const {
    return ui->TextConsole;
}

void Debug::closeEvent(QCloseEvent *event)
{
    emit signalConsoleClose();
    event->accept();
}

void Debug::receiveText(const QString& text) {
    textEdit->append(text);
}

void Debug::SlotMainWindowClose()
{
    Debug::close();
}

void Debug::on_ClearButton_clicked()
{
    ui->TextConsole->clear(); //очистить консоль
}

void Debug::on_CopyButton_clicked()
{
    ui->TextConsole->selectAll();//выбрать всё
    ui->TextConsole->copy();//скопировать в буфер обмена
}
