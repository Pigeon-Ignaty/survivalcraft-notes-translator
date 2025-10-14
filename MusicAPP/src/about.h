#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QCloseEvent>

class About : public QDialog
{
    Q_OBJECT
public:
    explicit About(QWidget *parent = nullptr);
    ~About();
private:
    QString getHelpVersion();//Ф-я получения версии dll
};

#endif // ABOUT_H
