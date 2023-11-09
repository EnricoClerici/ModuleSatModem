#ifndef SATMODEMDECODER_H
#define SATMODEMDECODER_H

#include <QTimer>

#include "GTModuleChannelDecoders.h"
#include "SatModemRes.h"


class SatModemDec : public GTPacketDecoder
{
    Q_OBJECT

public:
    SatModemDec(QString name = "SATMODEM", bool checkCRC=false);

protected:
    virtual bool doDecode(QByteArray packet);

private:
    void syntaxError(QString error);
    void checkMsg(QByteArray msg);

    QByteArray data;
    bool isStartDetected;
    QTimer *receptionTimeOut;

private slots:
    void onReceptionTimeout();

protected:
    bool m_checkCRC;

signals:
    void newMsg(QByteArray msg);

public:

    SatModemDec(const SatModemDec& other):GTPacketDecoder()
    {
        Q_UNUSED(other);
        assert(false && "DONT EVER USE THIS");
    }

};

#endif // SATMODEMDECODER_H
