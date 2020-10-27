#include "mainwindow.h"
#include "ui_mainwindow.h"
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
#include <gcodeget.h>

#include <QColorDialog>
#include "QGraphicsBlurEffect"

#include "QDebug"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
        QCoreApplication::setOrganizationName("PyroConverter");
        QCoreApplication::setOrganizationDomain("PyroConverter");
        QCoreApplication::setApplicationName("PyroConverter");
        ui->setupUi(this);
        this->defaultSetting(false);
        QObject::connect(ui->brightCut, SIGNAL(valueChanged(int)),this, SLOT(shades_activated()));
        QObject::connect(ui->contrast, SIGNAL(valueChanged(int)),this, SLOT(shades_activated()));
        QObject::connect(ui->darkCut, SIGNAL(valueChanged(int)),this, SLOT(shades_activated()));
        QObject::connect(ui->shades, SIGNAL(currentIndexChanged(int)),this, SLOT(shades_activated()));
        QObject::connect(ui->gain, SIGNAL(valueChanged(int)),this, SLOT(shades_activated()));
        QObject::connect(ui->pencil, SIGNAL(valueChanged(int)),this, SLOT(shades_activated()));
        QObject::connect(ui->inverse, SIGNAL(stateChanged(int)),this, SLOT(shades_activated()));
        QObject::connect(ui->mirrorHor, SIGNAL(stateChanged(int)),this, SLOT(shades_activated()));
        QObject::connect(ui->mirrorVer, SIGNAL(stateChanged(int)),this, SLOT(shades_activated()));
        this->backgroundImage.setRgb(255,255,255,255);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//Горячие клавиши
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if ((event->modifiers() & Qt::ControlModifier)) {
        switch (event->key()) {
        case Qt::Key_O:this->on_action_triggered(); break; //Открыть файл
        case Qt::Key_S:this->on_saveImage_triggered(); break; //
        case Qt::Key_A:this->on_action_6_triggered(); break; //
        case Qt::Key_X: this->on_action_5_triggered();break;//Закрыть программу
        }
    }
}


/*==========================================================================================
 *  Конвертер
 *
 *
 *
 *
 *
 *==========================================================================================
*/



void MainWindow::showScene(QImage img,int zoomValue)
{
    this->screenWidth=zoomValue;

    QPixmap pixmap = pixmap.fromImage(img.scaledToWidth(this->screenWidth,Qt::SmoothTransformation));
    this->screenHeight=pixmap.height();

    QPainter qPainter(&pixmap);
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    pen.setBrush(Qt::black);
    qPainter.setPen(pen);

    qPainter.drawRect(0,0,this->screenWidth-1,this->screenHeight-1);

    this->scene->clear();

        this->scene->setSceneRect(0,0,this->screenWidth,this->screenHeight);


    this->scene->addPixmap(pixmap);
    ui->graphicsView->setScene(this->scene);
    ui->graphicsView->show();
    ui->zoom->setValue(zoomValue);
}

//Открыть изображение
void MainWindow::on_action_triggered()
{
   QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
   QString str = QFileDialog::getOpenFileName(0, "Выберите изображение для преобразования в gcode", settings.value("openFile").toString(), "*.jpg *.jpeg *.png *.bmp *.gif");
    if(!str.isNull()){
        ui->saveImage->setEnabled(true);
        this->filePath=str;
        this->fileName=QFileInfo(this->filePath).baseName();
        settings.setValue("openFile", QDir(this->filePath).absolutePath());
        this->postImage=this->OrigImage=QImage(filePath);
        ui->zoom->setMaximum(this->postImage.width()*4);
        ui->zoom->setValue(this->postImage.width());
        setPal(true);
        ui->saveGcode->setEnabled(false);
    }
}

void MainWindow::setPal(bool arg){
    QWidget *obj[23]={
        ui->toOriginal,ui->contrast,ui->contrastLabel,ui->shadesLabel,ui->brightCut,ui->shades,ui->zagWidth,ui->zagHeight,ui->zoom,
        ui->pencilEffect,ui->pencil,ui->gain,ui->cutBox,ui->inverse,ui->darkCut,
        ui->colorReset,ui->mirrorHor,ui->mirrorVer,ui->turnToLeft,ui->turnToRight,
        ui->brightCutLabel, ui->darkCutLabel,ui->burnStyle
    };
    for(int i=0;i<23;i++){
        obj[i]->setEnabled(arg);
    }
    ui->pencilEffect->setChecked(false);
}

//Увеличение картинки
void MainWindow::on_zoom_valueChanged(int value)
{
    this->showScene(this->postImage,value);
}

//Сохранить изображение
void MainWindow::on_saveImage_triggered(){

    if(fileName.isNull()){
        return;
    }

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    QString str = QFileDialog::getSaveFileName(0, "Выберите место для сохранения изображения", settings.value("saveFile").toString()+"/"+this->fileName+"(edited).jpg", "*.jpg");
    if( !str.isNull() )
    {
        this->postImage.save(str,"JPG");
    }
}

//Сохранение  gкода
void MainWindow::on_saveGcode_clicked()
{
     QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

     double coordTransl=settings.value("sett/density").toDouble();


     int wid=round(ui->zagWidth->value()*(1.0/coordTransl)),
             height=wid*((double)OrigImage.height()/(double)this->OrigImage.width());

      QString pthAdd=" ["+QString::number(trunc(wid*settings.value("sett/density").toDouble()))+"x"+QString::number(trunc(height*settings.value("sett/density").toDouble()))+"]";
      QString str = QFileDialog::getSaveFileName(0, "Выберите место для сохранения gcode", settings.value("saveFile").toString()+"/"+this->fileName+pthAdd+".cnc", "*.cnc");
      if( !str.isNull() )
      {
          settings.setValue("sett/zagWidth",ui->zagWidth->value());

          settings.setValue("saveFile", QFileInfo(str).path());
          ui->saveGcode->setEnabled(false);
          setPal(false);
          QThread *thread=new QThread();
          GCODEGET *gcode=new GCODEGET();
          gcode->pyroImage=this->postImage.scaled(wid,height,Qt::KeepAspectRatio,Qt::SmoothTransformation);
          gcode->str=str;
          gcode->burnStyle=ui->burnStyle->currentIndex();
          gcode->shades=ui->shades->currentText().toDouble();
          ui->gCodeStatus->setMaximum(height);
          gcode->moveToThread(thread);
          connect(thread,SIGNAL(started()),gcode,SLOT(ger()));
          connect(gcode,SIGNAL(addStep(int)),this,SLOT(setStep(int)));
          connect(gcode,SIGNAL(complete()),this,SLOT(gcodeComplete()));
          thread->start(QThread::TimeCriticalPriority);

      }
}

//Расчет размера заготовки по ширине
void MainWindow::on_zagWidth_valueChanged(int arg1)
{
    double relay = (this->postImage.height()*1.00)/(this->postImage.width()*1.00);
    if(this->postImage.height()>0){
        qDebug()<<arg1<<" "<<relay;
        ui->zagHeight->setText(QString::number(trunc(arg1*relay)));
    }

}


//Размытие изображения
QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0)
{
    if(src.isNull()) return QImage();   //No need to do anything else!
    if(!effect) return src;             //No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF( -extent, -extent, src.width()+extent*2, src.height()+extent*2 ) );
    return res;
}


void MainWindow::on_shades_activated(int index)
{
    this->shades_activated();
}


//Изменить набор оттенков картинки
void MainWindow::shades_activated()
{
    this->scene->clear();
    QSize *sizera=new QSize();
    if(this->rotate==0 || this->rotate==2){
        sizera->setWidth(this->OrigImage.width());
        sizera->setHeight(this->OrigImage.height());
    }
    else{
        sizera->setWidth(this->OrigImage.height());
        sizera->setHeight(this->OrigImage.width());
    }

    QImage imageGause(*sizera,QImage::Format_ARGB32_Premultiplied);
    QImage image(*sizera,QImage::Format_ARGB32_Premultiplied);

    QSize sizeImage = image.size();
    int value=trunc(256/(ui->shades->currentText().toInt()));
    //порог с которого начинается белый цвет
    this->whiteColorValue=256-value;
    bool mirrorHor=ui->mirrorHor->isChecked(),
         mirrorVer=ui->mirrorVer->isChecked();

    imageGause=this->OrigImage;

    for (int f1= 0; f1<imageGause.width(); f1++) {
        for (int f2= 0; f2<imageGause.height(); f2++) {
             QRgb color = imageGause.pixel(f1, f2);
             int gray =qGray(color);
             gray =(gray<this->whiteColorValue)? (round(gray/value)*value):255;
             imageGause.setPixel(f1, f2, qRgb(gray,gray,gray));
        }
     }
    //Размытие картинки
    if(ui->pencilEffect->isChecked()){
        QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(ui->pencil->value());
        imageGause = applyEffectToImage(imageGause, blur);
    }

    for (int f1= 0; f1<sizeImage.width(); f1++) {
        for (int f2= 0; f2<sizeImage.height(); f2++) {
            int pos1=0, pos2=0;
            switch (this->rotate) {
                case 0: pos1=(mirrorHor) ? sizeImage.width()-f1-1 : f1 ; pos2=(mirrorVer)? sizeImage.height()-f2-1 :f2; break;
                case 1: pos1=(mirrorVer)?sizeImage.height()-f2-1:f2; pos2=(mirrorHor)? f1 :sizeImage.width()-1-f1; break;
                case 2: pos1=(!mirrorHor) ? sizeImage.width()-f1-1 : f1 ; pos2=(!mirrorVer)? sizeImage.height()-f2-1 :f2; break;
                case 3: pos1=(!mirrorVer)?sizeImage.height()-f2-1:f2; pos2=(!mirrorHor)? f1 :sizeImage.width()-1-f1; break;
            }

            QRgb color = this->OrigImage.pixel(pos1, pos2);
            int gray =qGray(color);
            //Замена прозрачного на белый
            if(qAlpha(color)<=32) gray=255;
            //Контрастность
            gray+=ui->contrast->value();
            if(gray<0)gray=0;
            if(gray>255){gray=255;}

            //Алгоритм работы карандаша
            if(ui->pencilEffect->isChecked()){
                color = imageGause.pixel(pos1, pos2);
                gray+=abs(qGray(color)-255);
                if(gray>255) gray=255;

                if(ui->gainEffect->isChecked()){
                    color = this->OrigImage.pixel(pos1, pos2);
                    gray=qGray(color)-trunc(abs(255-gray)*(ui->gain->value()*0.1));
                }
            }

            if(gray<0)gray=0;
            if(gray>255){gray=255;}

            if(gray>255-ui->brightCut->value() || gray<ui->darkCut->value()){
                gray=255;
            }

            //Инверсия изображения
            if(ui->inverse->isChecked()){
                gray=abs(gray-255);
            }
            gray =(gray<this->whiteColorValue)? (round(gray/value)*value):255;

            image.setPixel(f1, f2, qRgb(gray,gray,gray));
        }
    }
    this->postImage=image;

    this->showScene(this->postImage,ui->zoom->value());
    ui->zagWidth->setEnabled(true);
    ui->saveGcode->setEnabled(true);
}

//установка процента конвертирования в гкод
 void MainWindow::setStep(int step){
     ui->gCodeStatus->setValue(step);
 }

 //Уведомление о сохранении гкода
 void MainWindow::gcodeComplete(){
     QMessageBox::information(this,"Файл сохранен","Файл успешно сохранен");
     ui->shades->setEnabled(true);
     ui->saveGcode->setEnabled(true);
     setPal(true);
     ui->gCodeStatus->setValue(0);
 }

 //Поворот изображения
 void MainWindow::on_turnToRight_clicked()
 {
     if(++rotate>3) this->rotate=0;
     shades_activated();
 }

 //Поворот изображения
 void MainWindow::on_turnToLeft_clicked()
 {
     if(--rotate<0) this->rotate=3;
     shades_activated();
 }

 //Обрезать изображение
 void MainWindow::on_CuttImage_clicked()
 {
     this->OrigImage=this->OrigImage.copy((this->OrigImage.width()/this->screenWidth)*this->cutPositionX,(this->OrigImage.height()/this->screenHeight)*this->cutPositionY, (this->OrigImage.width()/this->screenWidth)*this->cutPositionWidth,((this->OrigImage.height()/this->screenHeight)*cutPositionHeight));
     shades_activated();
     ui->cutBox->setChecked(false);
 }

 //Сброс значений эффекта
 void MainWindow::on_colorReset_clicked()
 {
     this->rotate=0;
     ui->contrast->setValue(0);
     ui->contrast->setEnabled(true);
     ui->contrastLabel->setEnabled(true);
     ui->pencil->setEnabled(false);
     ui->pencilEffect->setChecked(false);
     ui->darkCut->setValue(0);
     ui->brightCut->setValue(0);
     ui->inverse->setChecked(false);
     ui->gain->setValue(0);
     shades_activated();
 }

 //Обрезка изображения
 void MainWindow::on_cutBox_toggled(bool arg1)
 {
     if(arg1){
         ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
     }
     else{
         ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
     }
 }

 //ВЫделение участка для обрезки
 void MainWindow::on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint)
 {
     if(viewportRect.width()*viewportRect.height()!=0){
         this->cutPositionX=(fromScenePoint.x()>toScenePoint.x()) ? toScenePoint.x() : fromScenePoint.x();
         this->cutPositionY=(fromScenePoint.y()>toScenePoint.y()) ? toScenePoint.y() : fromScenePoint.y();
         this->cutPositionWidth=viewportRect.width();
         this->cutPositionHeight=viewportRect.height();
     }

     QImage img = this->postImage;
     QPixmap pixmap = pixmap.fromImage(img.scaled(this->screenWidth,this->screenHeight,Qt::KeepAspectRatio,Qt::SmoothTransformation));
     QPainter qPainter(&pixmap);
     QPen pen;
     pen.setStyle(Qt::DashDotLine);
     pen.setWidth(2);
     pen.setBrush(Qt::red);
     pen.setCapStyle(Qt::RoundCap);
     pen.setJoinStyle(Qt::BevelJoin);
     qPainter.setPen(pen);

    qPainter.drawRect(this->cutPositionX,this->cutPositionY,this->cutPositionWidth, this->cutPositionHeight);
    this->scene->clear();
    this->scene->setSceneRect(0,0,this->screenWidth,this->screenHeight);
    this->scene->addPixmap(pixmap);
 }

 //Вернуться к оригиналу
 void MainWindow::on_toOriginal_clicked()
 {
     QImage img=this->postImage=this->OrigImage=QImage(filePath);
     this->showScene(this->postImage,ui->zoom->value());
     on_zagWidth_valueChanged(ui->zagWidth->value()+1);
     on_zagWidth_valueChanged(ui->zagWidth->value()-1);
 }

 void MainWindow::on_pencilEffect_clicked(bool arg1)
 {
     ui->contrast->setEnabled(!arg1);
     ui->pencil->setEnabled(arg1);
     ui->contrastLabel->setEnabled(!arg1);
     if(arg1)ui->contrast->setValue(0);
     shades_activated();
 }



/*==========================================================================================
 *  Настройки
 *
 *
 *
 *
 *
 *==========================================================================================
*/


//Сброс настроек по умолчанию
void MainWindow::on_pushButton_clicked()
{
    QMessageBox::StandardButton answer= QMessageBox::question(this,"Действие","Вы действительно хотите сбросить настройки по умолчанию",QMessageBox::No|QMessageBox::Yes);
    if(answer==QMessageBox::Yes){
        this->defaultSetting(true);
    }
}

//Окно о программе
void MainWindow::on_action_6_triggered()
{
    QMessageBox::about( 0 , tr("О программе..."), tr("<b>Конвертер</b> v1.00<sup>beta</sup><br><p>Программа предназначена для конвертирования изображения в gcode.</p>"));
}

//вычисление скорости
int MainWindow::getSpeed(int point){
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    int speedN=0;
    float avg=0;
    switch(ui->speedType->currentIndex()){
        //Линейная скорость
        case 0:
            if(ui->blackSpeed->value() < ui->whiteSpeed->value()){
                avg=(ui->whiteSpeed->value()-ui->blackSpeed->value())/255.0;
            }
            else {
                avg=-(ui->blackSpeed->value()-ui->whiteSpeed->value())/255.0;
            }
            speedN=ui->blackSpeed->value()+trunc(avg*point);
        break;
        //Параболическая зависимость
        case 1:
            speedN=ui->blackSpeed->value()-(pow(ui->blackSpeed->value()-(ui->blackSpeed->value()/10),2)*point*ui->k->value()/((ui->blackSpeed->value()/2)*(point-300)));
        break;
        //Выставленное вручную
        default:
            int point1=(point==0)?0:point/16;
            speedN=speed[point1]+(abs((speed[point1+1]-speed[point1])/16.0)*(point-(point1*16)));
            break;
    }
    return speedN;
}

//Отображение диаграммы
void MainWindow::showSpeed()
{
   QSlider * slider_array[17]={ui->m0,ui->m16,ui->m32,ui->m48,ui->m64,
                                ui->m80,ui->m96,ui->m112,ui->m128,ui->m144,
                                ui->m160,ui->m176,ui->m192,ui->m208,ui->m224,ui->m240,ui->m255};
   QLabel* label_array[17]={ui->l0,ui->l16,ui->l32,ui->l48,ui->l64,
                             ui->l80,ui->l96,ui->l112,ui->l128,ui->l144,
                             ui->l160,ui->l176,ui->l192,ui->l208,ui->l224,ui->l240,ui->l255};
   int maxim=speed[0];

   if(ui->speedType->currentIndex()==2)
   {
       for(int i=0;i<17;i++){
           if(maxim<speed[i]) maxim=speed[i];
       }
   }
   else if(ui->speedType->currentIndex()==0){
       maxim=(ui->blackSpeed->value()>=ui->whiteSpeed->value()) ? ui->blackSpeed->value() : ui->whiteSpeed->value();
   }
   else{
       maxim=(this->getSpeed(0)>this->getSpeed(255)) ? this->getSpeed(0) : this->getSpeed(255);
   }
   for(int i=0,j=0;i<17;i++){
       slider_array[i]->setMaximum(this->getSpeed((i*16)-j)+100000);
       slider_array[i]->setMinimum(0);
       slider_array[i]->setValue(this->getSpeed((i*16)-j));
       slider_array[i]->setMaximum((int)(maxim*1.1));

       QObject::connect(slider_array[i], SIGNAL(valueChanged(int)),this, SLOT(on_m0_valueChanged(int)));
       label_array[i]->setText(QString::number(this->getSpeed((i*16)-j)));
       if(ui->speedType->currentIndex()!=2){j=1;}
   }
}

//Сохранение настроек в системе
void MainWindow::on_saveSettings_clicked()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("sett/kSpeed",ui->k->value());
    settings.setValue("sett/freeStep",ui->freeStep->text());
    settings.setValue("sett/workStep",ui->workStep->text());
    settings.setValue("sett/startLine",ui->startLine->toPlainText());
    settings.setValue("sett/endLine",ui->endLine->toPlainText());
    settings.setValue("sett/middleLine",ui->middleLine->toPlainText());
    settings.setValue("sett/firstAxe",ui->firstAxe->text());
    settings.setValue("sett/secondAxe",ui->secondAxe->text());
    settings.setValue("sett/speedType",ui->speedType->currentIndex());
    settings.setValue("sett/wipeCount",ui->wipeCount->value());
    settings.setValue("sett/wipeCover",ui->wipeCover->currentIndex());
    settings.setValue("sett/wipeSpeed",ui->wipeSpeed->currentIndex());
    settings.setValue("sett/wipeWidth",ui->wipeWidth->value());
    settings.setValue("sett/sting",ui->sting->isChecked());
    settings.setValue("sett/maxWipe",ui->maxWipe->value());
    settings.setValue("sett/firstAxeInvert",ui->invertFirst->isChecked());
    settings.setValue("sett/secondAxeInvert",ui->invertSecond->isChecked());
    settings.setValue("sett/parking",ui->parking->isChecked());


    settings.setValue("sett/compressGCode",ui->compressGcode->isChecked());
    settings.setValue("sett/useShifts",ui->useShifts->isChecked());



    settings.setValue("sett/pauseCommand",ui->pauseCommand->text());
    settings.setValue("sett/spindleOnCommand",ui->spindleOnCommand->text());
    settings.setValue("sett/spindleOffCommand",ui->spindleOffCommand->text());

    //Всё что касается параметров выжигания
    settings.setValue("sett/methodBurning",ui->methodBurning->currentIndex());

    settings.setValue("sett/density",ui->density->value());
    //Скорости выставленные вручную
    settings.beginWriteArray("sett/speed");
    for (int i = 0; i < 17; i++) {
        settings.setArrayIndex(i);
        settings.setValue(QString::number(i), this->speed[i]);
    }
    settings.endArray();

    settings.setValue("sett/burnStyle",ui->burnStyle->currentIndex());
    settings.setValue("sett/slideType",ui->slideType->currentIndex());
    settings.setValue("sett/blackSpeed",ui->blackSpeed->value());
    settings.setValue("sett/whiteSpeed",ui->whiteSpeed->value());
    QMessageBox::information(0,"Информация","Параметры успешно сохранены!");
}

//Сброс настроек по умолчанию
void MainWindow::defaultSetting(bool def)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    if( settings.value("sett/kSpeed")>1 ||  settings.value("sett/kSpeed")<=0 || def){
          settings.setValue("sett/kSpeed","0.5");
            ui->k->setValue(settings.value("sett/kSpeed").toDouble());
            settings.setValue("sett/density",0.01);
           //Команды
           settings.setValue("sett/startLine","G00 X0 Y0\r\nM3 S1000\r\nG04 X0 P10\r\n");
           settings.setValue("sett/middleLine","");
           settings.setValue("sett/endLine","M5\r\nG00 X0\r\nG04 P20\r\nG00 Y0\r\n");

           //Настройка осей
           settings.setValue("sett/firstAxe","X");
           settings.setValue("sett/firstAxeInvert",false);
           settings.setValue("sett/secondAxe","Y");
           settings.setValue("sett/secondAxeInvert",false);
           settings.setValue("sett/slideType","1"); //Лазер или Нихромовая проволока
           settings.setValue("sett/parking",true);

           //Протирка жала
           settings.setValue("sett/wipeCount",2);
           settings.setValue("sett/wipeCover",0);
           settings.setValue("sett/wipeWidth",5);
           settings.setValue("sett/sting",true);
           settings.setValue("sett/maxWipe",5);
           settings.setValue("sett/wipeSpeed",3);


           //GCODE синтаксис
           settings.setValue("sett/freeStep","G00");
           settings.setValue("sett/workStep","G01");
           settings.setValue("sett/pauseCommand","G04 P");
           settings.setValue("sett/spindleOnCommand","M3 S");
           settings.setValue("sett/spindleOffCommand","M5");
           settings.setValue("sett/compressGCode",true);
           settings.setValue("sett/useShifts",true);

           //Всё что касается параметров выжигания
           settings.setValue("sett/methodBurning",0);
           settings.setValue("sett/blackSpeed","350");
           settings.setValue("sett/whiteSpeed","5010");
           settings.beginWriteArray("sett/speed");
           for (int i = 0; i < 17; i++) {
               settings.setArrayIndex(i);
               settings.setValue(QString::number(i), 40+i*25);
           }
           settings.endArray();
    }
    //Скорости выставленные вручную
    settings.beginReadArray("sett/speed");
    for (int i = 0; i < 17;i++) {
        settings.setArrayIndex(i);
        this->speed[i]=settings.value(QString::number(i)).toInt();
    }
    settings.endArray();

    ui->speedType->setCurrentIndex(-1);

    ui->zagWidth->setValue(settings.value("sett/zagWidth").toInt());


    ui->density->setValue(settings.value("sett/density").toDouble());
    ui->k->setValue(settings.value("sett/kSpeed").toDouble());
    ui->burnStyle->setCurrentIndex(-1);
    ui->burnStyle->setCurrentIndex(settings.value("sett/burnStyle").toInt());
    ui->blackSpeed->setValue((ui->speedType->currentIndex()!=2)?settings.value("sett/blackSpeed").toInt():speed[0]);
    ui->whiteSpeed->setValue((ui->speedType->currentIndex()!=2)?settings.value("sett/whiteSpeed").toInt():speed[16]);
    ui->speedType->setCurrentIndex(settings.value("sett/speedType").toInt());

    //Команды
    ui->startLine->setPlainText(settings.value("sett/startLine").toString());
    ui->middleLine->setPlainText(settings.value("sett/middleLine").toString());
    ui->endLine->setPlainText(settings.value("sett/endLine").toString());

    //Настройка осей
    ui->firstAxe->setText(settings.value("sett/firstAxe").toString());
    ui->invertFirst->setChecked(settings.value("sett/firstAxeInvert").toBool());
    ui->secondAxe->setText(settings.value("sett/secondAxe").toString());

    ui->invertSecond->setChecked(settings.value("sett/secondAxeInvert").toBool());
    ui->slideType->setCurrentIndex(settings.value("sett/slideType").toInt());
    ui->parking->setChecked(settings.value("sett/parking").toBool());


    ui->methodBurning->setCurrentIndex(settings.value("sett/methodBurning").toInt());

    //Протирка жала
    ui->maxWipe->setValue(settings.value("sett/maxWipe").toInt());
    ui->wipeCount->setValue(settings.value("sett/wipeCount").toInt());
    ui->wipeCover->setCurrentIndex(settings.value("sett/wipeCover").toInt());
    ui->wipeWidth->setValue(settings.value("sett/wipeWidth").toInt());
    ui->sting->setChecked(settings.value("sett/sting").toBool());
    ui->wipeSpeed->setCurrentIndex(settings.value("sett/wipeSpeed").toInt());

    //GCODE синтаксис
    ui->freeStep->setText(settings.value("sett/freeStep").toString());
    ui->workStep->setText(settings.value("sett/workStep").toString());
    ui->pauseCommand->setText(settings.value("sett/pauseCommand").toString());
    ui->spindleOnCommand->setText(settings.value("sett/spindleOnCommand").toString());
    ui->spindleOffCommand->setText(settings.value("sett/spindleOffCommand").toString());
    ui->compressGcode->setChecked(settings.value("sett/compressGCode").toBool());
    ui->useShifts->setChecked(settings.value("sett/useShifts").toBool());
}

//изменение коэффициента
void MainWindow::on_k_valueChanged(double arg1)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    ui->whiteSpeed->setValue(this->getSpeed(255));
    this->showSpeed();
}

//изменение скорости белого оттенка
void MainWindow::on_whiteSpeed_valueChanged(int index)
{
    if(ui->speedType->currentIndex()==2){
        speed[16]=index;
    }
    this->showSpeed();
}

//изменение скорости темного оттенка
void MainWindow::on_blackSpeed_valueChanged(int value)
{

    switch (ui->speedType->currentIndex()) {
        case 1: ui->whiteSpeed->setValue(getSpeed(255));break;
        case 2: speed[0]=value; break;
    }
    this->showSpeed();
}

//выставление параметров по оттенками
 void MainWindow::on_m0_valueChanged(int value)
 {
     QObject* obj = sender();

     QSlider * slider_array[17]={ui->m0,ui->m16,ui->m32,ui->m48,ui->m64,
                                 ui->m80,ui->m96,ui->m112,ui->m128,ui->m144,
                                 ui->m160,ui->m176,ui->m192,ui->m208,ui->m224,ui->m240,ui->m255};
     QLabel* label_array[17]={ui->l0,ui->l16,ui->l32,ui->l48,ui->l64,
                              ui->l80,ui->l96,ui->l112,ui->l128,ui->l144,
                              ui->l160,ui->l176,ui->l192,ui->l208,ui->l224,ui->l240,ui->l255};
     for(int i=0;i<17;i++){
         if(obj==slider_array[i]){
             if(i==0){
                 ui->blackSpeed->setValue(value);
             }
             else if(i==16){
                 ui->whiteSpeed->setValue(value);
             }
             speed[i]=value;

             label_array[i]->setText(QString::number(value));
         }
     }

 }

// Выставление параметра для диаграммы
void MainWindow::on_speedType_currentIndexChanged(int index)
{
    int stat=false;
    ui->whiteSpeed->setEnabled(true);
    ui->k->setEnabled(false);
    if(index==2){
        stat=true;
    }
    if(index==1){
        ui->whiteSpeed->setEnabled(false);
        ui->k->setEnabled(true);
    }
    QSlider * slider_array[17]={ui->m0,ui->m16,ui->m32,ui->m48,ui->m64,ui->m80,ui->m96,
                                ui->m112,ui->m128,ui->m144,ui->m160,ui->m176,ui->m192,
                                ui->m208,ui->m224,ui->m240,ui->m255};
   for(int i=0;i<17;i++){
       slider_array[i]->setEnabled(stat);
   }
   this->showSpeed();
}

//Изменение стиля выжигания
void MainWindow::on_burnStyle_currentIndexChanged(int index)
{
    ui->sting->setEnabled((bool)index);
    if(index>2){
        ui->maxWipe->setEnabled(true);
        ui->maxWipel->setEnabled(true);
    }
    else{

        ui->maxWipe->setEnabled(false);
        ui->maxWipel->setEnabled(false);
    }
    this->checkWipper();
}

void MainWindow::checkWipper()
{
    ui->sting->setEnabled((ui->slideType->currentIndex()!=1)?false:true);
    ui->sting->setCheckable((ui->slideType->currentIndex()!=1)?false:true);
}

//Изменение типа каретки
void MainWindow::on_slideType_currentIndexChanged(int index)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    this->checkWipper();
    ui->parking->setEnabled(false);
    ui->parking->setChecked(false);
    if(index==0){
        ui->carriageOn->setText("Включение лазера");
        ui->carriageOff->setText("Отключение лазера");
    }
    else if(index==1){
        ui->carriageOn->setText("Включение жала");
        ui->carriageOff->setText("Отключение жала");
        ui->parking->setEnabled(true);
        ui->parking->setChecked(settings.value("sett/parking").toBool());
    }
    else{
        ui->carriageOn->setText("Опустить карандаш");
        ui->carriageOff->setText("Поднять карандаш");
    }

}

void MainWindow::on_methodBurning_currentIndexChanged(int index)
{
    switch (index) {
        case 0:ui->slideLabel->setText("Скорость (мм/сек)"); break;
        case 1:ui->slideLabel->setText("Задержка (мс)"); break;
        case 2:ui->slideLabel->setText("Мощность лазера (PWM)"); break;
    }

    this->checkWipper();
}

void MainWindow::on_action_5_triggered()
{
    this->close();
}

void MainWindow::on_gainEffect_clicked()
{
    shades_activated();
}

void MainWindow::on_pencilEffect_toggled(bool arg1)
{
    ui->gainEffect->setEnabled(arg1);
}
