#include "gcodeget.h"
#include "QFileDialog"
#include "QString"
#include "QMessageBox"
#include "QGraphicsPixmapItem"
#include "QtMath"
#include "QTextStream"
#include "QXmlStreamReader"
#include "qmath.h"
#include "QTime"
#include <QThread>
#include "QDebug"
#include "mainwindow.h"

GCODEGET::GCODEGET(QObject *parent) : QObject(parent)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    //Скорости выставленные вручную
    settings.beginReadArray("sett/speed");
    for (int i = 0; i < 17;i++) {
        settings.setArrayIndex(i);
        this->speed[i]=settings.value(QString::number(i)).toInt();
    }
    settings.endArray();
    //this->zagWidth->setValue(settings.value("zagWidth").toInt());
    //this->zagHeight->setValue(settings.value("zagHeight").toString());
    this->density=(settings.value("sett/density").toDouble());
    this->k=(settings.value("sett/kSpeed").toDouble());

    this->blackSpeed=((this->speedType!=2)?settings.value("sett/blackSpeed").toInt():speed[0]);
    this->whiteSpeed=((this->speedType!=2)?settings.value("sett/whiteSpeed").toInt():speed[16]);
    this->speedType=(settings.value("sett/speedType").toInt());
    //Команды
    this->startLine=(settings.value("sett/startLine").toString());
    this->middleLine=(settings.value("sett/middleLine").toString());
    this->endLine=(settings.value("sett/endLine").toString());

    //Настройка осей
    this->firstAxe=(settings.value("sett/firstAxe").toString());
    this->invertFirst=(settings.value("sett/firstAxeInvert").toBool());
    this->secondAxe=(settings.value("sett/secondAxe").toString());
    this->invertSecond=(settings.value("sett/secondAxeInvert").toBool());
    this->slideType=(settings.value("sett/slideType").toInt());
    this->methodBurning=(settings.value("sett/methodBurning").toInt());
    this->parking=settings.value("sett/parking").toBool();

    //Протирка жала
    this->maxWipe=(settings.value("sett/maxWipe").toInt());
    this->wipeCount=(settings.value("sett/wipeCount").toInt());
    this->wipeCover=(settings.value("sett/wipeCover").toInt());
    this->wipeWidth=(settings.value("sett/wipeWidth").toInt());
    this->sting=(settings.value("sett/sting").toBool());
    this->wipeSpeed=(settings.value("sett/wipeSpeed").toInt());

    //GCODE синтаксис
    this->freeStep=(settings.value("sett/freeStep").toString());
    this->workStep=(settings.value("sett/workStep").toString());
    this->pauseCommand=(settings.value("sett/pauseCommand").toString());
    this->spindleOnCommand=(settings.value("sett/spindleOnCommand").toString());
    this->spindleOffCommand=(settings.value("sett/spindleOffCommand").toString());
    this->compressGCode=settings.value("sett/compressGCode").toBool();
    this->useShifts=settings.value("sett/useShifts").toBool();

}

//Сформировать строку gcode
QString GCODEGET::writeGCode(int modeNew,int x, int y,int speed,int wipeMode)
{
   QString line;
   QTextStream stream(&line);
   //Определение метода выжигания
   QString method;
   switch (this->methodBurning) {
       case 0: method="F"; break;
       case 1: method=this->pauseCommand;break;
       case 2: method=this->spindleOnCommand;break;
       case 3: break;
   }

   //Протирка по расстоянию
   if((oldRow!=y || oldCol!=x) && this->burnStyle>=2 && wipeMode!=1) {
       if(modeNew!=0){
           distance+=(pow(x-oldCol,2)+pow(y-oldRow,2))*density;
       }
       if(distance>this->maxWipe*10){
           distance=0;
           if(x>pyroImage.width()/2 && (this->wipeCover==2 || this->wipeCount==1)){
               stream<<this->wipe(-1,x,y);
           }
           else{
               stream<<this->wipe(1,x,y);
           }
       }
   }

   //Вывод режима
   if((modeNew!=mode)|| !this->compressGCode){
       if(modeNew){
           stream<<workStep;
       }
       else{
           stream<<freeStep;
       }
       mode=modeNew;
       if(this->useShifts){
           stream<<" ";
       }
   }

   if(x!=oldCol || !this->compressGCode){
       stream<<firstAxe<<(x*this->density);
       oldCol=x;
       if(this->useShifts){
           stream<<" ";
       }
   }

   if(y!=oldRow || !this->compressGCode){
       stream<<secondAxe<<(y*this->density);
       oldRow=y;
       if(this->useShifts){
           stream<<" ";
       }
   }

   if((oldSpeed!=this->getSpeed(speed) || !this->compressGCode) && mode){
       stream<<method<<this->getSpeed(speed);
       oldSpeed=this->getSpeed(speed);
   }
   stream<<"\r\n";


   return line;
}


//Протирка жала
QString GCODEGET::wipe(int direction, int col,int row)
{
    QString wipeC;
    QTextStream stream(&wipeC);

    if(this->sting)
    {
        int mode=(this->wipeSpeed==3)?0:1;
        int sp=0;
        switch (this->wipeSpeed) {
            case 0: sp=0; break;
            case 1: sp=127; break;
            case 2: sp=255; break;
        }

        if((this->wipeCover==0 || this->wipeCover==2) && direction==1){
            stream<<this->writeGCode(0,0,row,sp,1);
        }
        else if((this->wipeCover==1 || this->wipeCover==2) && direction==-1) {
           stream<<this->writeGCode(0,pyroImage.width(),row,sp,1);
        }

        for(int wipe=0;wipe<this->wipeCount;wipe++)
        {
            if((this->wipeCover==0 || this->wipeCover==2) && direction==1){
                stream<<this->writeGCode(mode,0,row,sp,1);
                stream<<this->writeGCode(mode,1.00*-this->wipeWidth/this->density,row,sp,1);
            }
            else if((this->wipeCover==1 || this->wipeCover==2) && direction==-1) {
                stream<<this->writeGCode(mode,pyroImage.width(),row,sp,1);
                stream<<this->writeGCode(mode,pyroImage.width()+(1.00*this->wipeWidth/this->density),row,sp,1);
            }
        }

        //Переделать под возврат каретки на нужную точку
        if((this->wipeCover==0 || this->wipeCover==2) && direction==1){
            stream<<this->writeGCode(0,col,row,sp,1);
        }
        else if((this->wipeCover==1 || this->wipeCover==2) && direction==-1) {
           stream<<this->writeGCode(0,col,row,sp,1);
        }
    }

    return wipeC;

}


//Вычислить направление движения для плоттера
bool GCODEGET::getDir(int x, int y){
    int currentPoint=255,max=-1;
    rowMin=0;
    colMin=0;
    for (int j=0;j<3;j++) {
        for (int i=0;i<3;i++){
            currentPoint=255;
            if((x+i-1<pyroImage.width() && x+i-1>=0) && (y+j-1<pyroImage.height() && y+j-1>=0)){
                 currentPoint=qGray(pyroImage.pixel(x+i-1,y+j-1));
            }
            if(currentPoint>max && (currentPoint<(256-(256/shades))))
            {
                max=currentPoint;
                rowMin=-1+i;
                colMin=-1+j;
            }
        }
    }
    if((rowMin==0 && colMin==0 ) || max==-1)
    {
        return false;
    }
    return true;
}

//вычисление скорости
int GCODEGET::getSpeed(int point){
    int speedN=0;
    float avg=0;
    switch(this->speedType){
        //Линейная скорость
        case 0:
            if(this->blackSpeed< this->whiteSpeed){
                avg=(this->whiteSpeed-this->blackSpeed)/255.0;
            }
            else {
                avg=-(this->blackSpeed-this->whiteSpeed)/255.0;
            }
            speedN=this->blackSpeed+trunc(avg*point);
        break;
        //Параболическая зависимость
        case 1:
            speedN=this->blackSpeed-(pow(this->blackSpeed-(this->blackSpeed/10),2)*point*this->k/((this->blackSpeed/2)*(point-300)));
        break;
        //Выставленное вручную
        default:
            int point1=(point==0)?0:point/16;
            speedN=speed[point1]+(abs((speed[point1+1]-speed[point1])/16.0)*(point-(point1*16)));
            break;
    }
    return speedN;
}

void GCODEGET::ger(){
    QFile f(str);
    f.open( QIODevice::WriteOnly);
    QTextStream stream( &f );

    stream << ";Black speed: " << blackSpeed<<"\r\n"
           << ";White speed: " << whiteSpeed<<"\r\n"
           << ";width zag: " << this->pyroImage.width()*density<<"\r\n"
           << ";height zag: " << this->pyroImage.height()*density<<"\r\n"
           << ";Thickness: " << density<<"\r\n"
           <<this->startLine<<"\r\n";
    int wid=this->pyroImage.width(), height=pyroImage.height(),whiteColorValue=255-trunc(256/(this->shades));

    //Режим плоттера
    if(this->burnStyle==3){
    int oldRow=0,oldCol=0;
        stream<<this->writeGCode(0,0,0,0,0);
        for (int j=0;j<pyroImage.height();j++) {
            for(int i=0;i<pyroImage.width();i++){
                int point=qGray(pyroImage.pixel(i,j));
                // Пролет над светлым оттенком
                if(point<=whiteColorValue){
                    int row=i, col=j;
                    //Подлет к точке рисования
                    if(!this->parking || abs(oldCol-col)<2){
                        stream<<this->writeGCode(0,i,j,0,0);
                    }
                    //Подлет к точке рисования через линейку
                    else{

                        if(this->wipeCover==2 && this->sting==1) {
                            if(i<((pyroImage.width()-1)/2)){
                                stream<<this->writeGCode(0,0,j,0,0);
                            }
                            else{
                                stream<<this->writeGCode(0,pyroImage.width()-1,j,0,0);
                            }
                            stream<<this->writeGCode(0,i,j,0,0);
                        }
                        else if(this->wipeCover==0 && this->sting==1){
                            stream<<this->writeGCode(0,0,j,0,0);
                            stream<<this->writeGCode(0,i,j,0,0);
                        }
                        else if(this->wipeCover==1 && this->sting==1){
                            stream<<this->writeGCode(0,pyroImage.width()-1,j,0,0);
                            stream<<this->writeGCode(0,i,j,0,0);
                        }
                        stream<<this->writeGCode(0,i,j,0,0);
                    }
                    stream<<this->writeGCode(1,i,j,255,0);
                    while(this->getDir(row,col)){
                        row+=rowMin;
                        col+=colMin;
                        oldCol=col;
                        oldRow=row;
                        int point2=qGray(pyroImage.pixel(row,col));
                        pyroImage.setPixel(row,col,qRgb(255,255,255));
                        stream<<this->writeGCode(1,row,col,point2,0);
                    }
                 }
            }
            emit addStep(j);
        }
    }
    // Режим Принтера
    else if(this->burnStyle==2){
        int i=0,j=0;
        while(i<wid && j<height){
            int point2,point,col=j;
            point2=point=qGray(pyroImage.pixel(i,j));
            while(point2==point){
                if(++i<wid && j<height){
                    point2=qGray(pyroImage.pixel(i,j));
                }else{
                    if(point<whiteColorValue){
                        stream<<this->writeGCode(1,i,j,point,0);
                        point2=256;
                    }
                    i=0;
                    if(++j>=height){
                        break;
                    }
                }
            }
            emit addStep(j);
            if(col!=j){
                stream<<this->writeGCode(0,i,j,0,0);
            }
            else{
                stream<<this->writeGCode(1,i,j,point,0);
            }
        }
    }
    //Обычный режим
    //Режим Змейки
    else{
        for(int col=0;col<this->pyroImage.height();col++)
        {
            if(col>0){
                stream<<this->writeGCode(0,0,col-1,0,0);
            }
            stream<<this->writeGCode(0,0,col,0,0);
            stream<<this->wipe(1,0,col)<<this->middleLine<<"\r\n";
            //Обычный
            for(int row=0;row<wid;row++){
                int point1=qGray(pyroImage.pixel(row,col));
                int point2=point1;
                while(qGray(point1)==qGray(point2) && (++row<wid))
                {
                    point2= qGray(pyroImage.pixel(row,col));
                }
                stream<<this->writeGCode(1,row,col,point1,0);
            }
            stream<<this->wipe(-1,pyroImage.width(),col);
            //Змейка
            if(this->burnStyle==1){
                if(++col<this->pyroImage.height()){
                    stream<<this->writeGCode(0,wid,col,0,0);
                    stream<<this->writeGCode(1,wid,col,0,0);
                    for(int row=wid-1;row>0;row--){
                        int point1= qGray(pyroImage.pixel(row,col));
                        int point2=point1;
                        while(qGray(point1)==qGray(point2) && (--row>0))
                        {
                            point2= qGray(pyroImage.pixel(row,col));
                        }
                        stream<<this->writeGCode(1,row,col,point1,0);
                    }
                }
            }
            emit addStep(col);
        }
    }
    stream<<"\r\n"<<this->endLine;
    f.close();
    emit complete();
}
