#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "QGraphicsScene"
#include <QKeyEvent>
#include "QSettings"
#include "QCoreApplication"
#include "gcodeget.h"


namespace Ui {
class MainWindow;
class GCodeGet;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    int burnStyle=0;

    //Изображение оригинал
    QImage OrigImage;

    QColor backgroundImage;

    //Изображение после постеризации
    QImage postImage;

    QGraphicsScene *scene = new QGraphicsScene;

    int speed[17], whiteColorValue, rotate=0;
    double startK, stepK;

    QSettings settings(const QString,const QString);

    QString fileName,filePath;

    //размер изображения на экране
    double screenWidth,screenHeight;

    int cutPositionX=0,cutPositionY=0,cutPositionWidth=0,cutPositionHeight=0;

int getSpeed(int point);

   MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:


    void on_action_triggered();

    void on_action_6_triggered();

    void on_zoom_valueChanged(int value);

    void on_zagWidth_valueChanged(int arg1);

    void on_saveGcode_clicked();

    void shades_activated();


    void on_saveSettings_clicked();

    void on_pushButton_clicked();

    void showSpeed();

    void on_k_valueChanged(double arg1);

    void on_whiteSpeed_valueChanged(int index);

    void defaultSetting(bool def);

    void setStep(int step);

    void gcodeComplete();

    void on_m0_valueChanged(int value);

    void on_burnStyle_currentIndexChanged(int index);

    void on_speedType_currentIndexChanged(int index);

    void on_slideType_currentIndexChanged(int index);

    void checkWipper();


    void on_blackSpeed_valueChanged(int value);


    void on_turnToRight_clicked();

    void on_turnToLeft_clicked();

    void on_CuttImage_clicked();

    void on_colorReset_clicked();

    void on_cutBox_toggled(bool arg1);

    void on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint);


    void on_toOriginal_clicked();

    void setPal(bool arg);

    void on_pencilEffect_clicked(bool arg1);

    void showScene(QImage img,int zoomValue);

    void on_methodBurning_currentIndexChanged(int index);

    void on_saveImage_triggered();
    void on_shades_activated(int index);

    void on_action_5_triggered();

    void on_gainEffect_clicked();

    void on_pencilEffect_toggled(bool arg1);

private:
    Ui::MainWindow *ui;
protected:
     virtual void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H




