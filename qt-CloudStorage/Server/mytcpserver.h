#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include "mytcpsocket.h"
#include <QThreadPool>
class MyTcpServer: public QTcpServer
{
    Q_OBJECT
public:
    static MyTcpServer& getInstance();
    void incomingConnection(qintptr handle);
    void resend(char *caTarName, PDU *pdu);
    QThreadPool m_threadPool;
private:
     MyTcpServer();
     MyTcpServer(const MyTcpServer& instance)=delete;
     MyTcpServer& operator=(const MyTcpServer&)=delete;
     //客户端socket指针链表
      QList<MyTcpSocket*> m_tcpSocketList;

public slots:
      void deleteSocket(MyTcpSocket* mysocket);
};

#endif // MYTCPSERVER_H
