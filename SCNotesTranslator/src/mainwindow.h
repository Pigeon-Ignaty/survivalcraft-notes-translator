#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "tinyxml2.h"
#include <QString>
#include <QFile>
#include "QFileDialog"
#include <QMessageBox>
#include "debug.h"
#include "iostream"
#include <QVariant>
#include <sstream>
#include "qtexteditstream.h"
#include <QObject>
#include "set"
#include "about.h"
#include <QSettings>
#include <QDebug>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QCoreApplication>
#include <QMenuBar>
#include <QLibrary>
#include <QTranslator>
#include <QApplication>
#include "MusicXmlReader.h"
#include "TranslateMusic.h"

using namespace std;
using namespace tinyxml2;

//Фун-я вызова окна справки
using  CreateHelpWidget = QWidget* (*)(QWidget* parent);

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    enum Instrument{
        Bell = 1,
        Organ = 2,
        Ping = 3,
        Strings = 4,
        Trumpet = 5,
        Voice = 6,
        Piano = 7,
        PianoLong = 8,
        Drums = 9,
        Bass = 10
    };
    enum NoteType{
        Pitch = 0,
        Octave = 1
    };

    enum VersionTranslate{
        Old = 0,
        New = 1
    };

    struct OctaveRange {
        int lowOctave;
        int highOctave;
        int offset;  //Для колокольчика до 2.4 - 1, для всех остальных 0
    };

    //Октавы и инcтрументы до 2.4
    // const QMap<Instrument/*номер инструмента*/, OctaveRange/*мин. октава, макс. октава, сдвиг*/>
    // m_instrOctRangeOld{
    //     {Bell,{3, 4, 1}},
    //     {Organ,{3, 5, 0}},
    //     {Ping,{3, 5, 0}},
    //     {Strings,{2, 4, 0}},
    //     {Trumpet,{2, 4, 0}},
    //     {Voice,{3, 5, 0}},
    //     {Piano,{2, 5, 0}},
    //     {PianoLong,{2, 5, 0}}
    // };

    // //Октавы и интрументы после 2.4
    // const QMap<Instrument/*номер инструмента*/, OctaveRange/*мин. октава, макс. октава, сдвиг*/>
    // m_instrOctRangeNew{
    //     {Bell,{2, 5, 0}},
    //     {Organ,{3, 5, 0}},
    //     {Ping,{3, 6, 0}},
    //     {Strings,{2, 5, 0}},
    //     {Trumpet,{2, 5, 0}},
    //     {Voice,{3, 6, 0}},
    //     {Piano,{2, 5, 0}},
    //     {PianoLong,{2, 5, 0}},
    //     {Bass,{2, 5, 0}},
    // };
    QMap<Instrument, OctaveRange> m_instrOctRangeCurrent;

    struct Language {
        static constexpr const char* ru = "ru";
        static constexpr const char* en = "en";
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void setupUi();// Собираем UI
    void initializeSettings(); //чтение ini настроек
    void loadHelpLibrary(); //Ф-я чтения dll

    void showTranslation();
    void show_notes();
    void show_octaves();
    void show_instruments(); //вывод допольнительных строк инструментов
    void show_volumes();

public slots:

    void slotOpenMenuConsole(bool checked);//Окно создание/скрытия или открытия окна консоли

    void slotOpenAboutQT();//Открыть окно о qt

    void slotOpenFile(QString testPathFile = "");//Нажатие кнопки Открыть
    void slotCloseFile();//Нажатие кнопки Закрыть
    void slotGetTranslation(QVector <int> &converted_notes, QVector <int> &converted_octaves); //Получаем перевод

    void slotComboBoxPartsIndexChanged(int index);//Слот изменения партии
    void slotComboBoxVoicesIndexChanged(const QString &arg1);//Слот изменения голосов
    void slotComboBoxInstrumentsIndexChanged(int index);//Слот изменения инструментов

    void slotDisplayInstrumentString(int arg1);
    void slotDisplayVolumeString(int arg1);

    void slotAboutApplication();

    void slotOpenManual();
    void slotChangeLanguage(QAction *action);//Слот смены языка
    void slotChangedVersion(bool checked);
    void slotConsoleClose();
public:
    void retranslateUI();
protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void signalMainWindowClose();
    void showAboutWindow(); //сигнал открытия окна о приложении
    void signalChangeLanguage();
private:
    //################################## UI элементы
    QWidget *m_centralWidget = nullptr;
    QTextEdit *m_translatedArea = nullptr;

    // Комбобоксы
    QComboBox *m_comboPartSelection = nullptr;
    QComboBox *m_comboVoiceSelection = nullptr;
    QComboBox *m_comboInstrumentSelection = nullptr;

    // Чекбоксы
    QCheckBox *m_checkVolumeString = nullptr;
    QCheckBox *m_checkInstrumentString = nullptr;

    // Метки
    QLabel *m_labelPartySelection = nullptr;
    QLabel *m_labelVoiceSelection = nullptr;
    QLabel *m_labelInstrumentSelection = nullptr;

    // Действия меню Файл
    QAction *m_openAction = nullptr;
    QAction *m_closeAction = nullptr;
    QAction *m_exitAction = nullptr;

    // Действия меню Настройки
    QActionGroup *m_languageGroup = nullptr;
    QAction *m_ruAction = nullptr;
    QAction *m_enAction = nullptr;
    QAction *m_configDrumsAction = nullptr;//Не используется
    QAction *m_configStringNameAction = nullptr;//Не используется
    QAction *m_consoleAction = nullptr;
    QAction *m_newVersionTranslateAction = nullptr;
    QAction *m_oldVersionTranslateAction = nullptr;

    // Действия меню Справка
    QAction *m_helpAction = nullptr;
    QAction *m_aboutApplicationAction = nullptr;
    QAction *m_aboutQtAction = nullptr;

    // Меню бар
    QMenu *m_fileMenu = nullptr;
    QMenu *m_settingsMenu = nullptr;
    QMenu *m_helpMenu = nullptr;
    QMenu *m_versionMenu = nullptr;
    QMenu *m_languageSelectionMenu = nullptr;//Выбор языка

    QTextEditStreamOutput *sout; // Объявление объекта outputStream
    Debug *console;//окно консоли
    About* aboutWindow = nullptr; //окно о программе

    QLibrary helpLib;//Библиотека справки
    QWidget *m_helpWidget = nullptr; //окно справки
    QTranslator *m_translator = nullptr;

    MusicXmlReader *m_xmlReader = nullptr;
    TranslateMusic *m_musicTranslator = nullptr;
    //данные из класса  MusicXMLReader
    XMLDocument doc; // объект класса XML
    bool successOpenFile = false; //флаг успешного открытия файла
    bool OpenAboutProgram = false; //флаг существования окна
    QVector <QString> MusicalParts; // список партий, полученный из файла
    QVector <QString> IdParts; // содержит название Id партий
    QStringList Voices; // массив для записи голосов
    map<string, int> PercussionSurvivalcraftMap{ // содержит название и тип инструмента в Survivalcraft
        {"Acoustic Bass Drum",1},
        {"Bass Drum 1",1},
        {"Side Stick",9}, //подходит 2,3
        {"Acoustic Snare",0},
        {"Electric Snare",0},
        {"Low Floor Tom",5},
        {"Closed Hi-Hat",2},
        {"High Floor Tom",5},
        {"Pedal Hi-Hat",3},
        {"Low Tom",5},
        {"Open Hi-Hat",8}, //вроде бы 4, но больше подходит 8
        {"Low-Mid Tom",6},
        {"Hi-Mid Tom",6},
        {"Crash Cymbal 1",7},
        {"High Tom",6},
        {"Ride Cymbal 1",8},
        {"Chinese Cymbal",7},
        {"Ride Bell",8},
        {"Tambourine",3},
        {"Splash Cymbal",4}, //подходит 4 и 7
        {"Cowbell",3},
        {"Crash Cymbal 2",7},
        {"Ride Cymbal 2",8},
        {"Open Hi Conga",6},
        {"Low Conga",5},
        {"Cabasa",3},
        {"Castanets",2},
        {"Hand Clap",9},
        {"F",15}
    };

    //массивы с конвертированными данными
    QVector <int> converted_notes;
    QVector <int> converted_notes_sequence;
    QVector <int> converted_octaves;
    QVector <int> converted_octaves_sequence;

    map<string, string> PercussionPartInstruments; //запись id инструмента и его названия из партии
    int count_measure = 1; //счётчик тактов с 1
    int divisions = 1; //переменная для правильного расчёта всех длительностей в партии
    string sign; // символ ключа
    int chromatic = 0; // количество тонов для перехода от написанного к звучащему тону
    int fifths = 0; // знаки при ключе
    int beats = 0; // количество долей в числителе тактового размера
    int beat_type = 0; //количество долей в знаменателе тактового размера
    int line = 0; //Используется для ключей.2 для знака G (скрипичный ключ), 4 для знака F (басовый ключ) и 3 для знака C (альтовый ключ).
    int bpm = 0; // beat per minute
    int step = 0; // шаг для pitch
    int octave = 0; // октава
    QVector<char> v_notes; // вектор с нотами      | r - rest пауза|
    QVector<string> v_octaves;//вектор с октавами    | n - null при rest в notes|
    QVector<int> v_semitone; // вектор с полутонами
    QVector<string> v_voices;// вектор с голосами
    QVector<int> v_league; // вектор с координатами лиг
    QVector<float> v_duration; // вектор с длительностями
    QVector<string> v_instruments;// вектор с ударными инструментами
    int alter = 0; //показывает полутон на ноте, если он есть на ключе
    string rest; // пауза
    int duration = 0; //счётчик тактов с 1
    int high_octave; //верхняя  октава
    int low_octave; //нижняя октава
    QByteArray parseXml(const QString & zipFilePath);

    int fullTranslate(); // функция, которая вызывает другие функции для перевода нот
    void notes_f(QVector<char>& notes, QVector<int>& semitone, int chromatic);
    void octaves_f(QVector<int>& converted_notes, QVector<string>& octaves, int instrument);
    void convertToSequence(QVector<int>&, QVector<float>&, QVector<int>&, NoteType type);//вывод послед нот и октав
    void convert_to_sequence_percussion(QVector <string>&, QVector<float>&);
    void show_information_about_composition(QVector<float>& duration, int fraction_numerator, int denominator_fraction, int bpm);

    int switch_hex_notes(char ch, int semitone); //перевод значений в 16 формат survivalcraft
    void print_amount(int note, int duration, int league, QVector <int>& notes_or_octaves); //формирование последовательности нот или октав с учётом длительности и лиг

private:
    void initializeConsole();
    void resetDefaultSettings(QSettings & settings);
    void loadSettings(QSettings & settings);
    void saveSettings(QSettings & settings);
    void updateInstrumentsComboBox();

    void fillComboBoxParties();
    void fillComboBoxVoices();
    void tryTranslate();// Валидация перед переводом
    //Данные для работы теста
    friend class Test_SCNotesTranslator;
signals:
    void signalBadXmlFile();//Открыт некорректный файл


};
#endif // MAINWINDOW_H
