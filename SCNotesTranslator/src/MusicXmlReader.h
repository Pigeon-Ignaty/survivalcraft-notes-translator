#ifndef MUSICXMLREADER_H
#define MUSICXMLREADER_H
#include <QObject>
#include <QFile>
#include "miniz.h"
#include "tinyxml2.h"
#include "QTextEditStream.h"

enum class Error {
    NoError,
    FileNotFound,
    EmptyFile,
    OpenFailed,
    ZipFileOpenError,
    ErrorFileInZip,
    ErrorExtractFile,
    XmlNotFoundInZip,
    TinyXmlFileError
};

struct MusicalPart{
    QString id;
    QString name;
    QVector <QString> voices;
};

class MusicXmlReader :public QObject
{
    Q_OBJECT
public:
    MusicXmlReader(QObject *parent, std::ostream& out = defaultStream());
    bool load(const QString &pathToFile); //Общая ф-я загрузки и открытия файла
    QString getErrorString() const {return m_errorString;} //Ф-я получения ошибки
    Error error() const{ return m_error; }
    bool parseParties(); // Ф-я поиска партий в файле
    bool parseVoices(const QString &partId); // Ф-я поиска голосов в партии
    QVector <MusicalPart> getMusicalParts() const {return m_musicalParts; }//Получение списка партий
    QVector <QString> getVoicesFromPart(const QString &partId) const;//Получение списка голосов
    tinyxml2::XMLDocument *getDoc(){ return &doc;}
private:
    // Вспомогательные функции открытия файла
    bool loadZipFile(const QString &pathToFile);
    bool loadFile(const QString &pathToFile);
    bool loadXML(const QString &pathToFile);
    QString extractFileNameFromContainer(QByteArray data); //Вспомогательная функция чтения container.xml

    QString m_errorString;
    Error m_error;
    QByteArray m_fileData;
    tinyxml2::XMLDocument doc;
    QVector <MusicalPart> m_musicalParts;

    std::ostream& sout;
    static std::ostream& defaultStream();

    friend class Test_SCNotesTranslator;
};

#endif // MUSICXMLREADER_H
