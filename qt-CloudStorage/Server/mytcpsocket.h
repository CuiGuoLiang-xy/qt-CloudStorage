#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include "protocol.h"
#include "reqhandler.h"

#include <QObject>
#include <QTcpSocket>
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    ~MyTcpSocket();
    void recvMsg();
    void sendMsg(PDU* pdu);
    QString m_strLoginName;
    ReqHandler* m_prh;
    PDU* readMsg();
    PDU* handleMsg(PDU* pdu);
    QByteArray buffer;
public slots:
    void clientOffline();
signals:
    void offline(MyTcpSocket* mysocket);
};

#endif // MYTCPSOCKET_H
