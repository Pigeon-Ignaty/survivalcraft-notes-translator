#ifndef ABOUT_H
#define ABOUT_H

#include <QWidget>
#include <QCloseEvent>

namespace Ui {
class About;
}

class About : public QWidget
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();

private:
    Ui::About *ui;
signals:
protected:
private slots:

};

#endif // ABOUT_H
