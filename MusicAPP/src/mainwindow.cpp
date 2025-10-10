#include "mainwindow.h"
#include <QMenu>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi();
    initializeSettings();
}


MainWindow::~MainWindow()
{
    if (help != nullptr) {
            delete help;
        }
}

void MainWindow::slotOpenFile() //открытие файла
{
    QSettings settings("config.ini", QSettings::IniFormat);
    QString path_to_file = QFileDialog::getOpenFileName(this,"Выбор .musicxml файла",settings.value("lastOpenedPath","").toString(),"*.musicxml *.mxl");//"Z:\\VS\\Survivalcraft-notes-translator gui\\survivalcraft-notes-translator\\MusicAPP\\xml"
    if(path_to_file.isEmpty()){
        return;
    }
    if(!path_to_file.isEmpty()){
        settings.setValue("lastOpenedPath", path_to_file);
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
        errorMessage.setText("Пук-среньк!");
        errorMessage.setInformativeText(u8"Произошла ошибка. Файл не найден, либо имеет неправильное расширение, либо содержит ошибку.");
        errorMessage.setWindowTitle("Ошибка");
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
                *sout << u8"Вывод следующих партий в музыкальной композиции: " << endl;
                int scorePartCount = 0;
                XMLElement* part_list = pRootElement->FirstChildElement("part-list");
                if (part_list != nullptr) {
                    for (XMLElement* score_part = part_list->FirstChildElement("score-part");
                        score_part != nullptr; score_part = score_part->NextSiblingElement("score-part"))
                    {
                        *sout << "ID: " << score_part->Attribute("id") << "\t";
                        XMLElement* part_name = score_part->FirstChildElement("part-name");
                        if (part_name != nullptr) {
                            *sout << u8"Название партии: " << string(part_name->GetText()) << endl;
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
                setWindowTitle(tr("MusicAPP translator: - %1").arg(fileName));//добавление в шапку
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

void MainWindow::SlotConsoleClose()
{
    *sout << "console close" << endl;
    m_consoleAction->setChecked(false);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("config.ini", QSettings::IniFormat); //сохранение настроек пользователя
    settings.setValue("myWidget/geometry", saveGeometry());
    settings.setValue("CheckBoxVolume", m_checkVolumeString->isChecked());
    settings.setValue("CheckBoxInstrument", m_checkInstrumentString->isChecked());
    settings.setValue("ComboBoxInstruments",m_comboInstrumentSelection->currentIndex());
    settings.setValue("MenuConsole",m_consoleAction->isChecked());
    emit SignalMainWindowClose();
    event->accept();
}


void MainWindow::slotOpenAboutQT()//о версии qt
{
    QMessageBox aboutQt;
    aboutQt.setWindowIcon(QIcon(":/logo_qt.ico"));
    QMessageBox::aboutQt(&aboutQt,"О версии QT");

}

void MainWindow::slotSaveUISettings()
{
    QSettings settings("config.ini", QSettings::IniFormat); //сохранение настроек пользователя
    settings.setValue("myWidget/geometry", saveGeometry());
    settings.setValue("CheckBoxVolume", m_checkVolumeString->isChecked());
    settings.setValue("CheckBoxInstrument", m_checkInstrumentString->isChecked());
    settings.setValue("ComboBoxInstruments",m_comboInstrumentSelection->currentIndex());
    settings.setValue("MenuConsole",m_consoleAction->isChecked());
    this->close();
    emit SignalMainWindowClose();
}

void MainWindow::slotCloseFile()//закрытие через menu bar
{
    m_openAction ->setEnabled(true);
    m_closeAction->setEnabled(false);
    setWindowTitle(tr("MusicAPP translator:"));
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
    *sout << u8"Голоса, обнаруженные в партии" << std::endl;
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


                        *sout << u8"Measure (такт): " << count_measure << endl;
                        if (fifths_tag != nullptr) {
                            fifths = atoi(fifths_tag->GetText());
                            *sout << u8"\tЗнаки: " << fifths << endl;
                        }

                        if (beats_tag != nullptr) {
                            beats = atoi(beats_tag->GetText());
                            *sout << u8"\tдолей в числителе тактового размера: " << beats_tag->GetText() << endl;
                        }

                        if (beat_type_tag != nullptr) {
                            beat_type = atoi(beat_type_tag->GetText());
                            *sout << u8"\t долей в знаменателе тактового размера: " << beat_type_tag->GetText() << endl;
                        }

                        if (sign_tag != nullptr) {
                            sign = string(sign_tag->GetText());
                            *sout << u8"\tsign (знак): " << sign << endl;
                        }

                        if (line_tage != nullptr) {
                            line = atoi(line_tage->GetText());
                            *sout << u8"\tline (номер для знака): " << line << endl;
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
    *sout << u8"Выбран инструмент " << m_comboInstrumentSelection->currentText().toStdString() << endl;
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
         *sout << u8"Ноты в notes_f:" << endl;
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
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть ZIP файл.");
        return 0;
    }

    // Получаем количество файлов в архиве
    int numFiles = mz_zip_reader_get_num_files(&zipArchive);
    if (numFiles < 1) {
        *sout << "В архиве нет файлов." << endl;
        QMessageBox::critical(this, "Ошибка", "В архиве нет файлов.");
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
            QMessageBox::critical(this, "Ошибка", "Не удалось прочитать файл в архиве.");
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
                QMessageBox::critical(this, "Ошибка", "Не удалось извлечь файл.");
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
        QMessageBox::critical(this, "Ошибка", "Не удалось найти файл с именем архива в архиве.");
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
    m_labelPartySelection = new QLabel("Выбор партии");
    m_labelPartySelection->setFixedHeight(25);
    m_labelPartySelection->setFont(commonFont);

    m_comboPartySelection = new QComboBox;
    m_comboPartySelection->setFixedHeight(25);
    m_comboPartySelection->setFont(commonFont);

    m_checkVolumeString = new QCheckBox("Строка громкости");
    m_checkVolumeString->setFixedHeight(25);
    m_checkVolumeString->setFont(commonFont);

    m_partColumn->addWidget(m_labelPartySelection);
    m_partColumn->addWidget(m_comboPartySelection);
    m_partColumn->addWidget(m_checkVolumeString);
    m_partColumn->addStretch();

    //---------- Second column
    m_labelVoiceSelection = new QLabel("Выбор голоса");
    m_labelVoiceSelection->setFixedHeight(25);
    m_labelVoiceSelection->setFont(commonFont);

    m_comboVoiceSelection = new QComboBox;
    m_comboVoiceSelection->setFixedHeight(25);
    m_comboVoiceSelection->setFont(commonFont);

    m_checkInstrumentString = new QCheckBox("Строка инструмента");
    m_checkInstrumentString->setFixedHeight(25);
    m_checkInstrumentString->setFont(commonFont);

    m_voiceColumn->addWidget(m_labelVoiceSelection);
    m_voiceColumn->addWidget(m_comboVoiceSelection);
    m_voiceColumn->addWidget(m_checkInstrumentString);
    m_voiceColumn->addStretch();

    //---------- Third column
    m_labelInstrumentSelection = new QLabel("Выбор инструмента");
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
    m_fileMenu = menuBar()->addMenu("Файл");

    m_openAction = new QAction("Открыть", this);
    m_openAction->setShortcut(QKeySequence::Open);

    m_closeAction = new QAction("Закрыть", this);
    m_closeAction->setShortcut(QKeySequence::Close);
    m_closeAction->setEnabled(false);  // Пока отключено

    m_exitAction = new QAction("Выход", this);
    m_exitAction->setShortcut(QKeySequence::Quit);

    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addAction(m_closeAction);
    m_fileMenu->addAction(m_exitAction);

    //---------- Menu: Настройки
    m_settingsMenu = menuBar()->addMenu("Настройки");

    m_configDrumsAction = new QAction("Конфигуратор ударных инструментов", this);
    m_configDrumsAction->setEnabled(false);

    m_configStringNameAction = new QAction("Настройка названия строк ", this);
    m_configStringNameAction->setEnabled(false);

    m_consoleAction = new QAction("Отладочная информация", this);

    m_versionMenu = new QMenu("Версия", this);
    m_versionBefore24SubAction = new QAction("До 2.4 (устаревшая)", this);
    m_versionAfter24SubAction = new QAction("После 2.4 (включая)", this);
    QActionGroup *versionGroup = new QActionGroup(this);
    versionGroup->setExclusive(true);
    versionGroup->addAction(m_versionBefore24SubAction);
    versionGroup->addAction(m_versionAfter24SubAction);
    m_versionBefore24SubAction->setData(0);
    m_versionAfter24SubAction->setData(1);

    m_versionMenu->addAction(m_versionBefore24SubAction);
    m_versionMenu->addAction(m_versionAfter24SubAction);

    m_settingsMenu->addAction(m_configDrumsAction);
    m_settingsMenu->addAction(m_configStringNameAction);
    m_settingsMenu->addAction(m_consoleAction);
    m_settingsMenu->addMenu(m_versionMenu);

    //---------- Menu: Справка
    m_helpMenu = menuBar()->addMenu("Справка");

    m_helpAction = new QAction("Руководство", this);
    m_aboutApplicationAction = new QAction("О программе", this);
    m_aboutQtAction = new QAction("О Qt", this);

    m_helpMenu->addAction(m_helpAction);
    m_helpMenu->addAction(m_aboutApplicationAction);
    m_helpMenu->addAction(m_aboutQtAction);

    //---------- Размер окна
    this->resize(900, 500);
    this->setWindowIcon(QIcon(":/logo_main.png"));
    //Подключения Action в Файл
    connect(m_helpAction, &QAction::triggered, this, &MainWindow::slotOpenMenuConsole);
    connect(m_openAction , &QAction::triggered, this, &MainWindow::slotOpenFile);
    connect(m_closeAction, &QAction::triggered, this, &MainWindow::slotCloseFile);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::slotSaveUISettings);

    //Подключения Action в Настройках
    connect(m_consoleAction, &QAction::triggered, this, &MainWindow::slotOpenMenuConsole);

    m_versionAfter24SubAction->setCheckable(true);
    m_versionBefore24SubAction->setCheckable(true);
    //Выбор версии перевода. Взаимоисключающий выбор
    connect(m_versionAfter24SubAction, &QAction::triggered, this, &MainWindow::slotChangedVersion);
    connect(m_versionBefore24SubAction, &QAction::triggered, this, &MainWindow::slotChangedVersion);

    m_versionAfter24SubAction->setChecked(false); //По умолчанию перевод на новую версию
    m_versionBefore24SubAction->setChecked(true);
    slotChangedVersion();

    //Подключения Action в Справка
    connect(m_helpAction, &QAction::triggered, this, &MainWindow::slotOpenGuideWindow);
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

void MainWindow::initializeSettings()
{
    //Инициализация окна консоли
    console = new Debug;
    connect(console,&Debug::signalConsoleClose,this,&MainWindow::SlotConsoleClose);//закрытие консоли и снятие галочки
    connect(this,&MainWindow::SignalMainWindowClose,console,&Debug::SlotMainWindowClose);//закрытие консоли если закрыли главное окно
    m_consoleAction->setCheckable(true);//по умолчанию кнопка bool в menu bar
    m_consoleAction->setChecked(true);//по умолчанию она выключена
    m_closeAction->setEnabled(false); //кнопка закрытия неактивна после запуска программы
    m_comboPartySelection->setEnabled(false);// списки не активны, если не открыт файл
    m_comboVoiceSelection->setEnabled(false);
    m_checkInstrumentString->setEnabled(false);
    m_checkVolumeString->setEnabled(false);
    m_comboInstrumentSelection->blockSignals(true);
    m_comboInstrumentSelection->setCurrentIndex(6 + (m_versionAfter24SubAction->isChecked() ? 1 : 0)); // по умолчанию пианино
    m_comboPartySelection->blockSignals(true);//отключаем слот изменения содержимого выпадающего списка
    m_comboVoiceSelection->blockSignals(true);//отключаем слот изменения содержимого выпадающего списка
    setWindowTitle(tr("MusicAPP translator:"));//название окна при запуске

    QSettings settings("config.ini", QSettings::IniFormat); //Восстановление настроек пользователя
    //для передачи sout в консоль
    QTextEdit* textEdit = console->getTextEdit();
    sout = new QTextEditStreamOutput(textEdit); // Инициализация sout
           // Проверка наличия и корректности значений в ini файле
           if (settings.contains("myWidget/geometry")) {
               // Попытка восстановления геометрии
               QByteArray geometry = settings.value("myWidget/geometry").toByteArray();
               if (!geometry.isEmpty() && restoreGeometry(geometry)) {
                   *sout << u8"Успешное восстановление геометрии" << endl;
                   }
               }
               if (!settings.contains("CheckBoxVolume")) {
                   settings.setValue("CheckBoxVolume", true);
                   *sout << "CheckBoxVolume сброшен." << endl;
               }
               if (!settings.contains("CheckBoxInstrument")) {
                   settings.setValue("CheckBoxInstrument", true);
                   *sout << "CheckBoxInstrument сброшен." << endl;
               }
               if (!settings.contains("ComboBoxInstruments") || settings.value("ComboBoxInstruments").toInt() < 0) {
                   settings.setValue("ComboBoxInstruments", 6);
                   *sout << "ComboBoxInstruments сброшен." << endl;
               }
               if (!settings.contains("MenuConsole")) {
                   settings.setValue("MenuConsole", false);
                   *sout << "MenuConsole сброшен." << endl;
               }
               if (!settings.contains("lastOpenedPath")) {
                   settings.setValue("lastOpenedPath", QCoreApplication::applicationDirPath());
                   *sout << "lastOpenedPath сброшен." << endl;
               }

            // чтение настроек и применение
            m_checkVolumeString->blockSignals(true);
            m_checkInstrumentString->blockSignals(true);
            m_consoleAction->blockSignals(true);
            m_checkVolumeString->setChecked(settings.value("CheckBoxVolume", false).toBool());
            m_checkInstrumentString->setChecked(settings.value("CheckBoxInstrument", false).toBool());
            m_comboInstrumentSelection->setCurrentIndex(settings.value("ComboBoxInstruments", 0).toInt());
            bool b = settings.value("MenuConsole", false).toBool();
            if(b){
                console->show();
                m_consoleAction->setChecked(true);
                }
            else{
                console->hide();
                m_consoleAction->setChecked(false);
                }
            m_checkVolumeString->blockSignals(false);
            m_checkInstrumentString->blockSignals(false);
            m_consoleAction->blockSignals(false);
            m_comboInstrumentSelection->blockSignals(false);

}

void MainWindow::show_notes(QVector<int> &converted_notes)
{
    *sout << u8"Ноты " << IdParts[m_comboPartySelection->currentIndex()].toStdString() << " " << m_comboVoiceSelection->currentText().toStdString() << endl;
    m_translatedArea->append("Ноты " + IdParts[m_comboPartySelection->currentIndex()]+ "," + m_comboVoiceSelection->currentText() + "\n");

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
    *sout << u8"Октавы " << IdParts[m_comboPartySelection->currentIndex()].toStdString() << " " << m_comboVoiceSelection->currentText().toStdString() << endl;
    m_translatedArea->append("Октавы " + IdParts[m_comboPartySelection->currentIndex()]+ "," + m_comboVoiceSelection->currentText() + "\n");

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
        *sout << u8"Инструменты " << IdParts[m_comboPartySelection->currentIndex()].toStdString() << " " << m_comboVoiceSelection->currentText().toStdString() << endl;
        m_translatedArea->append("Инструменты " + IdParts[m_comboPartySelection->currentIndex()]+ "," + m_comboVoiceSelection->currentText() + "\n");

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
        *sout << u8"Громкость " << IdParts[m_comboPartySelection->currentIndex()].toStdString() << " " << m_comboVoiceSelection->currentText().toStdString() << endl;
        m_translatedArea->append("Громкость " + IdParts[m_comboPartySelection->currentIndex()]+ "," + m_comboVoiceSelection->currentText() + "\n");

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

    aboutWindow = new About(nullptr);
    aboutWindow->setWindowTitle("О программе");
    aboutWindow->show();
}

void MainWindow::slotDisplayVolumeString(int arg1)
{
    slotComboBoxPartsIndexChanged(m_comboPartySelection->currentIndex());
}

void MainWindow::slotOpenGuideWindow() {
    if (help == nullptr) {
        // Создаем окно справки без родительского виджета
        help = new HelpDialog(nullptr);
        help->setWindowTitle("Справка");

        // Подключаем сигнал destroyed, чтобы обнулить указатель при закрытии окна
        connect(help, &HelpDialog::destroyed, this, [this]() {
            help = nullptr;
        });

        // Подключаем сигнал закрытия, если требуется
        connect(help, &HelpDialog::closed, this, &MainWindow::handleHelpDialogClosed);

        help->show();
    } else {
        // Если окно уже существует, активируем его
        help->raise();
        help->activateWindow();
    }
}

void MainWindow::slotChangedVersion()
{
    const QStringList oldInstruments  = {"1. Колокольчик (сломана октава, см. руководство)",
                                   "2. Óрган",
                                   "3. 8 бит",
                                   "4. Струнный инструмент",
                                   "5. М̶̶̶е̶̶̶д̶̶̶н̶̶̶а̶̶̶я̶̶̶ труба",
                                   "6. Вокальное ду",
                                   "7. Фортепиано",
                                   "8. Удлинённое фортепиано"};

    m_comboInstrumentSelection->blockSignals(true);
    m_comboInstrumentSelection->clear();
    int i = 1;
    for(const auto &instrument : oldInstruments)
        m_comboInstrumentSelection->addItem(instrument,i++);

        if(m_versionAfter24SubAction->isChecked()){//Если версия после 2.4, добавляем один инструмент
        m_comboInstrumentSelection->addItem("10. Бас-гитара",10);
        m_instrOctRangeCurrent = m_instrOctRangeNew;
    }
    else
        m_instrOctRangeCurrent = m_instrOctRangeOld;

    m_comboInstrumentSelection->blockSignals(false);

    const int index = m_comboPartySelection->currentIndex();
    //slotComboBoxPartsIndexChanged(-1);
    slotComboBoxPartsIndexChanged(index);


}
void MainWindow::handleHelpDialogClosed() {
    // Обработка закрытия окна
    help = nullptr;
}
