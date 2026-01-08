#include "TranslateMusic.h"

TranslateMusic::TranslateMusic(std::ostream& out): sout(out) {}

void TranslateMusic::parsingData(const QString idPart, const QString idVoice, const int instrument, tinyxml2::XMLDocument *doc)
{
    clearData();
    tinyxml2::XMLElement* pRootElement = doc->RootElement(); // корневой каталог
    if (nullptr != pRootElement) { // если не пустой
        tinyxml2::XMLElement* part_list_tag = pRootElement->FirstChildElement("part-list");
        if (part_list_tag != nullptr) {
            tinyxml2::XMLElement* score_part_tag = part_list_tag->FirstChildElement("score-part");
            while (score_part_tag) {
                const char* ScorePartId = score_part_tag->Attribute("id");
                if (ScorePartId == idPart) {
                    tinyxml2::XMLElement* score_instrument_tag = score_part_tag->FirstChildElement("score-instrument");
                    if (score_instrument_tag != nullptr) {
                        while (score_instrument_tag) {
                            tinyxml2::XMLElement* instrument_name_tag = score_instrument_tag->FirstChildElement("instrument-name");
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
        tinyxml2::XMLElement* part_id_tag = pRootElement->FirstChildElement("part");
        tinyxml2::XMLElement* measure_tag = nullptr;
        while (part_id_tag) {
            const char* partId = part_id_tag->Attribute("id");
            if (partId && (partId == idPart)) {
                measure_tag = part_id_tag->FirstChildElement("measure");
                // Анализируем каждый measure

                while (measure_tag) { // если не пустой
                    tinyxml2::XMLElement* attributes_tag = nullptr;
                    tinyxml2::XMLElement* divisions_tag = nullptr;
                    tinyxml2::XMLElement* key_tag = nullptr;
                    tinyxml2::XMLElement* fifths_tag = nullptr;
                    tinyxml2::XMLElement* time_tag = nullptr;
                    tinyxml2::XMLElement* beats_tag = nullptr;
                    tinyxml2::XMLElement* beat_type_tag = nullptr;
                    tinyxml2::XMLElement* clef_tag = nullptr;
                    tinyxml2::XMLElement* sign_tag = nullptr;
                    tinyxml2::XMLElement* line_tage = nullptr;
                    tinyxml2::XMLElement* transpose_tag = nullptr;
                    tinyxml2::XMLElement* direction_tag = nullptr;
                    tinyxml2::XMLElement* direction_type_tag = nullptr;
                    tinyxml2::XMLElement* metronome_tag = nullptr;
                    tinyxml2::XMLElement* beat_unit_tag = nullptr;
                    tinyxml2::XMLElement* per_minute_tag = nullptr;

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


                    sout << "Measure (такт): " << count_measure << "\n";
                    if (fifths_tag != nullptr) {
                        fifths = atoi(fifths_tag->GetText());
                        sout << "\tЗнаки: " << fifths << "\n";
                    }

                    if (beats_tag != nullptr) {
                        beats = atoi(beats_tag->GetText());
                        sout << "\tдолей в числителе тактового размера: " << beats_tag->GetText() << "\n";
                    }

                    if (beat_type_tag != nullptr) {
                        beat_type = atoi(beat_type_tag->GetText());
                        sout << "\t долей в знаменателе тактового размера: " << beat_type_tag->GetText() << "\n";
                    }

                    if (sign_tag != nullptr) {
                        sign = string(sign_tag->GetText());
                        sout << "\tsign (знак): " << sign << "\n";
                    }

                    if (line_tage != nullptr) {
                        line = atoi(line_tage->GetText());
                        sout << "\tline (номер для знака): " << line << "\n";
                    }

                    //////Читаем ноты
                    tinyxml2::XMLElement* note_tag = measure_tag->FirstChildElement("note");
                    tinyxml2::XMLElement* voice_tag = note_tag->FirstChildElement("voice");

                    QString current_voice = voice_tag->GetText();
                    bool flag_voice = false; //проверка на отсутсвие голоса в такте
                    while (note_tag) { // цикл по  нотам в measure
                        if(current_voice == idVoice){
                            flag_voice = true;
                            if (note_tag->FirstChildElement("pitch") != nullptr) { //если нота, то вывести ноту и октаву
                                tinyxml2::XMLElement* step_tag = note_tag->FirstChildElement("pitch")->FirstChildElement("step"); //нота
                                tinyxml2::XMLElement* alter_tag = note_tag->FirstChildElement("pitch")->FirstChildElement("alter"); //отклонение на полутон
                                tinyxml2::XMLElement* octave_tag = note_tag->FirstChildElement("pitch")->FirstChildElement("octave"); //октава
                                if (step_tag != nullptr) {
                                    sout << "v_notes " << v_notes.size() - 1 << "\t"; // для консоли
                                    sout << "\tstep: " << step_tag->GetText() << "\n"; // нота
                                    v_notes.push_back(step_tag->GetText()[0]);
                                    v_league.push_back(0);
                                }
                                if (alter_tag != nullptr) {
                                    alter = atoi(alter_tag->GetText());
                                    sout << "v_semitone " << v_semitone.size() - 1 << "\t";

                                    sout << "\talter: " << alter_tag->GetText() << "\n";
                                    v_semitone.push_back(atoi(alter_tag->GetText()));

                                }
                                else { //если нет alter
                                    sout << "v_semitone " << v_semitone.size() - 1 << "\t";

                                    v_semitone.push_back(0);

                                }
                                if (octave_tag != nullptr) {
                                    octave = atoi(octave_tag->GetText());
                                    sout << "\toctave: " << octave_tag->GetText() << "\n";
                                    v_octaves.push_back(octave_tag->GetText());

                                }
                            }
                            tinyxml2::XMLElement* rest_tag = note_tag->FirstChildElement("rest"); //Определение паузы
                            if (rest_tag != nullptr) {
                                rest = rest_tag->Name();
                                sout << "\t" << rest_tag->Name() << "\n";
                                v_notes.push_back('r');
                                v_octaves.push_back("n");
                                v_semitone.push_back(0);
                                v_league.push_back(0);

                                sout << "v_semitone " << v_semitone.size() - 1 << "\t";
                                sout << "v_notes " << v_notes.size() - 1 << "\t";

                            }
                            tinyxml2::XMLElement* duration_tag = note_tag->FirstChildElement("duration"); //Определение длительности
                            if (duration_tag != nullptr) {//длительность
                                duration = atoi(duration_tag->GetText());
                                sout << "\tduration: " << duration_tag->GetText() << "\n";
                                v_duration.push_back(duration);
                            }
                            tinyxml2::XMLElement* instrument_tag = note_tag->FirstChildElement("instrument");//для инструмента
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
                                        tinyxml2::XMLElement* tied_tag = note_tag->FirstChildElement("notations")->FirstChildElement("tied");
                                        //tied = tied_tag->Attribute("type");
                                        int count_tied = 0;
                                        while (tied_tag) {
                                            if (string(tied_tag->Attribute("type")) == "stop") count_tied++;
                                            //cout << "Attribute " << tied_tag->Attribute("type") << "\n";
                                            if (string(tied_tag->Attribute("type")) == "start") count_tied += 2;
                                            //cout << "\ttied: " << count_tied << "\n";

                                            tied_tag = tied_tag->NextSiblingElement("tied");//след элемент по tied

                                        }
                                        v_league[v_league.size() - 1] = count_tied;

                                        count_tied = 0;

                                    }


                                }
                            }
                            sout << "v_league " << v_league.size() - 1 << "\t";
                            sout << "\n";
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
    translate(instrument);
}

void TranslateMusic::setCurrentOctaveRange(QMap<Instrument, OctaveRange> newRange)
{
    m_currentOctaveRange = newRange;
}

void TranslateMusic::clearData()
{
    IdParts.clear();
    MusicalParts.clear();
    Voices.clear();
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
}

int TranslateMusic::translate(const int instrument)
{
    notes_f(v_notes, v_semitone, chromatic);//перевод из нот CDEFGAB в hex представление в survivalcraft
    octaves_f(converted_notes, v_octaves, instrument); //перевод октавы в формат survivalcraft, в том числе обрезка по октавам
    //calculation_duration(minimal_duration);//пока заморожено
    if (sign != "percussion") {//если не ударные
        convertToSequence(converted_notes, v_duration, v_league, NoteType::Pitch);// перевод в последовательность нот в зависимости от длительности
        convertToSequence(converted_octaves, v_duration, v_league, NoteType::Octave);//перевод в последовательность октав в зависимости от длительности
        //show_information_about_composition(v_duration, beats, beat_type, bpm);//рекомендуемая частота генератора, не работает правильно
        // show_notes(converted_notes_sequence); //вывод нот с делением на строки длиной 256 символов
        // show_octaves(converted_octaves_sequence);//вывод октав с делением на строки длиной 256 символов
        // show_instruments();
        //show_volumes(); //вывод громкости
    }
    else { //ударные
        convert_to_sequence_percussion(v_instruments, v_duration);
        // show_notes(converted_notes_sequence); //вывод нот с делением на строки длиной 256 символов
        // show_instruments();
        //show_volumes(); //вывод громкости
    }
    emit signalSendTranslation(converted_notes_sequence, converted_octaves_sequence);
    return 0;
}

void TranslateMusic::notes_f(QVector<char> &notes, QVector<int> &semitone, int chromatic)
{

    if (notes.size() != semitone.size()) {
        sout << "Error: vectors have different sizes" << endl;
    }
    sout << "Ноты в notes_f:" << endl;
    for (int i = 0; i < semitone.size(); i++) {
        semitone[i] = semitone[i] + chromatic;
        int hex_notes = switch_hex_notes(notes[i], semitone[i]);
        if (hex_notes < 0) { //??
            converted_notes.push_back(12 + hex_notes);
        }
        else {
            converted_notes.push_back(hex_notes);

        }
        sout << hex << uppercase << hex_notes;
    }
    sout << endl;
}

void TranslateMusic::octaves_f(QVector<int> &converted_notes, QVector<string> &octaves, int instrument)
{
    Instrument instr = static_cast<Instrument>(instrument);
    const int lowOctave = m_currentOctaveRange.value(instr).lowOctave;
    const int highOctave = m_currentOctaveRange.value(instr).highOctave;
    const int offset = m_currentOctaveRange.value(instr).offset;

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
                sout << ex.what() << endl;
            }
        }
    }
}

void TranslateMusic::convertToSequence(QVector<int>& converted, QVector<float>& duration, QVector<int>& league, NoteType type)
{
    if (converted.size() != duration.size()) {
        sout << "Error: vectors have different sizes" << endl;
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
        sout << ex.what() << endl;
    }
}

void TranslateMusic::convert_to_sequence_percussion(QVector<string> &, QVector<float> &)
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

int TranslateMusic::switch_hex_notes(char ch, int semitone)
{
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

void TranslateMusic::print_amount(int object, int duration, int league, QVector <int>& converted_notes_sequence)
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

ostream &TranslateMusic::defaultStream()
{
    static NullOStream nullStream;
    return nullStream;
}
