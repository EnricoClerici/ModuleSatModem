#ifndef SATMODEMDEV_H
#define SATMODEMDEV_H

#include <QObject>
#include <QTimer>

#include "SatModemMsg.h"
#include "GTModuleChannels.h"
#include "SatModemDec.h"

class SatModemDev : public QObject
{
    Q_OBJECT

public:
    SatModemDev();
    void setID(int id){deviceID = id;}
    int getID(){return deviceID;}
    void setDeviceChannel(GTModuleChannel *channel){deviceChannel = channel;}
    void sendMsg(SatModemMsg msg, int timeOut);
    void setChannel(GTModuleChannel *channel);

private:

private:
    int deviceID;
    bool isDeviceBusy;
    GTModuleChannel *deviceChannel;
    SatModemDec   *deviceDecoder;
    QTimer *readyCheckTimer;
    SatModemMsg bullet;

signals:
    SatModemMsg newSatModemCmdMsg(SatModemMsg msg);
    SatModemMsg newSatModemReqMsg(SatModemMsg msg);
    SatModemMsg newSatModemReplyMsg(SatModemMsg msg);

    SatModemMsg newSatModemExtCmdMsg(SatModemMsg msg);
    SatModemMsg newSatModemExtReqMsg(SatModemMsg msg);
    SatModemMsg newSatModemExtReplyMsg(SatModemMsg msg);

    SatModemMsg newSatModemDiscovery(SatModemMsg msg);
    SatModemMsg newSatModemDiscoveryRep(SatModemMsg msg);
    SatModemMsg newSatModemConfigReq(SatModemMsg msg);
    SatModemMsg newSatModemConfigRep(SatModemMsg msg);
    void deviceReady();
    void transferDone(SatModemMsg transfered);

public slots:

private slots:
    void onNewMsg(QByteArray msg);
    void freeDevice();
    void checkDevice();

};

#endif // SATMODEMDEV_H
