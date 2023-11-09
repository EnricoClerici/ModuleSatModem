#include <QThread>

#include "SatModemMod.h"
#include "SatModemRes.h"

using namespace SatModemRes;
using namespace GTModuleRes;

SatModemModule::SatModemModule(MsgLevel msgLevel ,QObject *parent )
    : GTModule("SATMODEM",SATMODEMMODULE_VER,msgLevel,parent)
{

}


bool SatModemModule::appInit()
{
    if (!installChannels())   return false;
    if (!installParameters()) return false;
    if (!installTimers())     return false;
    if (!installSharedMems()) return false;
    if (!localInit()) return false;

    return true;
}


bool SatModemModule::appActivate()
{
    return true;
}


bool SatModemModule::configModule()
{
    return true;
}


bool SatModemModule::appResourcesConfig()
{
    info("Initializing local resources");

    qRegisterMetaType<SatModemDec>("SATMODEM");
    qRegisterMetaType<SatModemMsg>("SatModemMsg");

    satModem = new Dev();
    connect(, &SatModemDev::deviceReady, this, &SatModemModule::onDeviceReady);
    connect(satModem, &SatModemDev::transferDone, this, &SatModemModule::onTransferDone);
    connect(satModem, &SatModemDev::newSatModemCmdMsg, this, &SatModemModule::onNewSatModemCmdMsg);
    connect(satModem, &SatModemDev::newSatModemReqMsg, this, &SatModemModule::onNewSatModemReqMsg);
    connect(satModem, &SatModemDev::newSatModemReplyMsg, this, &SatModemModule::onNewSatModemReplyMsg);

    connect(satModem, &SatModemDev::newSatModemExtCmdMsg, this, &SatModemModule::onNewSatModemExtCmdMsg);
    connect(satModem, &SatModemDev::newSatModemExtReqMsg, this, &SatModemModule::onNewSatModemExtReqMsg);
    connect(satModem, &SatModemDev::newSatModemExtReplyMsg, this, &SatModemModule::onNewSatModemExtReplyMsg);


    connect(satModem, &SatModemDev::newSatModemDiscovery, this, &SatModemModule::onNewSatModemDiscoveryMsg);
    connect(satModem, &SatModemDev::newSatModemDiscoveryRep, this, &SatModemModule::onNewSatModemDiscoveryReplyMsg);
    connect(satModem, &SatModemDev::newSatModemConfigRep, this, &SatModemModule::onNewSatModemConfigRepMsg);
    connect(satModem, &SatModemDev::newSatModemConfigReq, this, &SatModemModule::onNewSatModemConfigReqMsg);
    return true;
}


bool SatModemModule::installChannels()
{
    if(this->name().endsWith("BOA")){

        if ((deviceCh = channels.get("SATMODEM_DEV_BOA"))==nullptr) {error(channels.errString()); return false;}
        satModem->setChannel(deviceCh);

        if ((moduleCh = channels.get("SATMODEM_MOD_BOA"))==nullptr) {error(channels.errString()); return false;}
        if ((userControlCh = channels.get("USER_CONTROL"))==nullptr) {error(channels.errString()); return false;}

        userControlDec = static_cast<GTNMEADecoder *> (userControlCh->decoder());
        connect(userControlDec, &GTNMEADecoder::NMEAPacket, this, &SatModemModule::onUserControlMsg);

        moduleDec = static_cast<GTNMEADecoder *> (moduleCh->decoder());
        connect(moduleDec, &GTNMEADecoder::NMEAPacket, this, &SatModemModule::onModuleMsg);

        if ((externalCh = static_cast<GTModuleTCPServerChannel *>(channels.get("EXTERNAL")))==nullptr) {error(channels.errString()); return false;}
        connect(externalCh,&GTModuleTCPServerChannel::newConnection, this, &SatModemModule::onNewExtBoaConnection);

        //externalDec = static_cast<GTNMEADecoder *> (externalCh->decoder());
        //connect(externalDec, &GTTextDecoder::textPacket, this, &SatModemModule::onExternalBoaMsg);

        return true;
    }
    else if(this->name().endsWith("BASE")) {

        if ((deviceCh = channels.get("SATMODEM_DEV_BASE"))==nullptr) {error(channels.errString()); return false;}
        satModem->setChannel(deviceCh);

        if ((moduleCh = channels.get("SATMODEM_MOD_BASE"))==nullptr) {error(channels.errString()); return false;}

        if ((externalCh = static_cast<GTModuleTCPServerChannel *>(channels.get("EXTERNAL")))==nullptr) {error(channels.errString()); return false;}
        connect(externalCh,&GTModuleTCPServerChannel::newConnection, this, &SatModemModule::onNewExtBaseConnection);

        //externalDec = static_cast<GTTextDecoder *> (externalCh->decoder());
        //connect(externalDec, &GTTextDecoder::textPacket, this, &SatModemModule::onExternalBaseMsg);

        return true;

    }
    else {
        return false;
    }
}


bool SatModemModule::installParameters()
{
    if (!GTModuleRes::params.setUsrPtr("ID",&localDevice.deviceID)) { error(GTModuleRes::params.errString()); return false;}

    return true;
}


bool SatModemModule::installTimers()
{
    return true;
}


bool SatModemModule::installSharedMems()
{
    return true;
}

bool SatModemModule::localInit()
{
    snglReqQueueSize = DEFAULT_QU_SIZE;
    snglCmdQueueSize = DEFAULT_QU_SIZE;
    bcstReqQueueSize = DEFAULT_QU_SIZE;
    bcstCmdQueueSize = DEFAULT_QU_SIZE;
    extSnglReqQueueSize = DEFAULT_QU_SIZE;
    extSnglCmdQueueSize = DEFAULT_QU_SIZE;
    extBcstReqQueueSize = DEFAULT_QU_SIZE;
    extBcstCmdQueueSize = DEFAULT_QU_SIZE;
    discoveryTime = DISCOVERY_TIME;
    reqIdTime = WAIT_ID_TIME;
    localDevice.id = DEFAULT_ID;
    localDevice.slotTime = DEFAULT_SLOT_TIME;
    localDevice.slot = localDevice.deviceID;
    localDevice.isReady = false;
    localDevice.ip = static_cast<GTModuleUDPChannel *>(moduleCh)->destAddr().at(0);
    satModem->setID(localDevice.deviceID);
    isDiscoveryOn = false;

    updateTiming();

    if(this->name().endsWith("BOA"))
        QTimer::singleShot(reqIdTime, this, &SatModemModule::onReqIdTimeout);

    return true;
}


void SatModemModule::onNewExtBoaConnection(int id)
{
    GTNMEAModuleCmdDecoder * dec = static_cast<GTNMEAModuleCmdDecoder *>(externalCh->connDecoder(id));
    if (dec == nullptr) {
        return;
    }
    externalDecMap.insert(dec,id);
    connect(dec,&GTTextDecoder::textPacket, this, &SatModemModule::onExternalBoaMsg);
}


void SatModemModule::onNewExtBaseConnection(int id)
{
    GTNMEAModuleCmdDecoder * dec = static_cast<GTNMEAModuleCmdDecoder *>(externalCh->connDecoder(id));
    if (dec == nullptr) {
        return;
    }
    externalDecMap.insert(dec,id);
    connect(dec, &GTTextDecoder::textPacket, this, &SatModemModule::onExternalBaseMsg);
}


void SatModemModule::onMsg(MsgType what, QString who, QString payload)
{
    GTObject::onMsg(what,who,payload);
}


void SatModemModule::onNMEACmd(QString cmdType, QString code, QList<QVariant> params)
{
    if (cmdType == "MESSAGE")
    {
        if(localDevice.isReady){
            SatModemMsg tempMsg;
            tempMsg.setOrigin(static_cast<char>(localDevice.deviceID));
            tempMsg.setPayload(params[0].toString());
            tempMsg.setStatus(MSGSTATUS_CREATED);

            if(code == "SENDBCSTREQ") {
                tempMsg.setType(MSGTYPE_BCSTREQ);
                tempMsg.setDestination(BCST_ID);
            }
            else if(code == "SENDBCSTCMD"){
                tempMsg.setType(MSGTYPE_BCSTCMD);
                tempMsg.setDestination(BCST_ID);
            }
            else if(code == "SENDSNGLREQ") {
                tempMsg.setType(MSGTYPE_SNGLREQ);
                tempMsg.setDestination(static_cast<char>(params[1].toInt()));
            }
            else if(code == "SENDSNGLCMD") {
                tempMsg.setType(MSGTYPE_SNGLCMD);
                tempMsg.setDestination(static_cast<char>(params[1].toInt()));
            }
            scheduleMsg(tempMsg);
        }
        else {
            sendSatModemModule("$SENDERROR,Device not ready ");
        }
    }
    else if(cmdType == "CONFIG") {
        if(isDiscoveryOn){
            sendSatModemModule("$CFGERROR,Discovery ongoing");
        }
        else if(code == "CFGRQDISCOVERY") {
            if(params[0].toInt() < 10 || params[0].toInt() > 10000)
                sendSatModemModule("$CFGERROR,Invalid parameters");
            else {
                localDevice.slotTime = params[0].toInt();
                discoverDevices();
            }
        }
        else if(code == "CFGRQSETQUSIZE"){
            if(params[0].toInt() > 0 && params[0].toInt() < 100)
                bcstReqQueueSize = params[0].toInt();
            else
                sendSatModemModule("$CFGERROR,Invalid broadcast request size");
            if(params[1].toInt() > 0 && params[1].toInt() < 100)
                bcstCmdQueueSize = params[1].toInt();
            else
                sendSatModemModule("$CFGERROR,Invalid broadcast command size");
            if(params[2].toInt() > 0 && params[2].toInt() < 100)
                snglReqQueueSize = params[2].toInt();
            else
                sendSatModemModule("$CFGERROR,Invalid single request size");
            if(params[3].toInt() > 0 && params[3].toInt() < 100)
                snglCmdQueueSize = params[3].toInt();
            else
                sendSatModemModule("$CFGERROR,Invalid single command size");
        }
        else if(code == "CFGRQGETQUSIZE"){
                sendSatModemModule(QString("$CFGRPQUSIZE,%1,%2,%3,%4").arg(QString::number(bcstReqQueueSize))
                                     .arg(QString::number(bcstCmdQueueSize)).arg(QString::number(snglReqQueueSize))
                                     .arg(QString::number(snglCmdQueueSize)));
        }
    }
    else if(cmdType == "STATUS") {
        if(code == "QUSTATUSREQ"){
            sendSatModemModule(QString("$QUSTATUS,%1,%2,%3,%4")
                                 .arg(QString::number(bcstReqQueueSize - bcstReqQueue.size()))
                                 .arg(QString::number(bcstCmdQueueSize - bcstCmdQueue.size()))
                                 .arg(QString::number(snglReqQueueSize - snglReqQueue.size()))
                                 .arg(QString::number(snglCmdQueueSize - snglCmdQueue.size())));
        }
    }
}


void SatModemModule::onModuleMsg(QString code, QStringList params)
{
    if(reply.getStatus() == MSGSTATUS_EXPECTED_IN_SLOT || reply.getStatus() == MSGSTATUS_EXPECTED_IMMEDIATE){
        QString pld = "#" + code + ";" + params.join(";");
        reply.setPayload(pld);
        reply.setStatus(MSGSTATUS_SCHEDULED);
    }
    else if(reply.getStatus() == MSGSTATUS_SCHEDULED){
        QString pld = "#" + code + ";" + params.join(";");
        reply.setPayload(pld);
        reply.setStatus(MSGSTATUS_OVERWRITTEN);
        reply.setStatus(MSGSTATUS_SCHEDULED);
    }
    else {
        reply.setStatus(MSGSTATUS_REJECTED);
    }
}


void SatModemModule::onExternalBaseMsg(QString msg)
{
    QStringList fields = msg.split('!');

    if(localDevice.isReady){
        if(fields.size() == EXT_MSG_SIZE){
            SatModemMsg tempMsg;
            tempMsg.setStatus(MSGSTATUS_CREATED);
            tempMsg.setOrigin(static_cast<char>(localDevice.deviceID));
            tempMsg.setPayload(fields[2]);
            if(fields.at(0) == "REQ"){
                int dest = fields.at(1).toInt();
                if(dest == BCST_ID){
                    tempMsg.setType(MSGTYPE_EXT_BCSTREQ);
                    tempMsg.setDestination(BCST_ID);
                    scheduleExtMsg(tempMsg);
                }
                else if(dest != 0 && dest < BCST_ID){
                    tempMsg.setType(MSGTYPE_EXT_SNGLREQ);
                    tempMsg.setDestination(static_cast<char>(dest));
                    scheduleExtMsg(tempMsg);
                }
                else {
                    sendExternal("Invalid ID");
                    tempMsg.setStatus(MSGSTATUS_REJECTED);
                }
            }
            else if(fields.at(0) == "CMD"){
                int dest = fields.at(1).toInt();
                if(dest == BCST_ID){
                    tempMsg.setType(MSGTYPE_EXT_BCSTCMD);
                    tempMsg.setDestination(BCST_ID);
                    scheduleExtMsg(tempMsg);
                }
                else if(dest != 0 && dest < BCST_ID){
                    tempMsg.setType(MSGTYPE_EXT_SNGLCMD);
                    tempMsg.setDestination(static_cast<char>(dest));
                    scheduleExtMsg(tempMsg);
                }
                else {
                    sendExternal("Invalid ID");
                    tempMsg.setStatus(MSGSTATUS_REJECTED);
                }
            }
            else {
                sendExternal("Invalid message type");
                tempMsg.setStatus(MSGSTATUS_REJECTED);
            }
        }
        else if(fields.size() && fields.at(0) == "STATUS"){
            if(localDevice.isReady){
                sendExternal("Device ready");
            }
        }
        else {
            sendExternal("Invalid message format");
        }
    }
    else {
        sendExternal("Device not ready");
    }
}


void SatModemModule::onExternalBoaMsg(QString msg)
{
    if(extReply.getStatus() == MSGSTATUS_EXPECTED_IMMEDIATE || extReply.getStatus() == MSGSTATUS_EXPECTED_IN_SLOT){
        extReply.setStatus(MSGSTATUS_SCHEDULED);
        extReply.setPayload(msg);
    }
    else if(extReply.getStatus() == MSGSTATUS_SCHEDULED){
        extReply.setStatus(MSGSTATUS_OVERWRITTEN);
        extReply.setStatus(MSGSTATUS_SCHEDULED);
        extReply.setPayload(msg);
    }
    else {
        extReply.setStatus(MSGSTATUS_REJECTED);
    }
}


void SatModemModule::onUserControlMsg(QString code, QStringList params)
{
    if(code == "ID")
        localDevice.id = params.at(0);
}


void SatModemModule::onNewSatModemCmdMsg(SatModemMsg msg)
{
    if(localDevice.isReady)
        sendSatModemModule(msg.getPayload().replace("#","$").replace(";",","));
    else {
        msg.setStatus(MSGSTATUS_REJECTED);
    }
}


void SatModemModule::onNewSatModemReqMsg(SatModemMsg msg)
{
    if(localDevice.isReady){
        reply.setID(msg.getID());
        reply.setDestination(msg.getOrigin());
        reply.setOrigin(localDevice.deviceID);
        reply.setType(MSGTYPE_REPLY);
        if(msg.getDestination() == localDevice.deviceID){
            QTimer::singleShot(waitReplyTime, this, &SatModemModule::onReplyReady);
            reply.setStatus(MSGSTATUS_EXPECTED_IMMEDIATE);
        }
        else if(msg.getDestination() == BCST_ID){
            QTimer::singleShot(replyInSlotTime, this, &SatModemModule::onReplyReady);
            reply.setStatus(MSGSTATUS_EXPECTED_IN_SLOT);
        }
        sendSatModemModule(msg.getPayload().replace("#","$").replace(";",","));
    }
    else {
        msg.setStatus(MSGSTATUS_REJECTED);
    }
}


void SatModemModule::onNewSatModemReplyMsg(SatModemMsg msg)
{
    if(msg.getID() == expectedRepID && !repliedDev.contains(msg.getOrigin())){
        repliedDev.push_back(msg.getOrigin());
        sendSatModemModule(QString("$RECVREPLY,%1,%2").arg(msg.getOrigin()).arg(QString(msg.getPayload())));
    }
}


void SatModemModule::onNewSatModemExtCmdMsg(SatModemMsg msg)
{
    if(localDevice.isReady)
        sendExternal("RECVCMD!" + msg.getPayload());
    else {
        msg.setStatus(MSGSTATUS_REJECTED);
    }
}


void SatModemModule::onNewSatModemExtReqMsg(SatModemMsg msg)
{
    if(localDevice.isReady){
        extReply.setID(msg.getID());
        extReply.setDestination(msg.getOrigin());
        extReply.setOrigin(localDevice.deviceID);
        extReply.setType(MSGTYPE_EXT_REPLY);
        if(msg.getDestination() == localDevice.deviceID){
            QTimer::singleShot(waitReplyTime, this, &SatModemModule::onExtReplyReady);
            extReply.setStatus(MSGSTATUS_EXPECTED_IMMEDIATE);
        }
        else if(msg.getDestination() == BCST_ID){
            QTimer::singleShot(replyInSlotTime, this, &SatModemModule::onExtReplyReady);
            extReply.setStatus(MSGSTATUS_EXPECTED_IN_SLOT);
        }
        sendExternal("RECVREQ!" + msg.getPayload());
    }
    else {
        msg.setStatus(MSGSTATUS_REJECTED);
    }
}


void SatModemModule::onNewSatModemExtReplyMsg(SatModemMsg msg)
{
    if(msg.getID() == expectedExtRepID && !extRepliedDev.contains(msg.getOrigin())){
        extRepliedDev.push_back(msg.getOrigin());
        sendExternal(QString("RECVREP!%1!%2").arg(msg.getOrigin()).arg(QString(msg.getPayload())));
    }
}


void SatModemModule::onNewSatModemDiscoveryMsg(SatModemMsg msg)
{
    localDevice.slot = localDevice.deviceID;
    localDevice.slotTime = DEFAULT_SLOT_TIME;
    updateTiming();
    reply.setID(msg.getID());
    reply.setDestination(msg.getOrigin());
    reply.setOrigin(localDevice.deviceID);
    reply.setType(MSGTYPE_DISCOVERY_REP);
    reply.setStatus(MSGSTATUS_EXPECTED_IN_SLOT);
    reply.setPayload(localDevice.id + "," + localDevice.ip);
    reply.setStatus(MSGSTATUS_SCHEDULED);
    QTimer::singleShot(replyInSlotTime, this, &SatModemModule::onReplyReady);
}


void SatModemModule::onNewSatModemDiscoveryReplyMsg(SatModemMsg msg)
{
    QString pld = msg.getPayload();
    QStringList info = pld.split(',');
    if(info.size() == INFO_SIZE){
        bool isDeviceListed = false;
        for(int i=0; i < boaList.size(); i++){
            if(boaList[i].deviceID == msg.getOrigin())
                isDeviceListed = true;
        }
        if(!isDeviceListed){
            Boa tempBoa;
            tempBoa.deviceID = msg.getOrigin();
            tempBoa.id = info.at(0);
            tempBoa.ip = info.at(1);
            tempBoa.isReady = false;
            boaList.push_back(tempBoa);
        }
    }
}


void SatModemModule::onNewSatModemConfigReqMsg(SatModemMsg msg)
{
    QString pld = msg.getPayload();
    QStringList config = pld.split(',');
    if(localDevice.deviceID != MASTER_DEVICEID){
        localDevice.slot = config.at(0).toInt();
        localDevice.slotTime = config.at(1).toInt();
        localDevice.isReady = true;
        updateTiming();
    }
    reply.setID(msg.getID());
    reply.setDestination(msg.getOrigin());
    reply.setOrigin(localDevice.deviceID);
    reply.setType(MSGTYPE_CONFIG_REP);
    reply.setStatus(MSGSTATUS_EXPECTED_IMMEDIATE);
    reply.setPayload(QString::number(localDevice.slot) + "," + QString::number(localDevice.slotTime));
    reply.setStatus(MSGSTATUS_SCHEDULED);
    QTimer::singleShot(waitReplyTime, this, &SatModemModule::onReplyReady);
}


void SatModemModule::onNewSatModemConfigRepMsg(SatModemMsg msg)
{
    QString pld = msg.getPayload();
    QStringList config = pld.split(',');
    if(config.size() == CONFIG_SIZE){
        bool isDeviceListed = false;
        int index;
        for(index=0; index < boaList.size(); index++){
            if(boaList[index].deviceID == msg.getOrigin()){
                isDeviceListed = true;
                break;
            }
        }
        if(isDeviceListed && !boaList[index].isReady){
            boaList[index].slot = config.at(0).toInt();
            boaList[index].slotTime = config.at(1).toInt();
            boaList[index].isReady = true;
        }
    }
}


bool SatModemModule::sendSatModemModule(QString pck)
{
    if (moduleCh) {
        pck.append("\r\n");
        return moduleCh->write(pck);
    }
    return false;
}


bool SatModemModule::sendExternal(QString pck)
{
    if (externalCh) {
        pck.append("\r\n");
        return externalCh->write(pck);
    }
    return false;
}


bool SatModemModule::sendUserControl(QString pck)
{
    if (userControlCh) {
        pck.append("\r\n");
        return userControlCh->write(pck);
    }
    return false;
}


void SatModemModule::onDeviceReady()
{
    QMutexLocker locker(&mtxQu);
    if(replyQueue.size()){
        satModem->sendMsg(replyQueue.takeFirst(),0);
    }
    else if(bcstCmdQueue.size()){
        satModem->sendMsg(bcstCmdQueue.takeFirst(),0);
    }
    else if(snglCmdQueue.size()){
        satModem->sendMsg(snglCmdQueue.takeFirst(),0);
    }
    else if(bcstReqQueue.size()){
        SatModemMsg tempMsg = bcstReqQueue.takeFirst();
        expectedRepID = tempMsg.getID();
        repliedDev.clear();
        expectedRepDev.clear();
        for(auto b : boaList)
            expectedRepDev.push_back(b.deviceID);
        satModem->sendMsg(tempMsg,turnaroundTime);
    }
    else if(snglReqQueue.size()){
        SatModemMsg tempMsg = snglReqQueue.takeFirst();
        expectedRepID = tempMsg.getID();
        repliedDev.clear();
        expectedRepDev.clear();
        expectedRepDev.push_back(tempMsg.getDestination());
        satModem->sendMsg(tempMsg,slotTime);
    }
    else if(extReplyQueue.size()){
        satModem->sendMsg(extReplyQueue.takeFirst(),0);
    }
    else if(extBcstCmdQueue.size()){
        satModem->sendMsg(extBcstCmdQueue.takeFirst(),0);
    }
    else if(extSnglCmdQueue.size()){
        satModem->sendMsg(extSnglCmdQueue.takeFirst(),0);
    }
    else if(extBcstReqQueue.size()){
        SatModemMsg tempMsg = extBcstReqQueue.takeFirst();
        expectedExtRepID = tempMsg.getID();
        extRepliedDev.clear();
        expectedExtRepDev.clear();
        for(auto b : boaList)
            expectedExtRepDev.push_back(b.deviceID);
        satModem->sendMsg(tempMsg,turnaroundTime);
    }
    else if(extSnglReqQueue.size()){
        SatModemMsg tempMsg = extSnglReqQueue.takeFirst();
        expectedExtRepID = tempMsg.getID();
        extRepliedDev.clear();
        expectedExtRepDev.clear();
        expectedExtRepDev.push_back(tempMsg.getDestination());
        satModem->sendMsg(tempMsg,slotTime);
    }
}


void SatModemModule::scheduleMsg(SatModemMsg msg)
{
    QMutexLocker locker(&mtxQu);
    if(!boaList.isEmpty()){
        switch (msg.getType()) {
        case MSGTYPE_BCSTCMD:
            if(bcstCmdQueue.size() < bcstCmdQueueSize){
                msg.setStatus(MSGSTATUS_SCHEDULED);
                bcstCmdQueue.push_back(msg);
            }
            else {
                msg.setStatus(MSGSTATUS_REJECTED);
                sendSatModemModule("$SENDERROR,Broadcast command queue full");
            }
            break;
        case MSGTYPE_BCSTREQ:
            if(bcstReqQueue.size() < bcstReqQueueSize){
                msg.setStatus(MSGSTATUS_SCHEDULED);
                bcstReqQueue.push_back(msg);
            }
            else {
                msg.setStatus(MSGSTATUS_REJECTED);
                sendSatModemModule("$SENDERROR,Broadcast request queue full");
            }
            break;
        case MSGTYPE_SNGLCMD:
            if(snglCmdQueue.size() < snglCmdQueueSize){
                msg.setStatus(MSGSTATUS_SCHEDULED);
                snglCmdQueue.push_back(msg);
            }
            else {
                msg.setStatus(MSGSTATUS_REJECTED);
                sendSatModemModule("$SENDERROR,Single command queue full");
            }
            break;
        case MSGTYPE_SNGLREQ:
            if(snglReqQueue.size() < snglReqQueueSize){
                msg.setStatus(MSGSTATUS_SCHEDULED);
                snglReqQueue.push_back(msg);
            }
            else {
                msg.setStatus(MSGSTATUS_REJECTED);
                sendSatModemModule("$SENDERROR,Single request queue full");
            }
            break;
        }
    }
    else {
        msg.setStatus(MSGSTATUS_REJECTED);
        sendSatModemModule("$CFGERROR,Empty list");
    }
}


void SatModemModule::scheduleExtMsg(SatModemMsg msg)
{
    QMutexLocker locker(&mtxQu);
    if(!boaList.isEmpty()){
        switch (msg.getType()) {
        case MSGTYPE_EXT_BCSTCMD:
            if(extBcstCmdQueue.size() < extBcstCmdQueueSize){
                msg.setStatus(MSGSTATUS_SCHEDULED);
                extBcstCmdQueue.push_back(msg);
            }
            else {
                msg.setStatus(MSGSTATUS_OVERWRITTEN);
                extBcstCmdQueue[0] = msg;
                sendExternal("Broadcast command overwritten");
            }
            break;
        case MSGTYPE_EXT_BCSTREQ:
            if(extBcstReqQueue.size() < extBcstReqQueueSize){
                msg.setStatus(MSGSTATUS_SCHEDULED);
                extBcstReqQueue.push_back(msg);
            }
            else {
                msg.setStatus(MSGSTATUS_OVERWRITTEN);
                extBcstReqQueue[0] = msg;
                sendExternal("Broadcast request overwritten");
            }
            break;
        case MSGTYPE_EXT_SNGLCMD:
            if(extSnglCmdQueue.size() < extSnglCmdQueueSize){
                msg.setStatus(MSGSTATUS_SCHEDULED);
                extSnglCmdQueue.push_back(msg);
            }
            else {
                msg.setStatus(MSGSTATUS_OVERWRITTEN);
                extSnglCmdQueue[0] = msg;
                sendExternal("Single command overwritten");
            }
            break;
        case MSGTYPE_EXT_SNGLREQ:
            if(extSnglReqQueue.size() < extSnglReqQueueSize){
                msg.setStatus(MSGSTATUS_SCHEDULED);
                extSnglReqQueue.push_back(msg);
            }
            else {
                msg.setStatus(MSGSTATUS_OVERWRITTEN);
                extSnglReqQueue[0] = msg;
                sendExternal("Single request overwritten");
            }
            break;
        }
    }
    else {
        msg.setStatus(MSGSTATUS_REJECTED);
        sendExternal("Empty list");
    }
}


void SatModemModule::onTransferDone(SatModemMsg transfered)
{
    if(transfered.getType() == MSGTYPE_BCSTREQ || transfered.getType() == MSGTYPE_SNGLREQ){
        if(repliedDev.isEmpty()){
            transfered.setStatus(MSGSTATUS_NOTDELIVERED);
        }
        else if(repliedDev.size() == expectedRepDev.size()){
            transfered.setStatus(MSGSTATUS_DELIVERED);
        }
        else if(repliedDev.size() < expectedRepDev.size()){
            transfered.setStatus(MSGSTATUS_PARTDELIV);
        }
    }
    else if(transfered.getType() == MSGTYPE_EXT_BCSTREQ || transfered.getType() == MSGTYPE_EXT_SNGLREQ){
        if(extRepliedDev.isEmpty()){
            transfered.setStatus(MSGSTATUS_NOTDELIVERED);
        }
        else if(extRepliedDev.size() == expectedExtRepDev.size()){
            transfered.setStatus(MSGSTATUS_DELIVERED);
        }
        else if(extRepliedDev.size() < expectedExtRepDev.size()){
            transfered.setStatus(MSGSTATUS_PARTDELIV);
        }
    }
}


void SatModemModule::onReplyReady()
{
    QMutexLocker locker(&mtxQu);
    replyQueue.clear();
    if(reply.getStatus() == MSGSTATUS_SCHEDULED)
        replyQueue.push_back(reply);
    else {
        reply.setStatus(MSGSTATUS_MISSED);
    }
}


void SatModemModule::onExtReplyReady()
{
    QMutexLocker locker(&mtxQu);
    extReplyQueue.clear();
    if(extReply.getStatus() == MSGSTATUS_SCHEDULED)
        extReplyQueue.push_back(extReply);
    else {
        extReply.setStatus(MSGSTATUS_MISSED);
    }
}


void SatModemModule::onReqIdTimeout()
{
    if(localDevice.id == DEFAULT_ID) {
        sendUserControl("$QRY_ID");
        QTimer::singleShot(reqIdTime, this, &SatModemModule::onReqIdTimeout);
    }
}


void SatModemModule::discoverDevices()
{
    isDiscoveryOn = true;
    localDevice.isReady = false;
    boaList.clear();
    SatModemMsg discovery;
    discovery.setType(MSGTYPE_DISCOVERY);
    discovery.setDestination(BCST_ID);
    discovery.setOrigin(localDevice.deviceID);
    discovery.setPayload("D");
    discovery.setStatus(MSGSTATUS_SCHEDULED);
    QMutexLocker locker(&mtxQu);
    snglCmdQueue.push_back(discovery);
    QTimer::singleShot(discoveryTime, this, &SatModemModule::onDiscoveryTimeout);
}


void SatModemModule::onDiscoveryTimeout()
{
    if(!boaList.isEmpty()){
        slotCounter = 0;
        setDeviceConfig(boaList[slotCounter].deviceID,slotCounter+1);
        updateTiming();
    }
    else {
        sendSatModemModule("$CFGERROR,Empty list");
        isDiscoveryOn = false;
    }
}


void SatModemModule::checkConfig()
{
    if(!boaList.isEmpty()){
        bool isSlotOk = true;
        bool isSlotTimeOk = true;
        int slotIndex = 1;
        for(auto b : boaList){
            if(b.slot != slotIndex)
                isSlotOk = false;
            slotIndex++;
            if(b.slotTime != localDevice.slotTime)
                isSlotTimeOk = false;
        }
        if(isSlotOk && isSlotTimeOk){
            localDevice.isReady = true;
            QString list;
            list.append("$CFGRPDEVLIST");
            for(auto b : boaList){
                list.append("," + QString::number(b.deviceID));
                list.append("," + b.id);
                list.append("," + b.ip);
            }
            sendSatModemModule(list);
            QThread::msleep(500);
            for(auto b : boaList){
                sendSatModemModule(QString("$CFGRPSLOT,%1,%2").arg(b.deviceID).arg(b.slot));
                QThread::msleep(10);
                sendSatModemModule(QString("$CFGRPSLOTTIME,%1,%2").arg(b.deviceID).arg(b.slotTime));
                QThread::msleep(10);
            }
            sendSatModemModule("$CFGSTREADY");
        }
        else {
            sendSatModemModule("$CFGERROR,Bad configuration");
        }
    }
    isDiscoveryOn = false;
}


void SatModemModule::setDeviceConfig(int id, int slot)
{
    SatModemMsg config;
    config.setType(MSGTYPE_CONFIG_REQ);
    config.setOrigin(localDevice.deviceID);
    config.setDestination(id);
    config.setPayload(QString::number(slot) + "," + QString::number(localDevice.slotTime));
    config.setStatus(MSGSTATUS_SCHEDULED);
    QMutexLocker locker(&mtxQu);
    snglCmdQueue.push_back(config);
    QTimer::singleShot(localDevice.slotTime, this, &SatModemModule::onSetConfigTimeout);
}


void SatModemModule::onSetConfigTimeout()
{
    slotCounter++;
    if(slotCounter < boaList.size()){
        setDeviceConfig(boaList[slotCounter].deviceID,slotCounter+1);
    }
    else{
        checkConfig();
    }
}


void SatModemModule::updateTiming()
{
    waitReplyTime = static_cast<int>(localDevice.slotTime * WAIT_FOR_REPLY);
    replyInSlotTime = (localDevice.slot - 1)*localDevice.slotTime + waitReplyTime;
    slotTime = localDevice.slotTime;
    turnaroundTime = boaList.size()*slotTime;
}
