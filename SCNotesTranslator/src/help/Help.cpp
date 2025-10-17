#include "Help.h"
#include <QLabel>
#include <QIcon>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QGuiApplication>
#include <QScreen>
Help::Help(QWidget *parent) : QWidget (parent)
{
    setWindowFlags(Qt::Window);
    loadSettings();
    this->setWindowIcon(QIcon(":/pigeon.jpg"));
    this->setWindowTitle(tr("Manual"));
    setWindowModality(Qt::NonModal);
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

void Help::loadSettings()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    auto geometry = settings.value("Help/Geometry").toByteArray();
    if(geometry.isEmpty()){     
        if(QWidget *parent = parentWidget()){
            resize(parent->size());
            move(parent->pos());
            settings.setValue("Help/Geometry", saveGeometry());
        }
        else{
            resize(600,400);
        }
    }
    else{
        restoreGeometry(geometry);
    }
}

void Help::saveSettings()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.setValue("Help/Geometry", saveGeometry());
}

void Help::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QWebEngineView *view = findChild<QWebEngineView *>();
        if(view) {
            view->stop();           // остановка JS и загрузки
            view->page()->setView(nullptr); // отвязать страницу от виджета
            view->page()->deleteLater();
            view->deleteLater();
        }


    QWidget::closeEvent(event); // вызываем базовый обработчик
}

void Help::slotTranslate()
{
    setWindowTitle(tr("Manual"));
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
