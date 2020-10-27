#ifndef SETTING_H
#define SETTING_H

#include <QMainWindow>

namespace Ui {
class setting;
}

class setting : public QMainWindow
{
    Q_OBJECT

public:
    explicit setting(QWidget *parent = 0);
    ~setting();

private:
    Ui::setting *ui;
};

#endif // SETTING_H
