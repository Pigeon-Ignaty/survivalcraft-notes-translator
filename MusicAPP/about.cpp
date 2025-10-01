#include "about.h"
#include "ui_about.h"
#include <qpixmap.h>

About::About(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::About)
{
    setWindowModality(Qt::ApplicationModal);
    ui->setupUi(this);
    ui->label_2->setTextFormat(Qt::RichText);
    ui->label_2->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->label_2->setOpenExternalLinks(true);
}

About::~About()
{
    delete ui;
}
