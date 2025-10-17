#include "about.h"
#include <qpixmap.h>
#include <QPushButton>
#include <QDialog>
#include <QLibrary>
#include <QDir>

#define HELP_VERSION "1.0.0"

//Фун-я получения версии dll справки
using GetHelpVersion = const char* (*)();

About::About(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle(tr("About"));

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
    auto title = new QLabel(tr("Software for translating notes, octaves, sound volume, and instruments into Survivalcraft's 16-format"), this);
    title->setObjectName("TitleLabel");
    title->setWordWrap(true);
    title->setMaximumWidth(460);

    auto appVersion = new QLabel(this);
    appVersion->setText(tr("Application version: %1").arg(QString::fromUtf8(HELP_VERSION)));

    QString helpVersion = getHelpVersion();
    QString text;

    if (!helpVersion.isEmpty()) {
        text = tr("Manual version: ") + helpVersion;
    } else {
        QString url = "https://github.com/Pigeon-Ignaty/survivalcraft-notes-translator/releases";
        text = tr("Manual not found. ") + QString("<a href=\"%1\">%2</a>").arg(url, tr("Download"));
    }
    auto libraryVersion = new QLabel(text, this);
    libraryVersion->setTextFormat(Qt::RichText);
    libraryVersion->setTextInteractionFlags(Qt::TextBrowserInteraction);
    libraryVersion->setOpenExternalLinks(true);

    auto author = new QLabel(this);
    author->setText(tr("Author")+ ": <a href=\"https://github.com/Pigeon-Ignaty\">Pigeon Ignaty</a>");
    author->setTextFormat(Qt::RichText);
    author->setTextInteractionFlags(Qt::TextBrowserInteraction);
    author->setOpenExternalLinks(true);

    auto copyright = new QLabel(tr("© 2024 - 2025 Pigeon's rights are protected, but the pigeon itself doesn't use protection"), this);
    copyright->setWordWrap(true);
    copyright->setMaximumWidth(460);

    auto description = new QLabel(tr("This program is designed to read musicxml files containing notes, detect musical parts, and convert them into a special 16-format. As a result, the music in the game plays almost the same as the original notes. The program features a user-friendly and intuitive interface that simplifies the note translation process and significantly saves time."), this);
    description->setWordWrap(true);
    description->setMaximumWidth(460);

    auto newReleases = new QLabel();
    QString url = "https://github.com/Pigeon-Ignaty/survivalcraft-notes-translator/releases";
    newReleases->setText(QString("<a href=\"%1\">%2</a>").arg(url, tr("Check for new version")));
    newReleases->setTextFormat(Qt::RichText);
    newReleases->setTextInteractionFlags(Qt::TextBrowserInteraction);
    newReleases->setOpenExternalLinks(true);

    auto btnOk = new QPushButton(tr("Close"), this);
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
    logo->setPixmap(pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setAlignment(Qt::AlignCenter);
    layoutLogo->addWidget(logo);
    layoutLogo->addStretch();
    auto mainLayout = new QHBoxLayout;
    mainLayout->addLayout(layoutLogo);
    mainLayout->addLayout(layoutWithText);

    setLayout(mainLayout);
    adjustSize();
    setFixedSize(size());
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
