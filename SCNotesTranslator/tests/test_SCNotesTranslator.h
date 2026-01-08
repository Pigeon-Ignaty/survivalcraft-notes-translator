#pragma once

#include <QObject>
#include "mainwindow.h"
#include "MusicXmlReader.h"
#include <QAtomicInt>
#include <QTimer>
#include <QThread>
#include <QTime>
/*----------------------------------------------------
Класс тестирования всего приложения.
Всего 2 теста:
1. Открытие сломанного XML
    Открываем сломанный файл и ловим сигнал из приложения, что произошла ошибка открытия. В таком случае
    считаем тест успешным.
2. Перевод нескольких произведений
    Берём произведение, открываем. Далее проходимся по каждому инструменту, каждой партии и голосу.
    Перевод записываем в файл и сравниваем с правильным. Если содержимое совпадёт с файловым примером, то тест пройден.
На весь тест уходит 30 минут, поэтому требуется переписать архитектуру приложения.
----------------------------------------------------*/

struct TestResult{
    bool ok;
    QString message;
};

class Worker;

class Test_SCNotesTranslator :public QObject
{
Q_OBJECT
public:
    Test_SCNotesTranslator();
    ~Test_SCNotesTranslator();

    void run();//Запуск всех тестов
private slots:
    bool testsParsingFilesTranslate(); //Тест метода load из MusicXmlReader
    bool testCheckTranslation();//Тест проверки перевода на нескольких xml
private:
    bool createTempFile(const QString content = "", const QString& extension = "musicxml"); //Создание временного файла
    bool translateFile(QString file); //Переводим каждый xml файл

    template <typename Signal>
    bool waitForSignal(Signal signal, int timeoutSec = 10);//Ф-я для отслеживания сигналов для тестирования
    QString displayTime(); //Ф-я вывода времени из другого потока
    MainWindow *m_testApp = nullptr; //Тестируемое приложение
    MusicXmlReader *m_textXMLReader = nullptr;
    QThread *m_timerThread = nullptr;
    Worker *m_worker = nullptr; //Класс для запуска и оставноки таймера
    QVector<TestResult> results;

    bool compareFiles(QString toCheckFileName, QString verifiedFileName);//Сравниваем тестируемый и проверенный файлы
};

//Таймер в другом потоке
class Worker : public QObject {
    Q_OBJECT
public:
    Worker(QObject *parent = nullptr) : QObject(parent){

    };
public slots:
    void startTimer() {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, [this](){
            seconds.fetchAndAddRelaxed(1);
        });
        m_timer->start(1000);
    }

    void stopTimer(){
        if(m_timer && m_timer->isActive()){
            m_timer->stop();
        }
    }
    QAtomicInt getSeconds(){
        return seconds;
    }
private:
    QAtomicInt seconds;
    QTimer *m_timer = nullptr;

};
