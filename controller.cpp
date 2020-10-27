#include "controller.h"
#include <QSerialPort>
#include <QSerialPortInfo>

QSerialPort serial;
bool play;
bool stop;

QString endCommand="/n";

Controller::Controller(QObject *parent) : QObject(parent)
{

}

void Controller::connect(QString port,int speed){
    serial.close();
    serial.setPortName(port);
    //setup COM port
    switch(speed){
         case 1200:serial.setBaudRate(QSerialPort::Baud1200);break;
         case 2400:serial.setBaudRate(QSerialPort::Baud2400);break;
         case 4800:serial.setBaudRate(QSerialPort::Baud4800);break;
         case 9600:serial.setBaudRate(QSerialPort::Baud9600);break;
         case 19200:serial.setBaudRate(QSerialPort::Baud19200);break;
         case 38400:serial.setBaudRate(QSerialPort::Baud38400);break;
         case 57600:serial.setBaudRate(QSerialPort::Baud57600);break;
         case 115200:serial.setBaudRate(QSerialPort::Baud115200);break;
    }
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.open(QSerialPort::ReadWrite);
    if(serial.open()){
        emit con("Соединен "+port+"Скорость"+speed);
    }
}

void Controller::read(){
    messageController+=serial.readAll();
    emit read(messageController);


}

void Controller::write(QString command){
    command+=endCommand;
    serial.write(command.toLocal8Bit());
}

void Controller::burn(){
    QFile gcode("GCODE");
    if(gcode.open(QIODevice::ReadOnly | QIODevice::Text)){
        QByteArray str;
        int n = QTextStream(&gcode).readAll().split('\n').count();
        gcode.close();
        gcode.open(QIODevice::ReadOnly | QIODevice::Text);
        ui->progressBar->setMaximum(n);
        int i=0;
        while(!gcode.atEnd()){
            str=gcode.readLine()+"\n";
            serial.write(str);
            QObject().thread()->usleep(1000*1000*0.2);
            ui->progressBar->setValue(i++);
        }
    }
}
