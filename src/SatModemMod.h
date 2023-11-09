#ifndef SATMODEMMODULE_H
#define SATMODEMMODULE_H

#include <QObject>

#include "AppLib.h"
#include "GTModuleChannelDecoders.h"
#include "GTModuleChannels.h"
#include "SatModemMsg.h"
#include "SatModemDev.h"

#ifndef SATMODEMMODULE_VER
#define SATMODEMMODULE_VER "1.0"
#endif


class SatModemModule : public GTModule
{
    Q_OBJECT

public:
    explicit SatModemModule(MsgLevel msgLevel = GTObject::NORMAL,QObject *parent = nullptr);
    bool appInit();
    bool appActivate();
    bool configModule();
    bool appResourcesConfig();

private:
    bool installChannels();
    bool installParameters();
    bool installTimers();
    bool installSharedMems();
    bool sendSatModemModule(QString msg);
    bool sendExternal(QString msg);
    bool sendUserControl(QString msg);
    void scheduleMsg(SatModemMsg msg);
    void scheduleExtMsg(SatModemMsg msg);
    void discoverDevices();
    void updateTiming();
    bool localInit();
    void setDeviceConfig(int id, int slot);
    void checkConfig();

private:
    GTModuleChannel *deviceCh;
    GTModuleChannel *moduleCh;
    GTModuleChannel *userControlCh;
    GTModuleTCPServerChannel *externalCh;
    GTNMEADecoder   *moduleDec;
    GTNMEADecoder   *userControlDec;
    QMap< GTNMEADecoder *,int> externalDecMap;
    SatModemDev   *satModem;
    QVector<SatModemMsg> snglCmdQueue, snglReqQueue, bcstCmdQueue, bcstReqQueue, replyQueue;
    QVector<SatModemMsg> extSnglCmdQueue, extSnglReqQueue, extBcstCmdQueue, extBcstReqQueue, extReplyQueue;
    int bcstCmdQueueSize, snglCmdQueueSize, bcstReqQueueSize, snglReqQueueSize;
    int extBcstCmdQueueSize, extSnglCmdQueueSize, extBcstReqQueueSize, extSnglReqQueueSize;
    QVector<SatModemRes::Boa> boaList;
    SatModemRes::Boa localDevice;
    QMutex mtxQu;
    SatModemMsg reply, extReply;
    QVector<int> expectedRepDev, repliedDev;
    QVector<int> expectedExtRepDev, extRepliedDev;
    QByteArray expectedRepID, expectedExtRepID;
    int replyInSlotTime;
    int waitReplyTime;
    int extReplyInSlotTime;
    int extWaitReplyTime;
    int turnaroundTime;
    int discoveryTime;
    int reqIdTime;
    int slotTime;
    bool isDiscoveryOn;
    int slotCounter;

public slots:
    virtual void onMsg(MsgType what, QString who, QString payload);
    void onNMEACmd(QString cmdType, QString code, QList<QVariant> params);

private slots:
    void onModuleMsg(QString code, QStringList params);
    void onNewExtBaseConnection(int id);
    void onNewExtBoaConnection(int id);
    void onExternalBaseMsg(QString msg);
    void onExternalBoaMsg(QString msg);
    void onUserControlMsg(QString code, QStringList params);
    void onNewSatModemCmdMsg(SatModemMsg msg);
    void onNewSatModemReqMsg(SatModemMsg msg);
    void onNewSatModemReplyMsg(SatModemMsg msg);

    void onNewSatModemExtCmdMsg(SatModemMsg msg);
    void onNewSatModemExtReqMsg(SatModemMsg msg);
    void onNewSatModemExtReplyMsg(SatModemMsg msg);

    void onNewSatModemDiscoveryMsg(SatModemMsg msg);
    void onNewSatModemDiscoveryReplyMsg(SatModemMsg msg);
    void onNewSatModemConfigReqMsg(SatModemMsg msg);
    void onNewSatModemConfigRepMsg(SatModemMsg msg);
    void onTransferDone(SatModemMsg transfered);
    void onDeviceReady();
    void onReplyReady();
    void onExtReplyReady();
    void onReqIdTimeout();
    void onDiscoveryTimeout();
    void onSetConfigTimeout();

};


#endif // SATMODEMMODULE_H
