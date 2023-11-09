#include <QDebug>
#include <QDateTime>

#include "SatModemDec.h"


using namespace SatModemRes;


SatModemDec::SatModemDec(QString name,bool checkCRC): GTPacketDecoder (name), m_checkCRC(checkCRC)
{
    isStartDetected = false;
}


void SatModemDec::syntaxError(QString syntaxErr)
{
    error(syntaxErr);
}


bool SatModemDec::doDecode(QByteArray packet)
{
    //qDebug() << QDateTime::currentMSecsSinceEpoch() << "Packet received:" << packet;
    for(auto p : packet){
        if(p == static_cast<char>(SatModemRes::PCK_START)) {
            data.clear();
            data.push_back(SatModemRes::PCK_START);
            isStartDetected = true;
        }
        else if (p == static_cast<char>(SatModemRes::PCK_END) && isStartDetected) {
            isStartDetected = false;
            data.push_back(SatModemRes::PCK_END);
            checkMsg(data);
            data.clear();
        }
        else {
            data.push_back(p);
            if(data.size() > SatModemRes::MAX_RECV_BUFF){
                data.clear();
                isStartDetected = false;
                error("Reception buffer overflow, reception buffer reset");
            }
        }
    }
    return true;
}


void SatModemDec::checkMsg(QByteArray msg)
{
    if (msg.size() <= SatModemRes::MIN_PCK_SIZE) {
        syntaxError("WRONG_SIZE_PACKET");
    }
    else if (msg[0] != static_cast<char>(SatModemRes::PCK_START)) {
        syntaxError("MISSED_START");
    }
    else if (msg[msg.length()-1] != static_cast<char>(SatModemRes::PCK_END)){
        syntaxError("MISSED_END");
    }
    else {
        emit newMsg(msg);
    }
}


void SatModemDec::onReceptionTimeout()
{
    data.clear();
    isStartDetected = false;
    error("Reception timeout, reception buffer reset");
}
