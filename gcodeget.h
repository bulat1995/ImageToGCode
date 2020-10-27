#ifndef GCODEGET_H
#define GCODEGET_H

#include <QObject>
#include "mainwindow.h"

class GCODEGET : public QObject
{
    Q_OBJECT
public:
    explicit GCODEGET(QObject *parent = 0);

    int speed[17],shades,speedType=0,burnStyle=0,blackSpeed=0,
            whiteSpeed=0,methodBurning=0,maxWipe=0,wipeCount=0,wipeCover=0,wipeWidth=0,slideType=0,rowMin=0,colMin=0,
            oldCol=0,oldRow=0,oldSpeed=0,wipeSpeed=0, mode=-1;

    bool invertFirst=false,invertSecond=false,sting=false,useShifts=true,compressGCode=false,parking=false;

    QString startLine,middleLine,endLine,firstAxe,secondAxe,freeStep,workStep,pauseCommand,spindleOnCommand,spindleOffCommand;
    double density=0,coordTransl,k=0;
    QImage pyroImage;
    QString str;
    double distance=0;

signals:
        void addStep(int);
        void complete();
public slots:
        void ger();
        int getSpeed(int speed);
        QString wipe(int direction,int col,int row);
        bool getDir(int x, int y);
        QString writeGCode(int modeNew,int firstAxe, int secondAxe,int speed,int wipeMode);
};

#endif // GCODEGET_H
