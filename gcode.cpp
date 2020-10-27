#include "gcode.h"
#include "QGraphicsPixmapItem"
#include "QtMath"
#include "QTextStream"

gcode::gcode()
{

}
/*
void gcode::get(QImage pyroImage,double coordTransl,int blackSpeed, int whiteSpeed,int freeSpeed){

    int wid=pyroImage.width();
    int height=pyroImage.height();
    pyroImage.save("pyro.jpg","JPG");


        MainWindow::settings.setValue("saveFile", QFileInfo(str).path());
        QFile f(str);
        QFile GCODE("GCODE");
        GCODE.open(QIODevice::WriteOnly);
        QTextStream gcode(&GCODE);
        f.open( QIODevice::WriteOnly );
        QTextStream stream( &f );


        settings.setValue("density",coordTransl);
        settings.setValue("blackSpeed", blackSpeed);
        settings.setValue("whiteSpeed",  whiteSpeed);
        settings.setValue("freeSpeed",freeSpeed);

        stream << ";Black speed: " << blackSpeed<<endl
               << ";White speed: " << whiteSpeed<<endl
               << ";Free speed: " << freeSpeed<<endl
               << ";width zag: " << wid*coordTransl<<endl
               << ";height zag: " << height*coordTransl<<endl
               << ";Thickness: " << coordTransl<<endl
               <<this->startLine<<endl;


        //Ленивое выжигание
        if(this->burnStyle==3){
            stream <<this->WorkStep<<endl;
            gcode<<this->WorkStep<<endl;
            int i=0,j=0,beforePoint=0;
            double speed;
            while(i<wid && j<height){
                int point2,point,col=j;
                point2=point=pyroImage.pixel(i,j);

                while(point2==point){
                    if(++i<wid && j<height){
                        point2=pyroImage.pixel(i,j);
                    }else{
                        if(qRed(point)<this->whiteColorValue){
                            stream << this->firstAxe <<(i*coordTransl)<<this->secondAxe<<(j*coordTransl)<<" F"<<this->getSpeed(qRed(point))<<endl;
                            gcode<<this->firstAxe <<(i*coordTransl)<<this->secondAxe<<(j*coordTransl)<<" F"<<this->getSpeed(qRed(point))<<endl;
                            timeWork+=(i-beforePoint)*coordTransl/this->getSpeed(qRed(point));
                            point2=256;
                        }
                        beforePoint=i=0;
                        if(++j>=height){
                            break;
                        }
                    }
                }
                speed=(col==j)?this->getSpeed(qRed(point)):freeSpeed;
                stream << this->firstAxe <<(i*coordTransl)<<this->secondAxe<<(j*coordTransl)<<" F"<<speed<<endl;
                gcode<< this->firstAxe <<(i*coordTransl)<<this->secondAxe<<(j*coordTransl)<<" F"<<speed<<endl;
                beforePoint=i;
            }
            stream<<this->endLine<<endl;
        }else{
            for(int col=0;col<height;){

                //Пропуск светлых полосок
                if(this->burnStyle==2){
                    int topClear=0;
                    for(int point, row=0;row<wid;row++){
                        point=pyroImage.pixel(row,col);
                        if(qRed(point)<this->whiteColorValue){
                            topClear=0;
                            break;
                        }
                        else{
                            topClear++;
                        }

                    }
                    if(topClear>0){
                        col++; continue;
                    }
                }


                for(int row=0;row<wid;row++){
                    int  point2,point1 = pyroImage.pixel(row,col);
                    int lng=row;

                    for(; lng<wid;lng++){
                        point2= pyroImage.pixel(lng,col);
                        if(qRed(point1)!=qRed(point2)){
                            break;
                        }
                    }
                    row=--lng;
                    stream << this->firstAxe << (row*coordTransl)<<" F"<<this->getSpeed(qRed(point1))<<endl;
                    gcode<<this->firstAxe << (row*coordTransl)<<" F"<<this->getSpeed(qRed(point1))<<endl;
                }

                //Выжигание змейкой
                if(this->burnStyle==1 && ((++col)<height)){
                    stream<< this->secondAxe << (col*coordTransl)<<endl;
                    gcode<< this->secondAxe << (col*coordTransl)<<endl;
                    for(int row=wid-1;row>=0;row--){
                        int  point2,point1 = pyroImage.pixel(row,col);
                        int lng=row;
                        for(; lng>=0;lng--){
                            point2= pyroImage.pixel(lng,col);
                            if(qRed(point1)!=qRed(point2)){
                                break;
                            }
                        }
                        row=++lng;
                        stream << this->firstAxe << (row*coordTransl)<<" F"<<this->getSpeed(qRed(point1))<<endl;
                        gcode<<this->firstAxe << (row*coordTransl)<<" F"<<this->getSpeed(qRed(point1))<<endl;

                    }
                }
                col++;
                stream<<this->FreeStep<<" "<<this->firstAxe<<0<<endl<<this->middleLine<<endl<< this->secondAxe << (col*coordTransl)<<endl<<this->WorkStep<<endl;
                gcode<<this->FreeStep<<" "<<this->firstAxe<<0<<endl<<this->middleLine<<endl<< this->secondAxe << (col*coordTransl)<<endl<<this->WorkStep<<endl;

            }
            stream<<this->endLine<<endl;
            gcode<<this->endLine<<endl;
        }

        f.close(); GCODE.close();
    }
}*/
