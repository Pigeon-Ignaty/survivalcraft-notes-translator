#include "MusicXmlReader.h"
#include <QFileInfo>
#include <QDebug>

MusicXmlReader::MusicXmlReader(QObject *parent, std::ostream& out) : QObject(parent),
    m_error(Error::NoError), sout(out) {}

bool MusicXmlReader::load(const QString &pathToFile)
{
    //Чистим данные
    m_errorString.clear();
    m_musicalParts.clear();
    m_fileData.clear();
    doc.Clear();
    doc.ClearError();
    m_error = Error::NoError;

    if(!loadFile(pathToFile)) //Загружаем файл
        return false;

    if(!loadXML(pathToFile)) //Читаем xml
        return false;

    return true;
}

bool MusicXmlReader::parseParties()
{
    tinyxml2::XMLElement* pRootElement = doc.RootElement();
    if(!pRootElement)
        return false;

    tinyxml2::XMLElement* creditElement = pRootElement->FirstChildElement("credit");
    while (creditElement) {
        //Вроде как название и автор
        tinyxml2::XMLElement* creditTypeElement = creditElement->FirstChildElement("credit-type");
        tinyxml2::XMLElement* creditWordsElement = creditElement->FirstChildElement("credit-words");

        QString nameAndAuthor =
            QString(creditTypeElement && creditTypeElement->GetText()
                        ? creditTypeElement->GetText() : "") + " " +
            QString(creditWordsElement && creditWordsElement->GetText()
                        ? creditWordsElement->GetText() : "");
        sout << nameAndAuthor.toStdString() << "\t";

        creditElement = creditElement->NextSiblingElement("credit");
    }

    tinyxml2::XMLElement* partListElement = pRootElement->FirstChildElement("part-list");
    if(!partListElement)
        return false;
    sout << "Вывод следующих партий в музыкальной композиции: " << "\n";
    int scorePartCount = 0;
    for (auto scorePartElement = partListElement->FirstChildElement("score-part"); scorePartElement;
         scorePartElement = scorePartElement->NextSiblingElement("score-part"))
        {
            tinyxml2::XMLElement* partNameElement = scorePartElement->FirstChildElement("part-name");
            if(!partNameElement)
                continue;

            QString partName = partNameElement->GetText() ? partNameElement->GetText(): "";
            const char* id = scorePartElement->Attribute("id");
            if(!id)
                continue;
            sout << "ID: " << scorePartElement->Attribute("id") << "\t";

            sout << "Название партии: " << partName.toStdString() << "\n";
            m_musicalParts.append(MusicalPart{id, partName});

            scorePartCount++;
        }
    sout << "scorePartCount: " << scorePartCount << "\n";

    if(scorePartCount == 0)
        return false;
    return true;
}

bool MusicXmlReader::parseVoices(const QString &partId)
{
    tinyxml2::XMLElement* pRootElement = doc.RootElement();
    tinyxml2::XMLElement* measure_tag = nullptr;
    auto currentPart = std::find_if(m_musicalParts.begin(), m_musicalParts.end(),[&partId](const MusicalPart &otherPart){
        return otherPart.id == partId;
    });
    if(currentPart == m_musicalParts.end())
        return false;
    currentPart->voices.clear();

    if (pRootElement != nullptr) { // проверка на пустоту
        tinyxml2::XMLElement* part_id_tag = pRootElement->FirstChildElement("part");
        while (part_id_tag != nullptr) {
            const char* xmlPartId = part_id_tag->Attribute("id");
            if (xmlPartId && partId == QString::fromUtf8(xmlPartId)) {
                measure_tag = part_id_tag->FirstChildElement("measure");
                while (measure_tag != nullptr) {
                    tinyxml2::XMLElement* note_tag = measure_tag->FirstChildElement("note");
                    while (note_tag != nullptr) {
                        tinyxml2::XMLElement* voice_tag = note_tag->FirstChildElement("voice");
                        if (voice_tag != nullptr) {
                            if(const char *voice = voice_tag->GetText()){
                                QString voiceStr = QString::fromUtf8(voice);
                                if(!currentPart->voices.contains(voiceStr))
                                    currentPart->voices.push_back(voiceStr);
                            }
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
    std::sort(currentPart->voices.begin(), currentPart->voices.end());

    sout << "Голоса, обнаруженные в партии" << "\n";
    for (auto &voice : currentPart->voices) {
        sout << voice.toStdString() << "\n";
    }

    return !currentPart->voices.isEmpty();
}

QVector<QString> MusicXmlReader::getVoicesFromPart(const QString &partId) const
{
    auto currentPart = std::find_if(m_musicalParts.begin(), m_musicalParts.end(),[&partId](const MusicalPart &otherPart){
        return otherPart.id == partId;
    });
    if(currentPart == m_musicalParts.end())
        return {};
    return currentPart->voices;
}

bool MusicXmlReader::loadZipFile(const QString &pathToFile)
{
    m_fileData.clear();

    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(zipArchive));

    QFileInfo fileInfo(pathToFile);
    QString archiveName = fileInfo.fileName();

    //Ошибка открытия zip архива
    if (!mz_zip_reader_init_file(&zipArchive, pathToFile.toLocal8Bit().constData(), 0)) {
        m_error = Error::ZipFileOpenError;
        m_errorString = tr("Couldn't open mxl file: ") + archiveName;
        return false;
    }
    //Кол-во файлов в архиве
    int numFiles = mz_zip_reader_get_num_files(&zipArchive);

    //Путь к главному xml из container.xml
    QString rootXmlPath;
    //Ищем и читаем META-INF/container.xml
    for (int i = 0; i < numFiles; ++i) {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat))
            continue;

        QString fileName = QString::fromUtf8(fileStat.m_filename);

        if (fileName == "META-INF/container.xml") {
            size_t uncompressedSize = fileStat.m_uncomp_size;

            void* data = mz_zip_reader_extract_file_to_heap(
                &zipArchive, fileStat.m_filename, &uncompressedSize,0);

            if (!data) {
                mz_zip_reader_end(&zipArchive);
                m_error = Error::ErrorExtractFile;
                m_errorString = tr("Failed to extract container.xml");
                return false;
            }

            QByteArray containerData(static_cast<const char*>(data), static_cast<int>(uncompressedSize));

            mz_free(data);

            rootXmlPath = extractFileNameFromContainer(containerData);
            break;
        }
    }

    if (rootXmlPath.isEmpty()) {
        mz_zip_reader_end(&zipArchive);
        m_error = Error::ErrorExtractFile;
        m_errorString = tr("container.xml not found or invalid in mxl archive");
        return false;
    }

    //Извлекаем главный XML
    size_t xmlSize = 0;
    void* xmlData = mz_zip_reader_extract_file_to_heap(
        &zipArchive, rootXmlPath.toUtf8().constData(), &xmlSize, 0);

    if (!xmlData) {
        mz_zip_reader_end(&zipArchive);
        m_error = Error::ErrorExtractFile;
        m_errorString = tr("Failed to extract main MusicXML file: ") + rootXmlPath;
        return false;
    }

    m_fileData = QByteArray(
        static_cast<const char*>(xmlData), static_cast<int>(xmlSize) );

    mz_free(xmlData);
    mz_zip_reader_end(&zipArchive);

    return true;
}


bool MusicXmlReader::loadFile(const QString &pathToFile)
{
    QFile file(pathToFile);
    QFileInfo fileInfo(file);
    QString fileName = fileInfo.fileName();

    if(!file.exists()){
        m_error = Error::FileNotFound;
        m_errorString = tr("File not found: ") + fileName;
        return false;
    }

    if(fileInfo.size() == 0){
        m_error = Error::EmptyFile;
        m_errorString = tr("File is empty: ") + fileName;
        return false;
    }
    if(!file.open(QIODevice::ReadOnly)){
        m_error = Error::OpenFailed;
        m_errorString = tr("Couldn't open the file: ") + fileName;
        return false;
    }

    if(fileInfo.suffix().toLower() == "mxl"){//Если сжатый xml, то передаём в miniz
        return loadZipFile(pathToFile);
    }
    else{ //Если обычный musicxml
        m_fileData = file.readAll();
        file.close();
        return true;
    }
}

bool MusicXmlReader::loadXML(const QString &pathToFile)
{
    QFile file(pathToFile);
    QFileInfo fileInfo(file);
    QString fileName = fileInfo.fileName();

    tinyxml2::XMLError error = doc.Parse(m_fileData.constData(), m_fileData.size());
    if (error != tinyxml2::XML_SUCCESS) {//ошибка открытия
        m_error = Error::TinyXmlFileError;
        m_errorString = QString::fromUtf8(doc.ErrorStr()) + tr(" Something is wrong with the contents of the file: ") + fileName;
        return false;
    }
    return true;
}

QString MusicXmlReader::extractFileNameFromContainer(QByteArray data)
{
    tinyxml2::XMLDocument doc;
    if (doc.Parse(data.constData(), data.size()) != tinyxml2::XML_SUCCESS)
        return {};

    auto container = doc.FirstChildElement("container");
    if (!container)
        return {};

    auto rootfiles = container->FirstChildElement("rootfiles");
    if (!rootfiles)
        return {};

    auto rootfile = rootfiles->FirstChildElement("rootfile");
    if (!rootfile)
        return {};

    const char* fullPath = rootfile->Attribute("full-path");
    if (!fullPath)
        return {};

    return QString::fromUtf8(fullPath);
}

std::ostream &MusicXmlReader::defaultStream()
{
    static NullOStream nullStream;
    return nullStream;
}
