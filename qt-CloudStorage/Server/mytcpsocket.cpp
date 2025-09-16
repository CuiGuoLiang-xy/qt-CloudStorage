#include "mytcpsocket.h"
#include "operatedb.h"
#include "protocol.h"

MyTcpSocket::MyTcpSocket()
{
//    connect(this, &QTcpSocket::readyRead, this, &MyTcpSocket::recvMsg);
//    connect(this,&QTcpSocket::disconnected,this,&MyTcpSocket::clientOffline);
    m_prh = new ReqHandler;
}

MyTcpSocket::~MyTcpSocket()
{
    delete m_prh;
}
PDU *MyTcpSocket::readMsg()
{
    qDebug() << "\n\nrecvMsg 接收消息长度：" << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint)); //读协议总长度

    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU* pdu = mkPDU(0, uiMsgLen);
    this->read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));//读取除了协议总长度以外剩余的内容
    qDebug() << "recvMsg uiMsgType:" << pdu->uiMsgType
             << "uiPDULen:" << pdu->uiPDULen
             << "uiMsgLen:" << pdu->uiMsgLen
             << "caData:" << pdu->caData
             << "caMsg:" << pdu->caMsg;
    m_prh->pdu = pdu;
    return pdu;
}

PDU *MyTcpSocket::handleMsg(PDU* pdu)
{
    qDebug() << "handleMsg uiMsgType:" << pdu->uiMsgType
             << "uiPDULen:" << pdu->uiPDULen
             << "uiMsgLen:" << pdu->uiMsgLen
             << "caData:" << pdu->caData
             << "caMsg:" << pdu->caMsg;
    m_prh->pdu = pdu;
    switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
            return m_prh->handleRegist();
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
            return m_prh->handleLogin(m_strLoginName);
        case ENUM_MSG_TYPE_FIND_USER_REQUEST:
            return m_prh->handleFindUser();
        case ENUM_MSG_TYPE_ONLINE_USER_REQUEST:
            return m_prh->handleOnlineUser();
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
            return m_prh->handleAddFriend();
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE_REQUEST:
            return m_prh->handleAddFriendAgree();
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
            return m_prh->handleFlushFriend();
        case ENUM_MSG_TYPE_CHAT_REQUEST:
            return m_prh->handleChat();
        case ENUM_MSG_TYPE_MKDIR_REQUEST:
            return m_prh->handleMkdir();
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
            return m_prh->handleFlushFile();
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
            return m_prh->handleDelDir();
        case ENUM_MSG_TYPE_MV_FILE_REQUEST:
            return m_prh->handleMvFile();
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
            return m_prh->handleUploadInit();
        case ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST:
            return m_prh->handleUploadFileData();
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
            return m_prh->handleShareFile();
    default:
        return NULL;
    }
}
void MyTcpSocket::recvMsg()
{
    qDebug() << "\n\recvMsg 接收消息长度：" << this->bytesAvailable();
    QByteArray data = this->readAll();
    buffer.append(data);//解决半包：如果不是完整数据，我们把接收到的半包数据先存下来,等待后半部分


    while(buffer.size()>=int(sizeof(PDU))){//判断buffer中的数据够不够一个PDU结构体
        PDU* pdu = (PDU*)buffer.data();
        if(buffer.size()<int(pdu->uiPDULen)){//判断buffer中的数据够不够一个完整包
            break;
        }
        //处理一个完整包数据
        PDU* respdu=handleMsg(pdu);
        sendMsg(respdu);
        //移除处理完的包数据
        buffer.remove(0,pdu->uiPDULen);
    }


}

void MyTcpSocket::sendMsg(PDU *pdu)
{
    if(pdu==NULL){
        return ;
    }
    write((char*)pdu, pdu->uiPDULen);
    qDebug() << "send uiMsgType:" << pdu->uiMsgType
                << "uiPDULen:" << pdu->uiPDULen
                << "uiMsgLen:" << pdu->uiMsgLen
                << "caData:" << pdu->caData
                << "caMsg:" << pdu->caMsg;

    free(pdu);
    pdu = NULL;
}

void MyTcpSocket::clientOffline()
{
    OperateDB::getInstance().handleOffline(m_strLoginName.toStdString().c_str());
    emit offline(this);
}



