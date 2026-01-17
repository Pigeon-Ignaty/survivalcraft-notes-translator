#include "test_SCNotesTranslator.h"
#include <QDebug>
#include <QEventloop>
#include <QCryptographicHash>
#ifndef XML_DIR
#define XML_DIR "."
#endif

int main(int argc, char *argv[]){
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-logging --log-level=3");

    QApplication app(argc, argv);
    Test_SCNotesTranslator test;

    test.run();
    return 0;
}
Test_SCNotesTranslator::Test_SCNotesTranslator()
{

}

Test_SCNotesTranslator::~Test_SCNotesTranslator()
{
    delete m_testApp;
}

bool Test_SCNotesTranslator::testsParsingFilesTranslate()
{
    QString filePath = QCoreApplication::applicationDirPath() + "/test.musicxml";

    //1. Файл не найден
    createTempFile("Пожилой еретик");

    QFile::remove(filePath);

    m_textXMLReader->loadFile(filePath);
    if(m_textXMLReader->error() == Error::FileNotFound)
        results.append({true, m_textXMLReader->getErrorString()});
    else
        results.append({false, m_textXMLReader->getErrorString()});

    //2. Пустой файл
    createTempFile();

    m_textXMLReader->loadFile(filePath);
    if(m_textXMLReader->error() == Error::EmptyFile)
        results.append({true, m_textXMLReader->getErrorString()});
    else
        results.append({false, m_textXMLReader->getErrorString()});
    QFile::remove(filePath);

    //3.Создаём пустой архив
    createTempFile("", "mxl");
    m_textXMLReader->loadZipFile(filePath);
    if(m_textXMLReader->error() == Error::ZipFileOpenError)
        results.append({true, m_textXMLReader->getErrorString()});
    else
        results.append({false, m_textXMLReader->getErrorString()});

    //4. Здесь необходимо указать свой путь до файла, так как нельзя mz_zip_archive скормить из qrc, только полный путь!!!
    m_textXMLReader->loadZipFile("Z:\\VS\\Survivalcraft_notes_translator_last\\survivalcraft-notes-translator\\SCNotesTranslator\\tests\\resources\\xml\\Couldn't read file.mxl");
    if(m_textXMLReader->error() == Error::ErrorExtractFile)
        results.append({true, m_textXMLReader->getErrorString()});
    else
        results.append({false, m_textXMLReader->getErrorString()});
    //5. Нет id у партии
    m_textXMLReader->load(":/xml/brokenXml.musicxml");
    if(!m_textXMLReader->parseParties())
        results.append({true, "Нет id у партии"});
    else
        results.append({false, "Нет id у партии"});
    //6. Парсинг несуществующего голоса, должен вернуть false
    m_textXMLReader->load(":/xml/AllNotes_test.musicxml");
    if(m_textXMLReader->parseParties() && !m_textXMLReader->parseVoices("-3123123"))
        results.append({true, "Парсинг несуществующего голоса"});
    else
        results.append({false, "Парсинг несуществующего голоса"});
    return true;
}

bool Test_SCNotesTranslator::testCheckTranslation()
{
    //Тест немного корявый из-за всей логики в mainwindow и очень долгий, позже будет исправлено
    //Здесь передаём все файлы xml, получаем перевод, пишем в файлик. Затем сравниваем с правильным

    QVector<QPair<QString, QString>> musicFiles = {{"Bad_Piggies_Theme",".musicxml"},
                                                   {"Drums", ".mxl"},
                                                   {"AllNotes_test", ".musicxml"},
                                                   {"voices", ".musicxml"},
                                                   {"SUBWAY SURFERS (Main Theme)", ".musicxml"},
                                                    {"Bad_Piggies_Theme",".musicxml"},
                                                    {"Drums", ".mxl"},
                                                    {"AllNotes_test", ".musicxml"},
                                                    {"voices", ".musicxml"},
                                                    {"SUBWAY SURFERS (Main Theme)", ".musicxml"}};
    bool translationPassed = true;
    for(const auto &file : musicFiles){
        m_testApp->slotOpenFile(QString("%1/%2").arg(XML_DIR).arg(file.first + file.second));
        bool success =  translateFile(file.first);//Запускаем тест и передаём правильный файл
        results.append({success, file.first + file.second + " " + displayTime()});
        translationPassed &= success;
    }
    return translationPassed;
}

bool Test_SCNotesTranslator::createTempFile(const QString content, const QString& extension)
{
    QString exeDir = QCoreApplication::applicationDirPath();
    QString filePath = exeDir + "/test." + extension;
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        file.write(content.toUtf8());
        file.close();
        return false;
    }
    return true;
}

void Test_SCNotesTranslator::run()
{
    m_testApp = new MainWindow();

    m_testApp->show();
    //Включаем по умолчаем старую версию перевода. Подключаю слоты тут, так как не работает
    connect(m_testApp->m_newVersionTranslateAction, &QAction::triggered, m_testApp, &MainWindow::slotChangedVersion);
    connect(m_testApp->m_oldVersionTranslateAction, &QAction::triggered, m_testApp, &MainWindow::slotChangedVersion);
    m_testApp->m_newVersionTranslateAction->setChecked(false);
    m_testApp->m_oldVersionTranslateAction->trigger();

    m_textXMLReader = new MusicXmlReader(this);
    //Таймер
    m_timerThread = new QThread;
    m_worker = new Worker;
    m_worker->moveToThread(m_timerThread);
    QObject::connect(m_timerThread, &QThread::started, m_worker, &Worker::startTimer);
    connect(m_timerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_timerThread->start();

    qDebug() << "НАЧАЛО ТЕСТА" << displayTime();

    results.clear();
    testsParsingFilesTranslate();
    for(auto &result : results)
        qDebug() << result.ok << " " << result.message;

    results.clear();
    testCheckTranslation();
    for(auto &result : results)
        qDebug() << result.ok << " " << result.message;

    qDebug() << "КОНЕЦ ТЕСТА" << displayTime();

    QMetaObject::invokeMethod(m_worker, "stopTimer", Qt::QueuedConnection);
    m_timerThread->quit();     //Просим поток завершиться
    m_timerThread->wait();     //Ждём завершения
    m_timerThread->deleteLater();
}

bool Test_SCNotesTranslator::translateFile(QString fileName)
{
    QString allData;

    //Переведём всё произведение(все партии и все голоса) для всех инструментов
    auto translate = [this, &allData](){
        for(int instrument = 0; instrument < m_testApp->m_comboInstrumentSelection->count(); instrument++){
            m_testApp->m_comboInstrumentSelection->setCurrentIndex(instrument);
            qDebug() << "Инструмент: " << m_testApp->m_comboInstrumentSelection->currentText()  << displayTime();
            for(int part = 0; part < m_testApp->m_comboPartSelection->count(); part++){
                m_testApp->m_comboPartSelection->setCurrentIndex(part);
                qDebug() << "\tПартия: " << m_testApp->m_comboPartSelection->currentText();
                for(int voice = 0; voice < m_testApp->m_comboVoiceSelection->count(); voice++){
                    m_testApp->m_comboVoiceSelection->setCurrentIndex(voice);
                    qDebug() << "\tГолос: " << m_testApp->m_comboVoiceSelection->currentText();

                    allData.append(m_testApp->m_translatedArea->toPlainText());
                }
            }
        }
    };
    allData.append("СТАРАЯ ВЕРСИЯ\n");
    qDebug() << "СТАРАЯ ВЕРСИЯ";
    m_testApp->m_newVersionTranslateAction->setChecked(false);
    m_testApp->m_oldVersionTranslateAction->trigger();
    translate();

    allData.append("НОВАЯ ВЕРСИЯ\n");
    qDebug() << "НОВАЯ ВЕРСИЯ";

    m_testApp->m_oldVersionTranslateAction->setChecked(false);
    m_testApp->m_newVersionTranslateAction->trigger();

    translate();

    QString checkFileName = fileName + "_TO_CHECK.txt";

    //Открываем файл и записываем в него перевод
    QFile file(checkFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate| QIODevice::Text)){
        qDebug() << "Ошибка: не удалось открыть файл" << checkFileName;
        return false;
    }

    file.write(allData.toUtf8());

    file.close();

    //Сравниваем полученный файл с проверенным по хешу
    bool result = compareFiles(fileName + "_TO_CHECK.txt", ":/results/" + fileName + "_VERIFIED.txt");
    if(result){
        qDebug() << fileName << " перевод правильный";
        return true;
    }
    else{
        qDebug() << fileName << " перевод неправильный";
        return false;
    }
}

QString Test_SCNotesTranslator::displayTime()
{
    return QTime(0,0).addSecs(m_worker->getSeconds().loadAcquire()).toString("hh:mm:ss");
}

bool Test_SCNotesTranslator::compareFiles(QString toCheckFileName, QString verifiedFileName)
{
    QFile f1(toCheckFileName), f2(verifiedFileName);

    if(!f1.open(QIODevice::ReadOnly) || !f2.open(QIODevice::ReadOnly)){
        qDebug() << "Ошибка: не удалось открыть файлы " << toCheckFileName << ", " << verifiedFileName;

        return false;
    }

    QByteArray h1 = QCryptographicHash::hash(f1.readAll(), QCryptographicHash::Sha256);
    QByteArray h2 = QCryptographicHash::hash(f2.readAll(), QCryptographicHash::Sha256);

    return h1 == h2;
}

template <typename Signal>
bool Test_SCNotesTranslator::waitForSignal(Signal signal, int timeoutSec)
{
    bool received = false;
    QEventLoop loop;

    //Как только получаем сигнал из приложения, выходим
    QObject::connect(m_testApp, signal, &loop, [&]() {
        received = true;
        loop.quit();
    });

    //Запускаем таймер, который сработает и выйдет из цикла, если время закончится
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutSec * 1000);
    loop.exec();
    return received;
}
