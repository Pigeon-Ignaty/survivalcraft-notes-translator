#ifndef QTEXTEDITSTREAM_H
#define QTEXTEDITSTREAM_H

#include <streambuf>
#include <QTextEdit>
#include <ostream>
#include <QString>

class QTextEditStream : public std::basic_streambuf<char> {
public:
    QTextEditStream(QTextEdit *textEdit);
    ~QTextEditStream();

protected:
    virtual int_type overflow(int_type v) override;
    virtual int sync() override;

private:
    QByteArray buffer;  // Используем QByteArray для хранения UTF-8
    QTextEdit *textEdit;
};

class QTextEditStreamOutput : public std::ostream {
public:
    QTextEditStreamOutput(QTextEdit *textEdit);
    ~QTextEditStreamOutput();

private:
    QTextEditStream textEditStream;
};

#endif // QTEXTEDITSTREAM_H
