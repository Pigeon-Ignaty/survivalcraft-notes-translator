#include "Help.h"

#include <QLabel>
#include <QIcon>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QDir>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineView>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QDebug>


// Класс страницы, который открывает ссылки во внешнем браузере
class ExternalPage : public QWebEnginePage {
public:
    using QWebEnginePage::QWebEnginePage;

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override {
        if (type == QWebEnginePage::NavigationTypeLinkClicked) {
            QDesktopServices::openUrl(url); // открываем ссылку вне приложения
            return false; // не загружаем ссылку в QWebEngineView
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }
};

Help::Help(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Window);
    setWindowModality(Qt::NonModal);
    setWindowIcon(QIcon(":/pigeon.jpg"));
    setWindowTitle(tr("Manual"));

    loadSettings();

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Создаём профиль WebEngine в папке приложения
    QString profilePath = QCoreApplication::applicationDirPath() + "/web_profile";
    QDir().mkpath(profilePath);
    auto *profile = new QWebEngineProfile(profilePath, this);
    auto *page = new ExternalPage(profile, this);

    auto *view = new QWebEngineView(this);
    view->setPage(page);
    layout->addWidget(view);
    setLayout(layout);

    // Определяем путь к файлу справки
    QString baseDir = QCoreApplication::applicationDirPath();
    QString mainPath = QDir::cleanPath(baseDir + QDir::separator() + "guide/index.html");

    if (!QFile::exists(mainPath)) {
        QString reservePath = QDir::cleanPath(baseDir + QDir::separator() + "guide/guide/index.html");
        if (QFile::exists(reservePath)) {
            mainPath = reservePath;
        }
    }

    // Загружаем HTML-файл
    QUrl url = QUrl::fromLocalFile(mainPath);
    view->load(url);
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
            resize(640,480);
        }
    }
    else{
        restoreGeometry(geometry);
    }
    this->setMinimumSize(640,480);
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
