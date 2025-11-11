#include "mainwindow.h"
#include <QMenu>
#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi();
    initializeSettings();
    loadHelpLibrary();
}


MainWindow::~MainWindow()
{

}

void MainWindow::slotOpenFile(QString testPathFile) //открытие файла
{
    QSettings settings("config.ini", QSettings::IniFormat);
    QString path_to_file;
    if(testPathFile.isEmpty())//Если не тест, то выбираем через проводник
        path_to_file = QFileDialog::getOpenFileName(this,tr("Select a .musicxml file"),settings.value("UI/LastOpenFilePath","").toString(),"*.musicxml *.mxl");
    else
        path_to_file = testPathFile; //Иначе тест и получает путь из теста

    if(path_to_file.isEmpty()){
        return;
    }
    if(!path_to_file.isEmpty()){
        settings.setValue("UI/LastOpenFilePath", path_to_file);
        *sout << "Последний сохраненный путь:" << path_to_file.toUtf8().constData() << endl;
        }

    QFileInfo extension(path_to_file); //получаем расширение файла
    QByteArray xmlFile;
    if(extension.suffix() == "mxl")//если архив, то распаковка
    {
        xmlFile = parseXml(path_to_file);
    }
    else {//иначе запишем содержимое файла в QByteArray
        QFile file(path_to_file);
        if(file.open(QIODevice::ReadOnly)){
            xmlFile = file.readAll();
            file.close();
        }
    }
    tinyxml2::XMLError error = doc.Parse(xmlFile.constData(),xmlFile.size());
    if (!error == tinyxml2::XML_SUCCESS) {//ошибка открытия
        QMessageBox errorMessage;
        errorMessage.setIcon(QMessageBox::Critical); // Установка иконки ошибки
        errorMessage.setText(tr("Oops!"));
        errorMessage.setInformativeText(tr("An error occurred. The file was not found, has an incorrect extension, or contains an error."));
        errorMessage.setWindowTitle(tr("Error"));
        errorMessage.exec();
        *sout <<"Ошибка открытия " << doc.Error()<< endl;
        slotCloseFile();
        return;
    }

    else{//если успех
        slotCloseFile();
        m_comboPartySelection->setEnabled(true);// активация списков
        m_comboVoiceSelection->setEnabled(true);
        m_checkInstrumentString->setEnabled(true);
        m_checkVolumeString->setEnabled(true);

        XMLElement* pRootElement = doc.RootElement(); // корневой каталог
        if (nullptr != pRootElement) { // если не пустой
            XMLElement* credit = pRootElement->FirstChildElement("credit"); // спускаемся в credit
                while (credit) {
                    XMLElement* credit_type_tag = credit->FirstChildElement("credit-type"); // в цикле вытаскиваем тип
                    XMLElement* credit_words_tag = credit->FirstChildElement("credit-words"); // в цикле вытаскиваем имя типа
                    if (nullptr != credit_type_tag && credit_words_tag != nullptr) { // если не пустой
                        *sout << credit_type_tag->GetText() << ": " << credit_words_tag->GetText() << "\t"; // вывод автора и названия
                    }
                    credit = credit->NextSiblingElement("credit"); //след элемент
                }
                *sout << "Вывод следующих партий в музыкальной композиции: " << endl;
                int scorePartCount = 0;
                XMLElement* part_list = pRootElement->FirstChildElement("part-list");
                if (part_list != nullptr) {
                    for (XMLElement* score_part = part_list->FirstChildElement("score-part");
                        score_part != nullptr; score_part = score_part->NextSiblingElement("score-part"))
                    {
                        *sout << "ID: " << score_part->Attribute("id") << "\t";
                        XMLElement* part_name = score_part->FirstChildElement("part-name");
                        if (part_name != nullptr) {
                            *sout << "Название партии: " << string(part_name->GetText()) << endl;
                            MusicalParts.push_back(QString(part_name->GetText())); //добавление названий партии в массив
                            m_comboPartySelection->addItem(QString(score_part->Attribute("id")) + " " + QString(part_name->GetText()));//добавление название партии в ComboBox
                            IdParts.push_back(QString(score_part->Attribute("id"))); //добавление id партии
                        }

                        scorePartCount++;
                    }
                }
                m_comboPartySelection->setMaxVisibleItems(m_comboPartySelection->count());     // Установка максимального количества видимых элементов
                *sout << "scorePartCount: " << scorePartCount << endl;
                m_closeAction->setEnabled(true);
                QFileInfo fileInfo(path_to_file);//получение название файла
                QString fileName = fileInfo.fileName();
                setWindowTitle(tr("Survivalcraft notes translator: - %1").arg(fileName));//добавление в шапку
                successOpenFile = true;

            }
    }
    m_comboPartySelection->blockSignals(false);
    m_comboVoiceSelection->blockSignals(false);
    m_checkInstrumentString->blockSignals(false);
    slotComboBoxPartsIndexChanged(m_comboPartySelection->currentIndex());
}

void MainWindow::slotOpenMenuConsole(bool checked)
{
    if(checked){
        console->show();
    }
    else{
        console->hide();
    }
}

void MainWindow::slotConsoleClose()
{
    *sout << "console close" << endl;
    m_consoleAction->setChecked(false);
}

void MainWindow::retranslateUI()
{
    //Переводим actions
    m_fileMenu->setTitle(tr("File"));
    m_openAction->setText(tr("Open"));
    m_closeAction->setText(tr("Close"));
    m_exitAction->setText(tr("Exit"));

    m_settingsMenu->setTitle(tr("Settings"));
    m_languageSelectionMenu->setTitle(tr("Language"));
    m_configDrumsAction->setText(tr("Drum kit configurator"));
    m_configStringNameAction->setText(tr("Line name settings"));
    m_consoleAction->setText(tr("Debug information"));
    m_versionMenu->setTitle(tr("Version"));
    m_newVersionTranslateAction->setText(tr("After 2.4 (including)"));
    m_oldVersionTranslateAction->setText(tr("Before 2.4 (deprecated)"));

    m_helpMenu->setTitle(tr("Help"));
    m_helpAction->setText(tr("Manual"));
    m_aboutApplicationAction->setText(tr("About"));
    m_aboutQtAction->setText(tr("About QT"));
    //Перевод ui элементов
    m_labelPartySelection->setText(tr("Party selection"));
    m_labelVoiceSelection->setText(tr("Voice selection"));
    m_labelInstrumentSelection->setText(tr("Instrument selection"));

    m_checkVolumeString->setText(tr("Volume line"));
    m_checkInstrumentString->setText(tr("Instrument line"));

    updateInstrumentsComboBox();

}


void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("config.ini", QSettings::IniFormat); //сохранение настроек пользователя
    saveSettings(settings);
    emit signalMainWindowClose();
    event->accept();
}


void MainWindow::slotOpenAboutQT()//о версии qt
{
    QMessageBox aboutQt;
    aboutQt.setWindowIcon(QIcon(":/logo_qt.ico"));
    QMessageBox::aboutQt(&aboutQt,tr("About QT"));
}

void MainWindow::slotCloseFile()//закрытие через menu bar
{
    m_openAction ->setEnabled(true);
    m_closeAction->setEnabled(false);
    setWindowTitle(tr("Survivalcraft notes translator:"));
    m_comboPartySelection->blockSignals(true);
    m_comboVoiceSelection->blockSignals(true);
        if (m_comboPartySelection && m_comboPartySelection->count() > 0) {
        m_comboPartySelection->clear();
    }

    if (m_comboVoiceSelection && m_comboVoiceSelection->count() > 0) {
        m_comboVoiceSelection->clear();
    }
    m_comboPartySelection->setEnabled(false);// и блокировка
    m_comboVoiceSelection->setEnabled(false);
    IdParts.clear();
    MusicalParts.clear();
    Voices.clear();
    successOpenFile = false;
    v_notes.clear();
    v_semitone.clear();
    v_league.clear();
    v_semitone.clear();
    converted_notes.clear();
    converted_notes_sequence.clear();
    converted_octaves.clear();
    converted_octaves_sequence.clear();
    v_notes.clear();
    v_octaves.clear();
    v_semitone.clear();
    v_voices.clear();
    v_league.clear();
    v_duration.clear();
    v_instruments.clear();
    m_translatedArea->clear();
    *sout << "Файл закрыт" << endl;
}

void MainWindow::slotComboBoxPartsIndexChanged(int index) //если был выбрана какая-то партия, то вывести для неё все голоса
{
    if(IdParts.isEmpty())
        return;
    m_comboVoiceSelection->blockSignals(true);
    m_comboVoiceSelection->clear();
    m_comboVoiceSelection->blockSignals(false);
    QByteArray byteArray = IdParts[index].toUtf8(); //перевод в const char*
    const char* NamePart = byteArray.constData();
    XMLElement* measure_tag = nullptr;

    XMLElement* pRootElement = doc.RootElement(); // корневой элемент
    if (pRootElement != nullptr) { // проверка на пустоту
        XMLElement* part_id_tag = pRootElement->FirstChildElement("part");
        while (part_id_tag != nullptr) {
            const char* partId = part_id_tag->Attribute("id");
            if (partId != nullptr && std::string(partId) == NamePart) {
                measure_tag = part_id_tag->FirstChildElement("measure");
                while (measure_tag != nullptr) {
                    XMLElement* note_tag = measure_tag->FirstChildElement("note");
                    while (note_tag != nullptr) {
                        XMLElement* voice_tag = note_tag->FirstChildElement("voice");
                        if (voice_tag != nullptr) {
                            Voices.push_back(voice_tag->GetText());
                        }
                        note_tag = note_tag->NextSiblingElement("note");
                    }
                    measure_tag = measure_tag->NextSiblingElement("measure");
                }
                break; // Выход из цикла, если найдена выбранная партия
            }
            part_id_tag = part_id_tag->NextSiblingElement("part");
        }
    }
    //сортировка голосов
    Voices.removeDuplicates();
    sort(Voices.begin(), Voices.end());
    *sout << "Голоса, обнаруженные в партии" << std::endl;
    m_comboVoiceSelection->blockSignals(true);
    for (const QString& v : Voices) {
        *sout << v.toStdString() << endl;
        m_comboVoiceSelection->addItem(v);
    }
    m_comboVoiceSelection->blockSignals(false);
    slotComboBoxInstrumentsIndexChanged(0);
}

void MainWindow::slotComboBoxVoicesIndexChanged(const QString &arg1) //если произошёл выбор голоса, то сразу перевести
{
    //очистка массивов перед переводом
    v_notes.clear();
    v_semitone.clear();
    v_league.clear();
    v_semitone.clear();
    converted_notes.clear();
    converted_notes_sequence.clear();
    converted_octaves.clear();
    converted_octaves_sequence.clear();
    v_notes.clear();
    v_octaves.clear();
    v_semitone.clear();
    v_voices.clear();
    v_league.clear();
    v_duration.clear();
    v_instruments.clear();
    m_translatedArea->clear();
    if(IdParts.isEmpty())
        return;
    QByteArray byteArray = IdParts[m_comboPartySelection->currentIndex()].toUtf8(); //перевод в const char*
    const char* NamePart = byteArray.constData();
    XMLElement* pRootElement = doc.RootElement(); // корневой каталог
        if (nullptr != pRootElement) { // если не пустой
            XMLElement* part_list_tag = pRootElement->FirstChildElement("part-list");
            if (part_list_tag != nullptr) {
                XMLElement* score_part_tag = part_list_tag->FirstChildElement("score-part");
                while (score_part_tag) {
                    const char* ScorePartId = score_part_tag->Attribute("id");
                    if (string(ScorePartId) == NamePart) {
                        XMLElement* score_instrument_tag = score_part_tag->FirstChildElement("score-instrument");
                        if (score_instrument_tag != nullptr) {
                            while (score_instrument_tag) {
                                XMLElement* instrument_name_tag = score_instrument_tag->FirstChildElement("instrument-name");
                                PercussionPartInstruments[score_instrument_tag->Attribute("id")] = string(instrument_name_tag->GetText());
                                score_instrument_tag = score_instrument_tag->NextSiblingElement("score-instrument");

                            }
                            PercussionPartInstruments["F"] = "F";
                            break;

                        }

                    }
                    score_part_tag = score_part_tag->NextSiblingElement("score-part");

                }


            }
            count_measure = 0;
            XMLElement* part_id_tag = pRootElement->FirstChildElement("part");
            XMLElement* measure_tag = nullptr;
            while (part_id_tag) {
                const char* partId = part_id_tag->Attribute("id");
                if (partId && std::string(partId) == NamePart) {
                    measure_tag = part_id_tag->FirstChildElement("measure");
                    // Анализируем каждый measure

                    while (measure_tag) { // если не пустой
                        XMLElement* attributes_tag = nullptr;
                        XMLElement* divisions_tag = nullptr;
                        XMLElement* key_tag = nullptr;
                        XMLElement* fifths_tag = nullptr;
                        XMLElement* time_tag = nullptr;
                        XMLElement* beats_tag = nullptr;
                        XMLElement* beat_type_tag = nullptr;
                        XMLElement* clef_tag = nullptr;
                        XMLElement* sign_tag = nullptr;
                        XMLElement* line_tage = nullptr;
                        XMLElement* transpose_tag = nullptr;
                        XMLElement* direction_tag = nullptr;
                        XMLElement* direction_type_tag = nullptr;
                        XMLElement* metronome_tag = nullptr;
                        XMLElement* beat_unit_tag = nullptr;
                        XMLElement* per_minute_tag = nullptr;

                        if (measure_tag->FirstChildElement("attributes") != nullptr) {
                            attributes_tag = measure_tag->FirstChildElement("attributes");
                            if (attributes_tag->FirstChildElement("divisions") != nullptr) {
                                divisions_tag = attributes_tag->FirstChildElement("divisions");
                                divisions = atoi(divisions_tag->GetText());
                            }
                            if (attributes_tag->FirstChildElement("key") != nullptr) {
                                key_tag = attributes_tag->FirstChildElement("key");
                                if (key_tag->FirstChildElement("fifths") != nullptr) {
                                    fifths_tag = key_tag->FirstChildElement("fifths");
                                }
                            }
                            if (attributes_tag->FirstChildElement("time") != nullptr) {
                                time_tag = attributes_tag->FirstChildElement("time");
                                if (time_tag->FirstChildElement("beats") != nullptr) {
                                    beats_tag = time_tag->FirstChildElement("beats");
                                }
                                if (time_tag->FirstChildElement("beat-type") != nullptr) {
                                    beat_type_tag = time_tag->FirstChildElement("beat-type");
                                }
                            }
                            if (attributes_tag->FirstChildElement("clef") != nullptr) {
                                clef_tag = attributes_tag->FirstChildElement("clef");
                                if (clef_tag->FirstChildElement("sign") != nullptr) {
                                    sign_tag = clef_tag->FirstChildElement("sign");
                                    sign = sign_tag->GetText();
                                }
                                if (clef_tag->FirstChildElement("line") != nullptr) {
                                    line_tage = clef_tag->FirstChildElement("line");
                                }
                            }
                            if (attributes_tag->FirstChildElement("transpose") != nullptr) {
                                transpose_tag = attributes_tag->FirstChildElement("transpose");
                                if (transpose_tag->FirstChildElement("chromatic") != nullptr) {
                                    chromatic = atoi(transpose_tag->FirstChildElement("chromatic")->GetText());
                                }
                            }
                        }
                        if (measure_tag->FirstChildElement("direction") != nullptr) {
                            direction_tag = measure_tag->FirstChildElement("direction");
                            if (direction_tag->FirstChildElement("direction-type") != nullptr) {
                                direction_type_tag = direction_tag->FirstChildElement("direction-type");
                                if (direction_type_tag->FirstChildElement("metronome") != nullptr) {
                                    metronome_tag = direction_type_tag->FirstChildElement("metronome");
                                    if (metronome_tag->FirstChildElement("beat-unit") != nullptr) {
                                        beat_unit_tag = metronome_tag->FirstChildElement("beat-unit");
                                    }
                                    if (metronome_tag->FirstChildElement("per-minute") != nullptr) {
                                        per_minute_tag = metronome_tag->FirstChildElement("per-minute");
                                        bpm = atoi(per_minute_tag->GetText());
                                    }
                                }
                            }
                        }


                        *sout << "Measure (такт): " << count_measure << endl;
                        if (fifths_tag != nullptr) {
                            fifths = atoi(fifths_tag->GetText());
                            *sout << "\tЗнаки: " << fifths << endl;
                        }

                        if (beats_tag != nullptr) {
                            beats = atoi(beats_tag->GetText());
                            *sout << "\tдолей в числителе тактового размера: " << beats_tag->GetText() << endl;
                        }

                        if (beat_type_tag != nullptr) {
                            beat_type = atoi(beat_type_tag->GetText());
                            *sout << "\t долей в знаменателе тактового размера: " << beat_type_tag->GetText() << endl;
                        }

                        if (sign_tag != nullptr) {
                            sign = string(sign_tag->GetText());
                            *sout << "\tsign (знак): " << sign << endl;
                        }

                        if (line_tage != nullptr) {
                            line = atoi(line_tage->GetText());
                            *sout << "\tline (номер для знака): " << line << endl;
                        }

                        //////Читаем ноты
                        XMLElement* note_tag = measure_tag->FirstChildElement("note");
                        XMLElement* voice_tag = note_tag->FirstChildElement("voice");

                        string current_voice = voice_tag->GetText();
                        bool flag_voice = false; //проверка на отсутсвие голоса в такте
                        while (note_tag) { // цикл по  нотам в measure
                            if(current_voice == m_comboVoiceSelection->currentText().toStdString().c_str()){
                                flag_voice = true;
                                if (note_tag->FirstChildElement("pitch") != nullptr) { //если нота, то вывести ноту и октаву
                                    XMLElement* step_tag = note_tag->FirstChildElement("pitch")->FirstChildElement("step"); //нота
                                    XMLElement* alter_tag = note_tag->FirstChildElement("pitch")->FirstChildElement("alter"); //отклонение на полутон
                                    XMLElement* octave_tag = note_tag->FirstChildElement("pitch")->FirstChildElement("octave"); //октава
                                    if (step_tag != nullptr) {
                                        *sout << "v_notes " << v_notes.size() - 1 << "\t"; // для консоли
                                        *sout << "\tstep: " << step_tag->GetText() << endl; // нота
                                        v_notes.push_back(step_tag->GetText()[0]);
                                        v_league.push_back(0);
                                    }
                                    if (alter_tag != nullptr) {
                                        alter = atoi(alter_tag->GetText());
                                        *sout << "v_semitone " << v_semitone.size() - 1 << "\t";

                                        *sout << "\talter: " << alter_tag->GetText() << endl;
                                        v_semitone.push_back(atoi(alter_tag->GetText()));

                                    }
                                    else { //если нет alter
                                        *sout << "v_semitone " << v_semitone.size() - 1 << "\t";

                                        v_semitone.push_back(0);

                                    }
                                    if (octave_tag != nullptr) {
                                        octave = atoi(octave_tag->GetText());
                                        *sout << "\toctave: " << octave_tag->GetText() << endl;
                                        v_octaves.push_back(octave_tag->GetText());

                                    }
                                }
                                XMLElement* rest_tag = note_tag->FirstChildElement("rest"); //Определение паузы
                                if (rest_tag != nullptr) {
                                    rest = rest_tag->Name();
                                    *sout << "\t" << rest_tag->Name() << endl;
                                    v_notes.push_back('r');
                                    v_octaves.push_back("n");
                                    v_semitone.push_back(0);
                                    v_league.push_back(0);

                                    *sout << "v_semitone " << v_semitone.size() - 1 << "\t";
                                    *sout << "v_notes " << v_notes.size() - 1 << "\t";

                                }
                                XMLElement* duration_tag = note_tag->FirstChildElement("duration"); //Определение длительности
                                if (duration_tag != nullptr) {//длительность
                                    duration = atoi(duration_tag->GetText());
                                    *sout << "\tduration: " << duration_tag->GetText() << endl;
                                    v_duration.push_back(duration);
                                }
                                XMLElement* instrument_tag = note_tag->FirstChildElement("instrument");//для инструмента
                                if (instrument_tag != nullptr) {
                                        if (instrument_tag->Attribute("id") != nullptr) {
                                            v_instruments.push_back(instrument_tag->Attribute("id"));
                                    }
                                }
                                else {
                                    v_instruments.push_back("F");

                                }
                                if (note_tag->FirstChildElement("notations") != nullptr) {
                                    if (note_tag->FirstChildElement("notations")->FirstChildElement("tied") != nullptr) {
                                        if (note_tag->FirstChildElement("notations")->FirstChildElement("tied")->Attribute("type") != nullptr) {
                                            XMLElement* tied_tag = note_tag->FirstChildElement("notations")->FirstChildElement("tied");
                                            //tied = tied_tag->Attribute("type");
                                            int count_tied = 0;
                                            while (tied_tag) {
                                                if (string(tied_tag->Attribute("type")) == "stop") count_tied++;
                                                //cout << "Attribute " << tied_tag->Attribute("type") << endl;
                                                if (string(tied_tag->Attribute("type")) == "start") count_tied += 2;
                                                //cout << "\ttied: " << count_tied << endl;

                                                tied_tag = tied_tag->NextSiblingElement("tied");//след элемент по tied

                                            }
                                            v_league[v_league.size() - 1] = count_tied;

                                            count_tied = 0;

                                        }


                                    }
                                }
                                *sout << "v_league " << v_league.size() - 1 << "\t";
                                *sout << endl;
                                }

                            note_tag = note_tag->NextSiblingElement("note");//след элемент по нотам
                            if (note_tag != nullptr) {
                                voice_tag = note_tag->FirstChildElement("voice"); //запись номера голоса в след ноте
                                current_voice = voice_tag->GetText();
                            }
                        }
                        measure_tag = measure_tag->NextSiblingElement("measure");//след элемент по measure
                        count_measure++;
                        //если в такте не найден нужный голос, то добавляем паузу во весь такт
                        if (flag_voice == false) {
                            v_notes.push_back('r');
                            v_octaves.push_back("n");
                            v_semitone.push_back(0);
                            v_league.push_back(0);
                            v_instruments.push_back("F");
                            //вычисляем размер такта с помощью хитрого алгоритма, до которого бот не догадался, а только дезинформировал, тварь. А у меня потёк котелок
                            int measure_size = 0;

                            if (beats / beat_type) {//если числитель больше знаменателя
                                float beatCount = beats / beat_type; //смотрим во сколько
                                float time_8 = divisions / beatCount; //получаем время одной восьмой
                                measure_size = time_8 * beats; //умножаем время 1 / 8 на числитель тактового размера
                            }
                            else{//тоже самое, если знаменатель больше числителя
                                float beatDuration = (4 / (float)beat_type) * divisions;
                                measure_size = (float)beats * beatDuration;
                                // алгоритм до 15.06, делил на 0
                                /*float beats_2 = beats*2;
                                float beat_type_2 = beat_type*2;
                                float beatCount = beats_2 / beat_type_2;
                                float time_8 = divisions / beatCount;
                                measure_size = time_8 * beats;*/
                            }
                            v_duration.push_back(measure_size);
                        }

                    }

                    // Переход к следующему элементу <part>

                    break;
                }
                part_id_tag = part_id_tag->NextSiblingElement("part");

            }

            }
    fullTranslate();
}

void MainWindow::slotComboBoxInstrumentsIndexChanged(int index) //вызов функции перевода после выбора инструмента
{
    *sout << "Выбран инструмент " << m_comboInstrumentSelection->currentText().toStdString() << endl;
    XMLElement* pRootElement = doc.RootElement(); // корневой каталог
    if(!doc.Error()){ //если выбран файл
        if(!pRootElement)
            return;
        slotComboBoxVoicesIndexChanged(QString(""));

    }

}

int MainWindow::fullTranslate()
{
    int instrument = m_comboInstrumentSelection->currentData().toInt();//Текущий выбранный инструмент

     notes_f(v_notes, v_semitone, chromatic);//перевод из нот CDEFGAB в hex представление в survivalcraft
     octaves_f(converted_notes, v_octaves, instrument); //перевод октавы в формат survivalcraft, в том числе обрезка по октавам
     //calculation_duration(minimal_duration);//пока заморожено
     if (sign != "percussion") {//если не ударные
         convertToSequence(converted_notes, v_duration, v_league, NoteType::Pitch);// перевод в последовательность нот в зависимости от длительности
         convertToSequence(converted_octaves, v_duration, v_league, NoteType::Octave);//перевод в последовательность октав в зависимости от длительности
         //show_information_about_composition(v_duration, beats, beat_type, bpm);//рекомендуемая частота генератора, не работает правильно
         show_notes(converted_notes_sequence); //вывод нот с делением на строки длиной 256 символов
         show_octaves(converted_octaves_sequence);//вывод октав с делением на строки длиной 256 символов
         show_instruments();
         show_volumes(); //вывод громкости
     }
     else { //ударные
         convert_to_sequence_percussion(v_instruments, v_duration);
         show_notes(converted_notes_sequence); //вывод нот с делением на строки длиной 256 символов
         show_instruments();
         show_volumes(); //вывод громкости
     }
     return 0;
}

void MainWindow::notes_f(QVector<char> &notes, QVector<int> &semitone, int chromatic)
{
    if (notes.size() != semitone.size()) {
             *sout << "Error: vectors have different sizes" << endl;
         }
         *sout << "Ноты в notes_f:" << endl;
         for (int i = 0; i < semitone.size(); i++) {
             semitone[i] = semitone[i] + chromatic;
             int hex_notes = switch_hex_notes(notes[i], semitone[i]);
             if (hex_notes < 0) { //??
                 converted_notes.push_back(12 + hex_notes);
             }
             else {
                 converted_notes.push_back(hex_notes);

             }
             *sout << hex << uppercase << hex_notes;
         }
         *sout << endl;
}

void MainWindow::octaves_f(QVector<int> &converted_notes, QVector<string> &octaves, int instrument)
{
    Instrument instr = static_cast<Instrument>(instrument);
    const int lowOctave = m_instrOctRangeCurrent.value(instr).lowOctave;
    const int highOctave = m_instrOctRangeCurrent.value(instr).highOctave;
    const int offset = m_instrOctRangeCurrent.value(instr).offset;

    for (int i = 0; i < octaves.size(); i++) {

        if (octaves[i] == "n") {
            converted_octaves.push_back(15);
        }
        else {
            try {
                int number = 0;
                number = stoi(octaves[i]);

                if (number > highOctave && converted_notes[i] == 0) {
                    number = highOctave + 1;
                }
                else {
                    if (number < lowOctave) {
                        number = lowOctave;
                    }
                    else if (number > highOctave) {
                        number = highOctave;
                    }
                }
                    converted_octaves.push_back(number - lowOctave + offset);
            }
            catch (const exception& ex) {
                *sout << ex.what() << endl;
            }
        }
    }
}

int MainWindow::switch_hex_notes(char ch, int semitone) {
    switch (ch) {
    case 'C': return 0 + semitone; //до
    case 'D': return 2 + semitone; //ре
    case 'E': return 4 + semitone; //ми
    case 'F': return 5 + semitone; //фа
    case 'G': return 7 + semitone; //соль
    case 'A': return 9 + semitone; //ля
    case 'B': return 11 + semitone;//си
    default: return 15; //пауза
    }
}

QByteArray MainWindow::parseXml(const QString & zipFilePath)
{
    QByteArray fileData;
    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(zipArchive));

    // Открываем ZIP файл
    if (!mz_zip_reader_init_file(&zipArchive, zipFilePath.toLocal8Bit().data(), 0)) {
        *sout << "Не удалось открыть ZIP файл." << endl;
        QMessageBox::critical(this, tr("Error"), tr("Failed to open ZIP file."));
        return 0;
    }

    // Получаем количество файлов в архиве
    int numFiles = mz_zip_reader_get_num_files(&zipArchive);
    if (numFiles < 1) {
        *sout << "В архиве нет файлов." << endl;
        QMessageBox::critical(this, tr("Error"), tr("No files in the archive."));
        mz_zip_reader_end(&zipArchive);
        return 0;
    }

    // Получаем имя архива (без расширения)
    QString zipFileName = QFileInfo(zipFilePath).baseName();
    bool fileFound = false;

    // Проходим по всем файлам в архиве
    for (int i = 0; i < numFiles; ++i) {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat)) {
            *sout << "Не удалось прочитать файл в архиве." << endl;
            QMessageBox::critical(this, tr("Error"), tr("Failed to read the file in the archive."));
            mz_zip_reader_end(&zipArchive);
            return 0;
        }

        QString fileName = QString::fromStdString(fileStat.m_filename);

        // Проверяем, совпадает ли имя файла с именем архива (с расширением .xml)
        if (fileName == zipFileName + ".xml") {
            size_t uncompressedSize = fileStat.m_uncomp_size;
            void* p = mz_zip_reader_extract_file_to_heap(&zipArchive, fileStat.m_filename, &uncompressedSize, 0);
            if (!p) {
                *sout << "Не удалось извлечь файл." << endl;
                QMessageBox::critical(this, tr("Error"), tr("Failed to extract the file."));
                mz_zip_reader_end(&zipArchive);
                return 0;
            }

            // Копируем содержимое файла в QByteArray
            fileData = QByteArray((const char*)p, uncompressedSize);

            // Освобождаем выделенную память
            mz_free(p);

            fileFound = true;
            break;
        }
    }

    mz_zip_reader_end(&zipArchive);

    if (!fileFound) {
        *sout << "Не удалось найти файл с именем архива в архиве." << endl;
        QMessageBox::critical(this, tr("Error"), tr("Failed to find the file with the archive name inside the archive."));
        return 0;
    }

    return fileData;
}

void MainWindow::setupUi()
{
    QHBoxLayout *m_upLayout = new QHBoxLayout;
    QHBoxLayout *m_downLayout = new QHBoxLayout;
    QVBoxLayout *m_centralLayout = new QVBoxLayout;

    m_centralWidget = new QWidget(this);

    QVBoxLayout* m_partColumn = new QVBoxLayout;
    QVBoxLayout* m_voiceColumn = new QVBoxLayout;
    QVBoxLayout* m_instrumentColumn = new QVBoxLayout;

    //---------- Font
    QFont commonFont;
    commonFont.setPointSize(9);

    //---------- First column
    m_labelPartySelection = new QLabel(tr("Party selection"));
    m_labelPartySelection->setFixedHeight(25);
    m_labelPartySelection->setFont(commonFont);

    m_comboPartySelection = new QComboBox;
    m_comboPartySelection->setFixedHeight(25);
    m_comboPartySelection->setFont(commonFont);

    m_checkVolumeString = new QCheckBox(tr("Volume line"));
    m_checkVolumeString->setFixedHeight(25);
    m_checkVolumeString->setFont(commonFont);

    m_partColumn->addWidget(m_labelPartySelection);
    m_partColumn->addWidget(m_comboPartySelection);
    m_partColumn->addWidget(m_checkVolumeString);
    m_partColumn->addStretch();

    //---------- Second column
    m_labelVoiceSelection = new QLabel(tr("Voice selection"));
    m_labelVoiceSelection->setFixedHeight(25);
    m_labelVoiceSelection->setFont(commonFont);

    m_comboVoiceSelection = new QComboBox;
    m_comboVoiceSelection->setFixedHeight(25);
    m_comboVoiceSelection->setFont(commonFont);

    m_checkInstrumentString = new QCheckBox(tr("Instrument line"));
    m_checkInstrumentString->setFixedHeight(25);
    m_checkInstrumentString->setFont(commonFont);

    m_voiceColumn->addWidget(m_labelVoiceSelection);
    m_voiceColumn->addWidget(m_comboVoiceSelection);
    m_voiceColumn->addWidget(m_checkInstrumentString);
    m_voiceColumn->addStretch();

    //---------- Third column
    m_labelInstrumentSelection = new QLabel(tr("Instrument selection"));
    m_labelInstrumentSelection->setFixedHeight(25);
    m_labelInstrumentSelection->setFont(commonFont);

    m_comboInstrumentSelection = new QComboBox;
    m_comboInstrumentSelection->setFixedHeight(25);
    m_comboInstrumentSelection->setFont(commonFont);

    m_instrumentColumn->addWidget(m_labelInstrumentSelection);
    m_instrumentColumn->addWidget(m_comboInstrumentSelection);
    m_instrumentColumn->addStretch();

    //---------- Top and Bottom Layout
    m_upLayout->addLayout(m_partColumn);
    m_upLayout->addLayout(m_voiceColumn);
    m_upLayout->addLayout(m_instrumentColumn);

    m_translatedArea = new QTextEdit;
    commonFont.setPointSize(10);
    m_translatedArea->setFont(commonFont);
    m_downLayout->addWidget(m_translatedArea);
    m_centralLayout->addLayout(m_upLayout);
    m_centralLayout->addLayout(m_downLayout, 1);

    m_centralWidget->setLayout(m_centralLayout);
    setCentralWidget(m_centralWidget);

    //---------- Menu: Файл
    m_fileMenu = menuBar()->addMenu(tr("File"));

    m_openAction = new QAction(tr("Open"), this);
    m_openAction->setShortcut(QKeySequence::Open);

    m_closeAction = new QAction(tr("Close"), this);
    m_closeAction->setShortcut(QKeySequence::Close);
    m_closeAction->setEnabled(false);  // Пока отключено

    m_exitAction = new QAction(tr("Exit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);

    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addAction(m_closeAction);
    m_fileMenu->addAction(m_exitAction);

    //---------- Menu: Настройки
    m_settingsMenu = menuBar()->addMenu(tr("Settings"));

    m_languageSelectionMenu = new QMenu(tr("Language"),this);
    m_ruAction = new QAction(tr("Русский"), this);
    m_ruAction->setData(Language::ru);
    m_ruAction->setCheckable(true);

    m_enAction = new QAction(tr("English"), this);
    m_enAction->setData(Language::en);
    m_enAction->setCheckable(true);
    m_enAction->setChecked(true);

    m_languageGroup = new QActionGroup(this);
    m_languageGroup->setExclusive(true);
    m_languageGroup->addAction(m_ruAction);
    m_languageGroup->addAction(m_enAction);

    m_languageSelectionMenu->addAction(m_ruAction);
    m_languageSelectionMenu->addAction(m_enAction);

    m_configDrumsAction = new QAction(tr("Drum kit configurator"), this);
    m_configDrumsAction->setEnabled(false);

    m_configStringNameAction = new QAction(tr("Line name settings"), this);
    m_configStringNameAction->setEnabled(false);

    m_consoleAction = new QAction(tr("Debug information"), this);

    m_versionMenu = new QMenu(tr("Version"), this);
    m_oldVersionTranslateAction = new QAction(tr("Before 2.4 (deprecated)"), this);
    m_newVersionTranslateAction = new QAction(tr("After 2.4 (including)"), this);
    QActionGroup *versionGroup = new QActionGroup(this);
    versionGroup->setExclusive(true);
    versionGroup->addAction(m_oldVersionTranslateAction);
    versionGroup->addAction(m_newVersionTranslateAction);
    m_oldVersionTranslateAction->setData(0);
    m_newVersionTranslateAction->setData(1);

    m_versionMenu->addAction(m_oldVersionTranslateAction);
    m_versionMenu->addAction(m_newVersionTranslateAction);

    m_settingsMenu->addMenu(m_languageSelectionMenu);
    m_settingsMenu->addAction(m_configDrumsAction);
    m_settingsMenu->addAction(m_configStringNameAction);
    m_settingsMenu->addAction(m_consoleAction);
    m_settingsMenu->addMenu(m_versionMenu);

    //---------- Menu: Справка
    m_helpMenu = menuBar()->addMenu(tr("Help"));

    m_helpAction = new QAction(tr("Manual"), this);
    m_aboutApplicationAction = new QAction(tr("About"), this);
    m_aboutQtAction = new QAction(tr("About Qt"), this);

    m_helpMenu->addAction(m_helpAction);
    m_helpMenu->addAction(m_aboutApplicationAction);
    m_helpMenu->addAction(m_aboutQtAction);

    this->setWindowIcon(QIcon(":/logo_main.png"));
    //Подключения Action в Файл
    connect(m_helpAction, &QAction::triggered, this, &MainWindow::slotOpenMenuConsole);
    connect(m_openAction , &QAction::triggered, this, [=](){
        slotOpenFile("");
    });
    connect(m_closeAction, &QAction::triggered, this, &MainWindow::slotCloseFile);
    connect(m_exitAction, &QAction::triggered, qApp, &QApplication::quit);
    //Подключения Action в Настройках
    connect(m_languageGroup, &QActionGroup::triggered, this, &MainWindow::slotChangeLanguage);
    connect(m_consoleAction, &QAction::triggered, this, &MainWindow::slotOpenMenuConsole);

    m_newVersionTranslateAction->setCheckable(true);
    m_oldVersionTranslateAction->setCheckable(true);
    //Выбор версии перевода. Взаимоисключающий выбор
    connect(m_newVersionTranslateAction, &QAction::triggered, this, &MainWindow::slotChangedVersion);
    connect(m_oldVersionTranslateAction, &QAction::triggered, this, &MainWindow::slotChangedVersion);

    m_newVersionTranslateAction->setChecked(true); //По умолчанию перевод на новую версию
    m_oldVersionTranslateAction->setChecked(false);
    slotChangedVersion();

    //Подключения Action в Справка
    connect(m_helpAction, &QAction::triggered, this, &MainWindow::slotOpenManual);
    connect(m_aboutApplicationAction, &QAction::triggered, this, &MainWindow::slotAboutApplication);
    connect(m_aboutQtAction, &QAction::triggered, this, &MainWindow::slotOpenAboutQT);
    //Подключение QCheckBox
    connect(m_checkVolumeString, &QCheckBox::toggled, this, &MainWindow::slotDisplayVolumeString);
    connect(m_checkInstrumentString, &QCheckBox::toggled, this, &MainWindow::slotDisplayInstrumentString);

    //Смена партии, голоса и инструмента
    connect(m_comboPartySelection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::slotComboBoxPartsIndexChanged);
    connect(m_comboVoiceSelection, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &MainWindow::slotComboBoxVoicesIndexChanged);
    connect(m_comboInstrumentSelection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::slotComboBoxInstrumentsIndexChanged);
}

void MainWindow::convertToSequence(QVector<int>& converted, QVector<float>& duration, QVector<int>& league, NoteType type)
{
    if (converted.size() != duration.size()) {
        *sout << "Error: vectors have different sizes" << endl;
    }


    try {
        if (type == NoteType::Pitch) {//если ноты
            for (int i = 0; i < duration.size(); i++) {
                (print_amount(converted[i], duration[i], league[i], converted_notes_sequence));
            }
        }
        else if(type == NoteType::Octave){ // если октавы
            for (int i = 0; i < duration.size(); i++) {
                (print_amount(converted[i], duration[i], league[i], converted_octaves_sequence));
            }
        }

    }
    catch (const exception& ex) {
        *sout << ex.what() << endl;
    }
}

void MainWindow::print_amount(int object, int duration, int league, QVector <int>& converted_notes_sequence)
{
    for (int i = 0; i < duration; i++) {
        if (league) {
            if (league == 2) converted_notes_sequence.push_back(object);
            else if (league == 1 || league == 3)  converted_notes_sequence.push_back(15);
            league = 0;
        }
        else {
            if (i == 0) converted_notes_sequence.push_back(object);
            else converted_notes_sequence.push_back(15);
        }

    }

}

void MainWindow::initializeConsole()
{
    //Инициализация окна консоли
    console = new Debug;
    connect(console,&Debug::signalConsoleClose,this,&MainWindow::slotConsoleClose);
    //connect(m_consoleAction, &QAction::triggered, this, &MainWindow::slotConsoleClose);
    connect(this,&MainWindow::signalMainWindowClose,console,&Debug::SlotMainWindowClose);
    connect(this, &MainWindow::signalChangeLanguage, console, &Debug::slotTranslate); //Перевод
    //для передачи sout в консоль
    QTextEdit* textEdit = console->getTextEdit();
    sout = new QTextEditStreamOutput(textEdit); // Инициализация sout
}

void MainWindow::resetDefaultSettings(QSettings & settings)
{
    // Сбрасываем значения по умолчанию, если они не были найдены
    if (!settings.contains("App/Geometry")) {
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->availableGeometry();

        int width = screenGeometry.width() *0.6;
        int height = screenGeometry.height() *0.6;
        this->resize(width,height);
        this->move(screenGeometry.x() + (screenGeometry.width() - width) / 2,
                   screenGeometry.y() + (screenGeometry.height() - height) / 2);
        settings.setValue("App/Geometry", saveGeometry());
        *sout << "Установлен размер приложения по умолчанию." << endl;
    }
    if(!settings.contains("Console/Visible")){
        settings.setValue("Console/Visible", false);
        *sout << "Console сброшен." << endl;
    }
    if(!settings.contains("App/Version")){
        settings.setValue("App/Version", VersionTranslate::New);
        *sout << "Version сброшен." << endl;
    }
    if (!settings.contains("UI/CheckBoxVolume")) {
        settings.setValue("UI/CheckBoxVolume", true);
        *sout << "CheckBoxVolume сброшен." << endl;
    }
    if (!settings.contains("UI/CheckBoxInstrument")) {
        settings.setValue("UI/CheckBoxInstrument", true);
        *sout << "CheckBoxInstrument сброшен." << endl;
    }
    if (!settings.contains("UI/ComboBoxInstruments") || settings.value("UI/ComboBoxInstruments").toInt() < 0) {
        settings.setValue("UI/ComboBoxInstruments", 6);
        *sout << "ComboBoxInstruments сброшен." << endl;
    }
    if (!settings.contains("UI/LastOpenFilePath")) {
        settings.setValue("UI/LastOpenFilePath", QCoreApplication::applicationDirPath());
        *sout << "LastOpenFilePath сброшен." << endl;
    }
    //Если первый раз открываем приложение или ini файл повреждён, то берём язык из системы
    if(!settings.contains("App/Language")){
        QString localeName = QLocale::system().name().section('_',0,0);
        const QStringList availableLanguages = {"ru", "en"};
        if(!availableLanguages.contains(localeName)){//Если языка нет в списке, то английский
            settings.setValue("App/Language", "en");
        }
        else
            settings.setValue("App/Language", localeName);
        *sout << "Язык приложения установлен в соответствии с языком системы" << endl;
    }
}

void MainWindow::loadSettings(QSettings &settings)
{
    //Геометрия
    QByteArray geometry = settings.value("App/Geometry").toByteArray();
    if(!geometry.isEmpty())
        restoreGeometry(geometry);
    //Консоль
    //Делаем кликабельным action
    m_consoleAction->setCheckable(true);
    bool isConsoleOpen = settings.value("Console/Visible").toBool();
    if(isConsoleOpen){
        m_consoleAction->setChecked(true);
    }
    else{
        m_consoleAction->setChecked(false);
    }
    slotOpenMenuConsole(isConsoleOpen);
    //Версия перевода
    int versionTranslate = settings.value("App/Version").toInt();
    for(auto action : m_versionMenu->actions()){
        if(action->data().toInt() == versionTranslate){
            action->setChecked(true);
            break;
        }
    }
    //Язык
    QString localeName = settings.value("App/Language").toString();
    //m_languageGroup->blockSignals(true);
    //Устанавливаем нужный action с датой в виде локали
    for(auto action : m_languageGroup->actions()){
        if(action->data().toString() == localeName){
            action->setChecked(true);
            emit m_languageGroup->triggered(action);
            break;
        }
    }
    //m_languageGroup->blockSignals(false);
    //Строка громкости
    m_checkVolumeString->blockSignals(true);
    m_checkVolumeString->setChecked(settings.value("UI/CheckBoxVolume").toBool());
    m_checkVolumeString->blockSignals(false);
    //Строка интрумента
    m_checkInstrumentString->blockSignals(true);
    m_checkInstrumentString->setChecked(settings.value("UI/CheckBoxInstrument").toBool());
    m_checkInstrumentString->blockSignals(false);
    //Последний выбранный инструмент в зависимости от версии перевода и языка приложения
    m_comboInstrumentSelection->blockSignals(true);
    int instrumentId = settings.value("UI/ComboBoxInstruments").toInt();
    updateInstrumentsComboBox();
    int index = m_comboInstrumentSelection->findData(instrumentId);
    if (index != -1)
        m_comboInstrumentSelection->setCurrentIndex(index);
    else
        m_comboInstrumentSelection->setCurrentIndex(Instrument::Piano);
    m_comboInstrumentSelection->blockSignals(false);

    //Временно отключаем виджеты до открытия файла
    m_comboPartySelection->setEnabled(false);
    m_comboVoiceSelection->setEnabled(false);
    m_checkVolumeString->setEnabled(false);
    m_checkInstrumentString->setEnabled(false);
}

void MainWindow::saveSettings(QSettings &settings)
{
    //Геометрия
    settings.setValue("App/Geometry", saveGeometry());
    //Консоль
    settings.setValue("Console/Visible",m_consoleAction->isChecked());
    //Версия перевода
    for(auto action : m_versionMenu->actions()){
        if(action->isChecked()){
            settings.setValue("App/Version",action->data());
            break;
        }
    }
    //Язык
    for(auto action : m_languageGroup->actions()){
        if(action->isChecked()){
            settings.setValue("App/Language",action->data());
            break;
        }
    }
    //Строка громкости
    settings.setValue("UI/CheckBoxVolume", m_checkVolumeString->isChecked());
    //Строка интрумента
    settings.setValue("UI/CheckBoxInstrument", m_checkInstrumentString->isChecked());
    //Последний выбранный инструмент в зависимости от версии перевода и языка приложения
    settings.setValue("UI/ComboBoxInstruments",m_comboInstrumentSelection->currentData());
}

void MainWindow::updateInstrumentsComboBox()
{
    m_comboInstrumentSelection->blockSignals(true);
    //Получаем текущий выбранный инструмент
    int currentData = m_comboInstrumentSelection->currentData().toInt();
    m_comboInstrumentSelection->clear();

    const QStringList instrumentsList  =
    {tr("1. Bell"),
    tr("2. Organ"),
    tr("3. Ping"),
    tr("4. Strings"),
    tr("5. Trumpet"),
    tr("6. Voice"),
    tr("7. Piano"),
    tr("8. Piano Sustain"),
    tr("10. Bass")};

    //Добавляем все инструменты, кроме последнего из списка
    for (int i = 0; i < 8;i++) {
        m_comboInstrumentSelection->addItem(instrumentsList.value(i), i + 1);
    }
    //Если новая версия, то добавляем последний инструмент
    if(m_newVersionTranslateAction->isChecked()){
        m_comboInstrumentSelection->addItem(instrumentsList.last(),Instrument::Bass);
        m_instrOctRangeCurrent = m_instrOctRangeNew; //заменяем map новыми инструментами
    }
    else if(m_oldVersionTranslateAction->isChecked()){
        m_comboInstrumentSelection->setItemText(0, tr("1. Bell (The first octave is broken, see manual)"));
        m_instrOctRangeCurrent = m_instrOctRangeOld;
    }
    //Выбираем изначальный инструмент, иначе фортепиано по умолчанию
    int index = m_comboInstrumentSelection->findData(currentData);
    if(index > -1){
        m_comboInstrumentSelection->setCurrentIndex(index);
    }
    else{
        //индексы начинаются с 0, поэтому -1
        m_comboInstrumentSelection->setCurrentIndex(Instrument::Piano - 1);
    }
    m_comboInstrumentSelection->blockSignals(false);
}

void MainWindow::initializeSettings()
{
    setWindowTitle(tr("Survivalcraft notes translator:"));//Задаём название приложения
    initializeConsole();//Загружаем консоль
    QSettings settings("config.ini", QSettings::IniFormat); //Открываем ini файл
    resetDefaultSettings(settings);
    loadSettings(settings);
}

void MainWindow::loadHelpLibrary()
{
    QString oldPath = QDir::currentPath();

    QString dllPath = QCoreApplication::applicationDirPath() + "/Help.dll";
    helpLib.setFileName(dllPath);
    //Ищем библиотку рядом с exe и в папке guide
    if (!helpLib.load()) {
        *sout << "Библиотека справки не найдена по основному пути!\n";

        QString libFullPath = QCoreApplication::applicationDirPath() + "/guide/help.dll";
        QFileInfo libInfo(libFullPath);

        if (libInfo.exists()) {
            QDir::setCurrent(libInfo.absolutePath());
            helpLib.setFileName("Help");

            if (!helpLib.load()) {
                *sout << "Библиотека справки не найдена по резервному пути!\n";
                QDir::setCurrent(oldPath);
                return;
            }
        }
    }

    CreateHelpWidget createHelpWidget = (CreateHelpWidget)helpLib.resolve("createHelpWidget");

    if (!createHelpWidget) {
        *sout << "Не удалось найти функцию createHelpWidget в библиотеке!\n";
        helpLib.unload();
        QDir::setCurrent(oldPath);
        return;
    }

    m_helpWidget = createHelpWidget(this);

    int signalIndex = this->metaObject()->indexOfSignal("signalChangeLanguage()");
    int slotIndex = m_helpWidget->metaObject()->indexOfSlot("slotTranslate()");

    QMetaObject::connect(this, signalIndex, m_helpWidget, slotIndex);//Перевод текста в dll
    QDir::setCurrent(oldPath); // восстановление рабочей директории
}

void MainWindow::show_notes(QVector<int> &converted_notes)
{
    *sout << "Ноты " << IdParts[m_comboPartySelection->currentIndex()].toStdString() << " " << m_comboVoiceSelection->currentText().toStdString() << endl;
    m_translatedArea->append(tr("Notes ") + IdParts[m_comboPartySelection->currentIndex()]+ "," + m_comboVoiceSelection->currentText() + "\n");

    int count = 0;
    for (int& i : converted_notes_sequence) {
        if (count == 256) {
            *sout << endl;
            m_translatedArea->moveCursor(QTextCursor::End);
            m_translatedArea->insertPlainText("\n");
            count = 0;
        }
        *sout << hex << uppercase << i;
        m_translatedArea->moveCursor(QTextCursor::End);
        m_translatedArea->insertPlainText(QString::number(i, 16).toUpper());
        count++;
    }
    while (count != 256) {
        *sout << "F";
        m_translatedArea->moveCursor(QTextCursor::End);
        m_translatedArea->insertPlainText("F");

        count++;
    }
    *sout << endl;
    m_translatedArea->moveCursor(QTextCursor::End);
    m_translatedArea->insertPlainText("\n");

}

void MainWindow::show_octaves(QVector<int> &converted_octaves)
{
    *sout << "Октавы " << IdParts[m_comboPartySelection->currentIndex()].toStdString() << " " << m_comboVoiceSelection->currentText().toStdString() << endl;
    m_translatedArea->append(tr("Octaves ") + IdParts[m_comboPartySelection->currentIndex()]+ "," + m_comboVoiceSelection->currentText() + "\n");

    int count = 0;
    for (int& i : converted_octaves_sequence) {
        if (count == 256) {
            *sout << endl;
            m_translatedArea->moveCursor(QTextCursor::End);
            m_translatedArea->insertPlainText("\n");
            count = 0;
        }
        *sout << hex << uppercase << i;
        m_translatedArea->moveCursor(QTextCursor::End);
        m_translatedArea->insertPlainText(QString::number(i, 16).toUpper());
        count++;
    }
    while (count != 256) {
        *sout << "F";
        m_translatedArea->moveCursor(QTextCursor::End);
        m_translatedArea->insertPlainText("F");
        count++;
    }
    *sout << endl;
    m_translatedArea->moveCursor(QTextCursor::End);
    m_translatedArea->insertPlainText("\n");
}

void MainWindow::convert_to_sequence_percussion(QVector<string> &, QVector<float> &)
{
    if (v_instruments.size() != v_duration.size()) { // если размеры не совпадают
        throw std::invalid_argument("Sizes of vectors v_instruments and v_duration are not equal");
    }
    converted_notes.clear();
    for (int i = 0; i < v_instruments.size(); i++) {
        converted_notes.push_back(PercussionSurvivalcraftMap[PercussionPartInstruments[v_instruments[i]]]);
    }
    for (int i = 0; i < v_duration.size(); i++) {
        (print_amount(converted_notes[i], v_duration[i], 0, converted_notes_sequence));
    }
}

void MainWindow::show_instruments()
{
    if(m_checkInstrumentString->isChecked()){
        *sout << "Инструменты " << IdParts[m_comboPartySelection->currentIndex()].toStdString() << " " << m_comboVoiceSelection->currentText().toStdString() << endl;
        m_translatedArea->append(tr("Instruments ") + IdParts[m_comboPartySelection->currentIndex()]+ "," + m_comboVoiceSelection->currentText() + "\n");

        int count = 0;
        for (int& i : converted_notes_sequence) {
            if (count == 256) {
                *sout << endl;
                m_translatedArea->moveCursor(QTextCursor::End);
                m_translatedArea->insertPlainText("\n");
                count = 0;
            }
            *sout << hex << uppercase << i;
            m_translatedArea->moveCursor(QTextCursor::End);
            m_translatedArea->insertPlainText(QString::number(m_comboInstrumentSelection->currentData().toInt(),16).toUpper());
            count++;
        }
        while (count != 256) {
            *sout << "F";
            m_translatedArea->moveCursor(QTextCursor::End);
            m_translatedArea->insertPlainText(QString::number(m_comboInstrumentSelection->currentData().toInt(),16).toUpper());
            count++;
        }
        *sout << endl;
        m_translatedArea->moveCursor(QTextCursor::End);
        m_translatedArea->insertPlainText("\n");
    }
}

void MainWindow::show_volumes()
{
    if(m_checkVolumeString->isChecked()){ //если выбрано вывод строки инструментов
        *sout << "Громкость " << IdParts[m_comboPartySelection->currentIndex()].toStdString() << " " << m_comboVoiceSelection->currentText().toStdString() << endl;
        m_translatedArea->append(tr("Volume ") + IdParts[m_comboPartySelection->currentIndex()]+ "," + m_comboVoiceSelection->currentText() + "\n");

        int count = 0;
        for (int& i : converted_notes_sequence) {
            if (count == 256) {
                *sout << endl;
                m_translatedArea->moveCursor(QTextCursor::End);
                m_translatedArea->insertPlainText("\n");
                count = 0;
            }
            *sout << "F";
            m_translatedArea->moveCursor(QTextCursor::End);
            m_translatedArea->insertPlainText("F");
            count++;
        }
        while (count != 256) {
            *sout << "F";
            m_translatedArea->moveCursor(QTextCursor::End);
            m_translatedArea->insertPlainText("F");
            count++;
        }
        *sout << endl;
        m_translatedArea->moveCursor(QTextCursor::End);
        m_translatedArea->insertPlainText("\n");
    }
}

void MainWindow::slotDisplayInstrumentString(int arg1)
{
    slotComboBoxPartsIndexChanged(m_comboPartySelection->currentIndex());
}

void MainWindow::slotAboutApplication()
{
    About about(this);
    about.exec();
}

void MainWindow::slotDisplayVolumeString(int arg1)
{
    slotComboBoxPartsIndexChanged(m_comboPartySelection->currentIndex());
}

void MainWindow::slotOpenManual() {
    //Если нет dll, то показываем заглушку
    if(!m_helpWidget){
        QMessageBox warning;
        warning.setIcon(QMessageBox::Warning);
        warning.setWindowTitle(tr("Warning"));
        QString url = "https://github.com/Pigeon-Ignaty/survivalcraft-notes-translator/releases";
        QString message = tr("Failed to load the manual! Most likely, it was not downloaded.<br>Please visit the ");
        QString linkText = tr("releases page");
        warning.setText(message + QString("<a href=\"%1\">%2</a>.").arg(url, linkText));
        warning.setTextInteractionFlags(Qt::TextBrowserInteraction);
        warning.setWindowIcon(QIcon(":/pigeon.jpg"));
        warning.exec();
        return;
    }
    //Иначе показываем справку
    m_helpWidget->show();
}

void MainWindow::slotChangeLanguage(QAction *action)
{
    //Если русский, то устанавливаем m_translator
    if(action->data().toString() == Language::ru){
        m_translator = new QTranslator(this);

        if(m_translator->load(":/translations/SCNotesTranslator_ru.qm"))
            qApp->installTranslator(m_translator);
    }
    //Английский - удаляем
    else if(action->data().toString() == Language::en){
        if(m_translator){
            qApp->removeTranslator(m_translator);
            delete m_translator;
            m_translator = nullptr;
        }
    }
    retranslateUI();
    emit signalChangeLanguage();
}

void MainWindow::slotChangedVersion()
{
    updateInstrumentsComboBox();
    slotComboBoxPartsIndexChanged(m_comboPartySelection->currentIndex());
}
