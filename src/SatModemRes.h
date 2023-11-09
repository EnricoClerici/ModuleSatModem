#ifndef SATMODEMRES_H
#define SATMODEMRES_H

#include <QString>
#include <QVector>

namespace SatModemRes {


#define WAIT_FOR_REPLY  0.3
#define DEFAULT_ID      "n/A"

    enum DefParams {
        DEFAULT_SLOT      = 0,
        DEFAULT_SLOT_TIME = 100,
        DEFAULT_QU_SIZE   = 1,
        MASTER_DEVICEID   = 0,
    };

    enum TimeParams {
        DISCOVERY_TIME    = 2000,
        WAIT_ID_TIME      = 3000,
        READY_CHECK_TIME  = 5
    };

    enum MsgProp {
        PCK_START       = '?',
        PCK_END         = '!',
        BCST_ID         = 99,
        MAX_RECV_BUFF   = 1024,
        MIN_PCK_SIZE    = 15,
        INFO_SIZE       = 2,
        CONFIG_SIZE     = 2,
        EXT_MSG_SIZE    = 3
    };

    enum MsgType {
        MSGTYPE_SNGLREQ       = 'A',
        MSGTYPE_BCSTREQ       = 'B',
        MSGTYPE_SNGLCMD       = 'C',
        MSGTYPE_BCSTCMD       = 'D',
        MSGTYPE_REPLY         = 'E',
        MSGTYPE_DISCOVERY     = 'F',
        MSGTYPE_DISCOVERY_REP = 'G',
        MSGTYPE_CONFIG_REQ    = 'H',
        MSGTYPE_CONFIG_REP    = 'I',
        MSGTYPE_EXT_SNGLREQ   = 'J',
        MSGTYPE_EXT_BCSTREQ   = 'K',
        MSGTYPE_EXT_SNGLCMD   = 'L',
        MSGTYPE_EXT_BCSTCMD   = 'M',
        MSGTYPE_EXT_REPLY     = 'N'
    };

    enum MsgStatus {
        MSGSTATUS_CREATED               = 1,
        MSGSTATUS_SCHEDULED             = 2,
        MSGSTATUS_SENT                  = 3,
        MSGSTATUS_DELIVERED             = 4,
        MSGSTATUS_PARTDELIV             = 5,
        MSGSTATUS_NOTDELIVERED          = 6,
        MSGSTATUS_REJECTED              = 7,
        MSGSTATUS_OVERWRITTEN           = 8,
        MSGSTATUS_MISSED                = 9,
        MSGSTATUS_EXPECTED_IMMEDIATE    = 10,
        MSGSTATUS_EXPECTED_IN_SLOT      = 11,
        MSGSTATUS_UNDEFINED             = 12,
        MSGSTATUS_LOADED                = 13,
        MSGSTATUS_RECEIVED              = 14,
    };

    enum MsgConfig {
        MSGCONFIG_SETID         = 1,
        MSGCONFIG_SETSLOT       = 2,
        MSGCONFIG_SETSLOTTIME   = 3,
        MSGCONFIG_GETCONFIG     = 4
    };

    struct Boa {
        int deviceID;
        int slot;
        int slotTime;
        QString id;
        QString ip;
        bool isReady;
    };
}

#endif // SATMODEMRES_H
