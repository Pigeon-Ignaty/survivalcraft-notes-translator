#include "helpdialog.h"
#include "ui_helpdialog.h"
#include <QUrl>
#include <QMessageBox>
#include <QWebEngineView>

HelpDialog::HelpDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    // Указываем URL с локальным или внешним HTML-файлом
    QString htmlContent = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Help</title>
        </head>
        <body>
            <h1>Welcome to Help</h1>
            <p>This is a paragraph with an image and a video:</p>
            <img src="https://via.placeholder.com/150" alt="Sample Image">
            <video width="320" height="240" controls>
                <source src="https://www.w3schools.com/html/mov_bbb.mp4" type="video/mp4">
                Your browser does not support the video tag.
            </video>
        </body>
        </html>
    )";

    // Загружаем HTML-контент
    //ui->webEngineView->setHtml(htmlContent);
    //ui->widget->setHtml(htmlContent);
    ui->widget->setUrl(QUrl::fromLocalFile("C:\\Users\\Nikolay\\Downloads\\1\\справка — site.html"));

    //ui->
}

HelpDialog::~HelpDialog()
{
    delete ui;
}

void HelpDialog::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
    emit closed();
}
