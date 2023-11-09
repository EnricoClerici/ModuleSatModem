#include <QTimer>
#include <QDateTime>

#include "SatModemDev.h"

using namespace SatModemRes;


SatModemDev::SatModemDev()
{
    isDeviceBusy = false;

    readyCheckTimer = new QTimer(this);
    readyCheckTimer->setInterval(READY_CHECK_TIME);
    connect(readyCheckTimer, &QTimer::timeout, this, &SatModemDev::checkDevice);
    readyCheckTimer->start();
}


void SatModemDev::setChannel(GTModuleChannel *channel)
{
    deviceChannel = channel;
    deviceDecoder = static_cast<SatModemDec *>(deviceChannel->decoder());
    connect(deviceDecoder, &SatModemDec::newMsg, this, &SatModemDev::onNewMsg);
}


void SatModemDev::sendMsg(SatModemMsg msg, int timeOut)
{
    isDeviceBusy = true;
    bullet = msg;
    bullet.setStatus(MSGSTATUS_LOADED);
    if (deviceChannel) {
        QByteArray pck;
        pck += PCK_START;
        pck += bullet.getID();
        pck += QString("%1").arg(bullet.getOrigin(), 2, 10, QChar('0'));
        pck += QString("%1").arg(bullet.getDestination(), 2, 10, QChar('0'));
        pck += static_cast<char>(bullet.getType());
        pck += bullet.getPayload();
        pck += PCK_END;
        if(deviceChannel->write(pck)){
            bullet.setStatus(MSGSTATUS_SENT);
            //qDebug() << QDateTime::currentMSecsSinceEpoch() << "Message sent:" << pck;
        }
        else
            bullet.setStatus(MSGSTATUS_REJECTED);
    }
    QTimer::singleShot(timeOut, this, &SatModemDev::freeDevice);
}


void SatModemDev::onNewMsg(QByteArray msg)
{
    SatModemMsg newMsg;
    QByteArray tempID;
    newMsg.setID(msg.mid(1,8));
    newMsg.setOrigin(msg.mid(9,2).toInt());
    newMsg.setDestination(msg.mid(11,2).toInt());
    newMsg.setType(static_cast<char>(msg[13]));
    newMsg.setPayload(QString(msg.mid(14, msg.size() - MIN_PCK_SIZE)));
    newMsg.setStatus(MSGSTATUS_RECEIVED);
    //qDebug() << QDateTime::currentMSecsSinceEpoch() << "Message received:" << msg;
    if(newMsg.getDestination() == deviceID || newMsg.getDestination() == BCST_ID){
        if(newMsg.getType() == MSGTYPE_BCSTCMD || newMsg.getType() == MSGTYPE_SNGLCMD)
            emit newSatModemCmdMsg(newMsg);
        else if(newMsg.getType() == MSGTYPE_BCSTREQ || newMsg.getType() == MSGTYPE_SNGLREQ)
            emit newSatModemReqMsg(newMsg);
        else if(newMsg.getType() == MSGTYPE_REPLY)
            emit newSatModemReplyMsg(newMsg);
        else if(newMsg.getType() == MSGTYPE_DISCOVERY)
            emit newSatModemDiscovery(newMsg);
        else if(newMsg.getType() == MSGTYPE_DISCOVERY_REP)
            emit newSatModemDiscoveryRep(newMsg);
        else if(newMsg.getType() == MSGTYPE_CONFIG_REQ)
            emit newSatModemConfigReq(newMsg);
        else if(newMsg.getType() == MSGTYPE_CONFIG_REP)
            emit newSatModemConfigRep(newMsg);
        else if(newMsg.getType() == MSGTYPE_EXT_BCSTCMD || newMsg.getType() == MSGTYPE_EXT_SNGLCMD)
            emit newSatModemExtCmdMsg(newMsg);
        else if(newMsg.getType() == MSGTYPE_EXT_BCSTREQ || newMsg.getType() == MSGTYPE_EXT_SNGLREQ)
            emit newSatModemExtReqMsg(newMsg);
        else if(newMsg.getType() == MSGTYPE_EXT_REPLY)
            emit newSatModemExtReplyMsg(newMsg);
    }
}


void SatModemDev::freeDevice()
{
    transferDone(bullet);
    isDeviceBusy = false;
}


void SatModemDev::checkDevice()
{
    if(!isDeviceBusy)
        emit deviceReady();
}
