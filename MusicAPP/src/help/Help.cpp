#include "Help.h"
#include <QLabel>
#include <QIcon>
#include <QCoreApplication>
#include <QFile>
Help::Help(QWidget *parent) : QWidget (parent)
{
    setWindowFlags(Qt::Window);
    resize(parent->size());
    this->setWindowIcon(QIcon(":/pigeon.jpg"));
    QHBoxLayout *layout = new QHBoxLayout(this);
    QWebEngineView *view = new QWebEngineView(this);
    layout->addWidget(view);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    //Проверяем пути, вдруг распаковали с лишней папкой guide
    QString mainPath = QCoreApplication::applicationDirPath() + "/guide/index.html";
    if(!QFile::exists(mainPath)){
        QString reservePath = QCoreApplication::applicationDirPath() + "/guide/guide/index.html";

        if(QFile::exists(reservePath)){
            mainPath = reservePath;
        }
    }
    view->load(QUrl::fromLocalFile(mainPath));
}

Help::~Help()
{

}

extern "C"{
    // Экспорт функции для вызова справки
    HELPSHARED_EXPORT QWidget* createHelpWidget(QWidget *parent){
        return new Help(parent);
    }
    // Экспорт функции для вызова ф-ии с версией dll
    HELPSHARED_EXPORT const char *getLibraryVersion(){
        return HELP_VERSION;
    }
}
