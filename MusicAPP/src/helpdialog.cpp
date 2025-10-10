#include "helpdialog.h"
#include "ui_helpdialog.h"
#include <QUrl>
#include <QMessageBox>
#include <QWebEngineView>
#include <QIcon>
#include <QDebug>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineView>
#include <QDir>
HelpDialog::HelpDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/pigeon.jpg"));
    // html страница на wp
    QWebEngineSettings::defaultSettings()->setAttribute(
        QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    QString path = QDir(QCoreApplication::applicationDirPath()).filePath("guide/index.html");
    ui->widget->setUrl(QUrl::fromLocalFile(path));

}
HelpDialog::~HelpDialog()
{
    delete ui;
}

void HelpDialog::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
    emit closed();
}

