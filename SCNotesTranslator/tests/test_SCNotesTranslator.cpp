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

bool Test_SCNotesTranslator::testErrorXml()
{
    //Запускаем открытие с неправильным файлом
    QTimer::singleShot(0, [=](){
        m_testApp->slotOpenFile(QString("%1/error.musicxml").arg(XML_DIR));
    });

    QTimer::singleShot(0, [=](){
        const auto topLevelWidgets = QApplication::topLevelWidgets();
        for (QWidget *w : topLevelWidgets) {
            if (QMessageBox *mb = qobject_cast<QMessageBox*>(w)) {
                mb->accept(); // закрываем окно с ошибкой
            }
        }
    });

    bool ok = waitForSignal(&MainWindow::signalBadXmlFile, 10);
    return ok;
}

bool Test_SCNotesTranslator::testCheckTranslation()
{
    //Тест немного корявый из-за всей логики в mainwindow и очень долгий, позже будет исправлено
    //Здесь передаём все файлы xml, получаем перевод, пишем в файлик. Затем сравниваем с правильным

    QVector<QPair<QString, QString>> musicFiles = {{"Bad_Piggies_Theme",".musicxml"},
                                                   {"Drums", ".mxl"},
                                                   {"AllNotes_test", ".musicxml"},
                                                   {"voices", ".musicxml"},
                                                   {"SUBWAY SURFERS (Main Theme)", ".musicxml"}};
    bool translationPassed = true;
    for(const auto &file : musicFiles){
        m_testApp->slotOpenFile(QString("%1/%2").arg(XML_DIR).arg(file.first + file.second));
        translationPassed &= translateFile(file.first);//Запускаем тест и передаём правильный файл
    }
    if(translationPassed)
        qDebug() << "2. Тест CheckTranslation пройден" << displayTime();
    else
        qDebug() << "2. Тест CheckTranslation не пройден" << displayTime();

    return translationPassed;
}

void Test_SCNotesTranslator::run()
{
    m_testApp = new MainWindow();
    m_testApp->show();

    //Таймер
    m_timerThread = new QThread;
    m_worker = new Worker;
    m_worker->moveToThread(m_timerThread);
    QObject::connect(m_timerThread, &QThread::started, m_worker, &Worker::startTimer);
    connect(m_timerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_timerThread->start();

    // ----- ТЕСТЫ -----
    QVector <QPair<bool, QString>> tests{
        {testErrorXml(),        "1. Тест ErrorXml"},        //Отрытие ошибочного файла
        {testCheckTranslation(),"2. Тест CheckTranslation"} //Проверка перевода каждого файла
    };

    qDebug() << "НАЧАЛО ТЕСТА" << displayTime();

    for(const auto& test : tests){
        if(test.first)
            qDebug() << test.second << (test.first ? " пройден" : " не пройден") << displayTime();
    }
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
            for(int party = 0; party < m_testApp->m_comboPartySelection->count(); party++){
                m_testApp->m_comboPartySelection->setCurrentIndex(party);
                qDebug() << "\tПартия: " << m_testApp->m_comboPartySelection->currentText();
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

    //Сначала переведём для старой, затем для новой версии
    m_testApp->m_oldVersionTranslateAction->setEnabled(true);
    translate();

    allData.append("НОВАЯ ВЕРСИЯ\n");
    qDebug() << "НОВАЯ ВЕРСИЯ";

    m_testApp->m_newVersionTranslateAction->setEnabled(true);
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
