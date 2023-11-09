#include <QDebug>
#include <QDateTime>

#include "SatModemMsg.h"

using namespace SatModemRes;

SatModemMsg::SatModemMsg()
{
    status = MSGSTATUS_UNDEFINED;
    id = QUuid::createUuid().toString().remove(0,1).split('-').at(0).toUtf8();
}


void SatModemMsg::setStatus(SatModemRes::MsgStatus st)
{
    MsgStatus oldStatus = status;
    QString oldStatusTxt = getStatusText(oldStatus);
    status = st;
    //qDebug().noquote() << QDateTime::currentMSecsSinceEpoch() << id << "->" << getStatusText(status);
}


QString SatModemMsg::getStatusText(SatModemRes::MsgStatus st)
{
    QString statusText;
    switch (st) {
    case MSGSTATUS_CREATED:
        statusText = "Created";
        break;
    case MSGSTATUS_SCHEDULED:
        statusText = "Scheduled";
        break;
    case MSGSTATUS_SENT:
        statusText = "Sent";
        break;
    case MSGSTATUS_DELIVERED:
        statusText = "Delivered";
        break;
    case MSGSTATUS_PARTDELIV:
        statusText = "Partially delivered";
        break;
    case MSGSTATUS_NOTDELIVERED:
        statusText = "Not delivered";
        break;
    case MSGSTATUS_REJECTED:
        statusText = "Rejected";
        break;
    case MSGSTATUS_OVERWRITTEN:
        statusText = "Overwritten";
        break;
    case MSGSTATUS_MISSED:
        statusText = "Missed";
        break;
    case MSGSTATUS_EXPECTED_IMMEDIATE:
        statusText = "Expected immediately";
        break;
    case MSGSTATUS_EXPECTED_IN_SLOT:
        statusText = "Expected in slot";
        break;
    case MSGSTATUS_UNDEFINED:
        statusText = "Undefined";
        break;
    case MSGSTATUS_LOADED:
        statusText = "Loaded";
        break;
    case MSGSTATUS_RECEIVED:
        statusText = "Received";
        break;
    }
    return statusText;
}


QString SatModemMsg::getTypeText()
{
    QString typeText;
    switch (type) {
    case MSGTYPE_BCSTCMD:
        typeText = "Broadcast command";
        break;
    case MSGTYPE_BCSTREQ:
        typeText = "Broadcast request";
        break;
    case MSGTYPE_SNGLCMD:
        typeText = "Single command";
        break;
    case MSGTYPE_SNGLREQ:
        typeText = "Single request";
        break;
    case MSGTYPE_REPLY:
        typeText = "Reply";
        break;
    case MSGTYPE_DISCOVERY:
        typeText = "Discovery";
        break;
    case MSGTYPE_DISCOVERY_REP:
        typeText = "Discovery reply";
        break;
    case MSGTYPE_CONFIG_REQ:
        typeText = "Config request";
        break;
    case MSGTYPE_CONFIG_REP:
        typeText = "Config reply";
        break;
    case MSGTYPE_EXT_BCSTCMD:
        typeText = "External broadcast command";
        break;
    case MSGTYPE_EXT_BCSTREQ:
        typeText = "External broadcast request";
        break;
    case MSGTYPE_EXT_SNGLCMD:
        typeText = "External single command";
        break;
    case MSGTYPE_EXT_SNGLREQ:
        typeText = "External broadcast request";
        break;
    case MSGTYPE_EXT_REPLY:
        typeText = "External reply";
        break;
    }
    return typeText;
}


void SatModemMsg::setPayload(QString pld)
{
    payload = pld.toUtf8();
}
