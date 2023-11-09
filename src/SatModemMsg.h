#ifndef SATMODEMMSG_H
#define SATMODEMMSG_H

#include <QObject>
#include <QUuid>
#include <QMutex>

#include "SatModemRes.h"


class SatModemMsg
{

public:
    SatModemMsg();

    QByteArray getID(){return id;}
    void setType(char tp){type = static_cast<SatModemRes::MsgType>(tp);}
    void setOrigin(int org){origin = org;}
    void setDestination(int dest){destination = dest;}
    void setPayload(QString pld);
    void setStatus(SatModemRes::MsgStatus st);
    void setID(QByteArray msgID){id = msgID;}
    int getOrigin(){return origin;}
    int getDestination(){return destination;}
    char getType(){return type;}
    SatModemRes::MsgStatus getStatus(){return status;}
    QByteArray getPayload(){return payload;}

    QString getStatusText(SatModemRes::MsgStatus st);
    QString getTypeText();
    QString getFormatText();

private:


private:
    QByteArray id;
    int origin;
    int destination;
    SatModemRes::MsgType type;
    SatModemRes::MsgStatus status;
    QByteArray payload;

signals:

public slots:

private slots:
    void onStatusChanged();

};

#endif // SATMODEMMSG_H
