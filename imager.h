#ifndef IMAGER_H
#define IMAGER_H

#include <QObject>

class Imager : public QObject
{
    Q_OBJECT
public:
    explicit Imager(QObject *parent = 0);
    saveGcode();

signals:

public slots:
};

#endif // IMAGER_H
