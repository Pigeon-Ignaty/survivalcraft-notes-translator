#include "about.h"
#include "ui_about.h"
#include <qpixmap.h>
#include <QPushButton>
#include <QDialog>
#include <QLibrary>
#include <QDir>
//Фун-я получения версии dll справки
using GetHelpVersion = const char* (*)();

About::About(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle(tr("О программе"));

    //Стили для виджетов
    this->setStyleSheet(R"(
        QLabel {
            color: black;
            font-size: 14px;
            font-weight: normal;
        }

        QLabel#TitleLabel {
            font-size: 16px;
            font-weight: bold;
            color: black;
        }

        QPushButton {
            font-size: 14px;
            padding: 6px 12px;
        }
    )");
    auto layoutWithText = new QVBoxLayout;
    auto layoutLogo = new QVBoxLayout;
    auto title = new QLabel("Программное обеспечение для перевода нот, октав, громкости звука и инструментов в 16-формат Survivalcraft", this);
    title->setObjectName("TitleLabel");
    title->setWordWrap(true);
    title->setMaximumWidth(460);

    auto appVersion = new QLabel(this);
    appVersion->setText(tr("Версия приложения: 1.0.0"));

    QString helpVersion = getHelpVersion();
    QString text;

    if (!helpVersion.isEmpty()) {
        text = tr("Версия справки: ") + helpVersion;
    } else {
        text = tr("Справка не найдена. ") +
                R"(<a href="https://github.com/Pigeon-Ignaty/survivalcraft-notes-translator/releases">Скачать</a>)";
    }
    auto libraryVersion = new QLabel(text, this);
    libraryVersion->setTextFormat(Qt::RichText);
    libraryVersion->setTextInteractionFlags(Qt::TextBrowserInteraction);
    libraryVersion->setOpenExternalLinks(true);

    auto author = new QLabel(this);
    author->setText(tr("Автор: <a href=\"https://github.com/Pigeon-Ignaty\">Pigeon Ignaty</a>"));
    author->setTextFormat(Qt::RichText);
    author->setTextInteractionFlags(Qt::TextBrowserInteraction);
    author->setOpenExternalLinks(true);

    auto copyright = new QLabel(tr("© 2024 - 2025 Права голубя защищены, но сам он не пользуется защитой"), this);
    copyright->setWordWrap(true);
    copyright->setMaximumWidth(460);

    auto description = new QLabel(tr("Данная программа предназначена для чтения файлов musicxml с нотами, обнаружения музыкальных партий и перевода их в специальный 16-формат. В результате музыка в игре воспроизводится практически так же, как и оригинальные ноты. Программа имеет удобный и понятный интерфейс, упрощающий процесс перевода нот и значительно экономящий время."), this);
    description->setWordWrap(true);
    description->setMaximumWidth(460);

    auto newReleases = new QLabel();
    newReleases->setText(tr("<a href=\"https://github.com/Pigeon-Ignaty/survivalcraft-notes-translator/releases\">Проверить на наличие новой версии</a>"));
    newReleases->setTextFormat(Qt::RichText);
    newReleases->setTextInteractionFlags(Qt::TextBrowserInteraction);
    newReleases->setOpenExternalLinks(true);

    auto btnOk = new QPushButton(tr("Закрыть"), this);
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);

    layoutWithText->addWidget(title);
    layoutWithText->addSpacing(10);
    layoutWithText->addWidget(appVersion);
    layoutWithText->addWidget(libraryVersion);
    layoutWithText->addWidget(author);
    layoutWithText->addWidget(copyright);
    layoutWithText->addWidget(description);
    layoutWithText->addWidget(newReleases);
    layoutWithText->addStretch();
    layoutWithText->addWidget(btnOk, 0, Qt::AlignRight);

    auto logo = new QLabel;
    QPixmap pix(":/logo_main.png");
    logo->setPixmap(pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setAlignment(Qt::AlignCenter);
    layoutLogo->addWidget(logo);
    auto mainLayout = new QHBoxLayout;
    mainLayout->addLayout(layoutLogo);
    mainLayout->addLayout(layoutWithText);

    setLayout(mainLayout);
    setFixedSize(700, 380);
}

About::~About()
{

}

QString About::getHelpVersion()
{
    QString originalWorkingDir = QDir::currentPath();

    QLibrary helpLibrary("Help");

    //Ищем библиотку рядом с exe или в папке guide
    if (!helpLibrary.load()) {
        QString libFullPath = QCoreApplication::applicationDirPath() + "/guide/Help.dll";
        QFileInfo helpLibInfo(libFullPath);

        if (helpLibInfo.exists()) {
            QDir::setCurrent(helpLibInfo.absolutePath());
            helpLibrary.setFileName("Help");

            if (!helpLibrary.load()) {
                QDir::setCurrent(originalWorkingDir);
                return {};
            }
        }
    }
    GetHelpVersion getVersionFunc = (GetHelpVersion)helpLibrary.resolve("getLibraryVersion");

    if (!getVersionFunc) {
        helpLibrary.unload();
        QDir::setCurrent(originalWorkingDir);
        return {};
    }

    const char* helpVersion = getVersionFunc();
    QDir::setCurrent(originalWorkingDir); // восстановление рабочей директории
    return QString::fromUtf8(helpVersion);
}
