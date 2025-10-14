#ifndef HELP_H
#define HELP_H

#include "help_global.h"
#include <QWidget>
#include <QCloseEvent>
#include <QDialog>
#include <QWebEngineView>
#include <QHBoxLayout>

class HELPSHARED_EXPORT Help : public QWidget
{
    Q_OBJECT
public:
    Help(QWidget *parent = nullptr);
    ~Help();
};


#endif // HELP_H
