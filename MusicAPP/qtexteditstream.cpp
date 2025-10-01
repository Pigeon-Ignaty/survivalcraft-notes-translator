#include "QTextEditStream.h"

QTextEditStream::QTextEditStream(QTextEdit *textEdit) : textEdit(textEdit) {}

QTextEditStream::~QTextEditStream() {
    sync();
}

std::streambuf::int_type QTextEditStream::overflow(std::streambuf::int_type v) {
    if (v == '\n') {
        textEdit->append(QString::fromUtf8(buffer));  // Преобразуем в QString
        buffer.clear();
    } else {
        buffer.append(static_cast<char>(v));  // Заполняем UTF-8 строку
    }
    return v;
}

int QTextEditStream::sync() {
    if (!buffer.isEmpty()) {
        textEdit->append(QString::fromUtf8(buffer));  // Преобразуем перед выводом
        buffer.clear();
    }
    return 0;
}

QTextEditStreamOutput::QTextEditStreamOutput(QTextEdit *textEdit)
    : std::ostream(nullptr), textEditStream(textEdit) {
    rdbuf(&textEditStream);
}

QTextEditStreamOutput::~QTextEditStreamOutput() {}
