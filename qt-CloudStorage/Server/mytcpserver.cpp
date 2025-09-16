#include "clienttask.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QTcpSocket>

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr handle)
{
    qDebug()<<"新的客户端连接";
    MyTcpSocket *ptcpSocket = new MyTcpSocket;
    ptcpSocket->setSocketDescriptor(handle);
    m_tcpSocketList.append(ptcpSocket);

    connect(ptcpSocket,&MyTcpSocket::offline,this,&MyTcpServer::deleteSocket);//连接移除socket的信号槽

    ClientTask *task = new ClientTask(ptcpSocket);
    m_threadPool.start(task);
}

MyTcpServer::MyTcpServer()
{
    m_threadPool.setMaxThreadCount(16);
}

void MyTcpServer::resend(char *caTarName, PDU *pdu)
{
    if (caTarName == NULL || pdu == NULL) {
            return;
        }
        for (int i = 0; i < m_tcpSocketList.size(); i++) {
            if (caTarName == m_tcpSocketList.at(i)->m_strLoginName) {
                m_tcpSocketList.at(i)->write((char*)pdu, pdu->uiPDULen);
                qDebug() << "send uiMsgType:" << pdu->uiMsgType
                         << "uiPDULen:" << pdu->uiPDULen
                         << "uiMsgLen:" << pdu->uiMsgLen
                         << "caData:" << pdu->caData
                         << "caData+32:" << pdu->caData+32
                         << "caMsg:" << pdu->caMsg;
                break;
            }
        }

}

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
    m_tcpSocketList.removeOne(mysocket);
    mysocket->deleteLater();
    mysocket = NULL;
    //测试是否移除成功
    qDebug()<<m_tcpSocketList.size();
    for(int i=0;i<m_tcpSocketList.size();i++){
        qDebug()<<m_tcpSocketList.at(i)->m_strLoginName;

    }
}
