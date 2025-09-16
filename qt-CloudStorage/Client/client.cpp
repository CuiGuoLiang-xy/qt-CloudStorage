#include "client.h"
#include "ui_client.h"
#include <QFile>
#include <QDebug>
#include <QHostAddress>
#include "index.h"
#include "protocol.h"
#include "uploader.h"

#include <QLineEdit>
#include <QMessageBox>

#include <QString>
Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    m_prh=new ResqHandler;
    ui->setupUi(this);
    loadConfig();
    connect(&my_qtcpSocket, &QTcpSocket::connected, this, &Client::showConnect);



    connect(&my_qtcpSocket, &QTcpSocket::readyRead,this,&Client::recvMsg );



    my_qtcpSocket.connectToHost(QHostAddress(my_strIP),my_strPORT);
}

Client &Client::getInstance()
{
    static Client instance;
    return instance;
}

Client::~Client()
{
    delete ui;
}

void Client::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly)){
       QByteArray data1= file.readAll();
       QString data2=QString(data1);
       QStringList strList =data2.split("\r\n");
       my_strIP = strList.at(0);
       my_strPORT = strList.at(1).toUShort();
       m_strRootDir=strList.at(2);
       file.close();
       qDebug()<<"Client::loadConfig my_strIP"<<my_strIP<<"my_strPORT"<<my_strPORT;
    }else{
       qDebug()<<"配置失败";
    }
}

void Client::sendMsg(PDU *pdu)
{
    my_qtcpSocket.write((char*)pdu, pdu->uiPDULen);
    qDebug() << "sendMsg uiMsgType:" << pdu->uiMsgType
                << "uiPDULen:" << pdu->uiPDULen
                << "uiMsgLen:" << pdu->uiMsgLen
                << "caData:" << pdu->caData
                << "caData+32:" << pdu->caData+32
                << "caMsg:" << pdu->caMsg;

    free(pdu);
    pdu = NULL;
}

QString Client::getRootDir()
{
    return m_strRootDir;
}

void Client::handleMsg(PDU *pdu)
{
    qDebug() << "\n\nrecvMsg uiMsgType:" << pdu->uiMsgType
                 << "uiPDULen:" << pdu->uiPDULen
                 << "uiMsgLen:" << pdu->uiMsgLen
                 << "caData:" << pdu->caData
                 << "caMsg:" << pdu->caMsg;
    m_prh->pdu= pdu;
    switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            bool ret;
            memcpy(&ret,pdu->caData,sizeof(bool));
            if(ret){
                QMessageBox::information(this,"注册","注册成功");
            }else{
                QMessageBox::information(this,"注册","注册失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            bool ret;
            memcpy(&ret,pdu->caData,sizeof(bool));
            if(ret){
                //登陆成功后展示首页并隐藏登录界面
                Index::getInstance().show();
                Index::getInstance().setWindowTitle(m_strLoginName);
                hide();

            }else{
                QMessageBox::information(this,"登录","登录失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_FIND_USER_RESPOND:
        {
            int ret;
            memcpy(&ret,pdu->caData,sizeof(int));
            Friend* f =Index::getInstance().getFriend();
            if(ret == -1){
                QMessageBox::information(f,"提示","查找失败");
            }else if(ret ==0){
                QMessageBox::information(f,"提示","该用户离线");
            }else if (ret == 1) {
                QMessageBox::information(f,"提示", "该用户在线");
            }else if (ret == 2) {
                QMessageBox::information(f,"提示", "该用户不存在");
            }
            break;
        }
            case ENUM_MSG_TYPE_ONLINE_USER_RESPOND:
            {
                uint uiSize =pdu->uiMsgLen/32;
                QStringList nameList;
                char tmp[32];
                for(uint i=0;i<uiSize;i++){
                    memcpy(tmp,pdu->caMsg+i*32,32);
                    if(QString(tmp)==m_strLoginName){
                        continue;
                    }
                    nameList.append(tmp);
                }
                Index::getInstance().getFriend()->m_pOnlineUser->updateListWidget(nameList);
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
            {
                int ret;
                   memcpy(&ret, pdu->caData, sizeof(int));
                   Friend* f = Index::getInstance().getFriend();
                   if (ret == -1) {
                       QMessageBox::information(f, "提示", "添加失败");
                   } else if (ret == 0) {
                       QMessageBox::information(f, "提示", "该用户不在线");
                   } else if (ret == -2) {
                       QMessageBox::information(f, "提示", "对方已经是你的好友");
                   }
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
            {
                char caName[32] = {'\0'};
                memcpy(caName, pdu->caData, 32);
                int ret = QMessageBox::question(Index::getInstance().getFriend(), "添加好友", QString("是否同意 %1 的添加好友请求？").arg(caName));
                qDebug()<<"\n-----------------ret:"<<ret;
                if (ret != QMessageBox::Yes) {
                    return;
                }
                qDebug()<<"mkPDU start";
                PDU* respdu = mkPDU(ENUM_MSG_TYPE_ADD_FRIEND_AGREE_REQUEST, 0);
                memcpy(respdu->caData, pdu->caData, 64);
                Client::getInstance().sendMsg(respdu);
                qDebug()<<"mkPDU end";

            }
            case ENUM_MSG_TYPE_ADD_FRIEND_AGREE_RESPOND:
            {
                bool ret;
                memcpy(&ret, pdu->caData, sizeof(bool));
                if(ret){
                    QMessageBox::information(Index::getInstance().getFriend(), "提示", "添加好友成功");
                    Index::getInstance().getFriend()->flushFriend();
                }else{
                    QMessageBox::information(Index::getInstance().getFriend(), "提示", "添加好友失败");
                }
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
            {
                uint uiSize =pdu->uiMsgLen/32;
                QStringList nameList;
                char tmp[32];
                for(uint i=0;i<uiSize;i++){
                    memcpy(tmp,pdu->caMsg+i*32,32);
                    nameList.append(tmp);
                }
                Index::getInstance().getFriend()->updateListWidget(nameList);
                break;
            }
            case ENUM_MSG_TYPE_CHAT_REQUEST:
            {
                char caChatName[32] = {'\0'};
                memcpy(caChatName,pdu->caData,32);
                Chat* c= Index::getInstance().getFriend()->m_pChat;
                if(c->isHidden()){
                    c->setWindowTitle(caChatName);
                    c->show();
                }
                c->m_StrChatName = caChatName;
                c->update_TE(QString("%1: %2").arg(caChatName).arg(pdu->caMsg));
                break;
            }
            case ENUM_MSG_TYPE_MKDIR_RESPOND:
            {
                bool ret;

                memcpy(&ret,pdu->caData,sizeof(bool));
                qDebug()<<"ret:     "<<ret;
                if(ret){
                    Index::getInstance().getFile()->flushFile();
                }else{
                    QMessageBox::information(&Client::getInstance(),"提示","创建文件夹失败");
                }
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
            {
                int iCount = pdu->uiMsgLen/sizeof(FileInfo);
                    QList<FileInfo*> pFileList;
                    for (int i = 0; i < iCount; i++) {
                        FileInfo* pFileInfo = new FileInfo;
                        memcpy(pFileInfo, pdu->caMsg+i*sizeof (FileInfo), sizeof (FileInfo));
                        pFileList.append(pFileInfo);
                    }
                    Index::getInstance().getFile()->updateFileList(pFileList);
              break;
            }
            case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
            {
                bool ret;

                memcpy(&ret,pdu->caData,sizeof(bool));
                qDebug()<<"ret:     "<<ret;
                if(ret){
                    Index::getInstance().getFile()->flushFile();
                }else{
                    QMessageBox::information(&Client::getInstance(),"提示","删除文件夹失败");
                }
                break;
            }
            case ENUM_MSG_TYPE_MV_FILE_RESPOND:
            {
                bool ret;

                memcpy(&ret,pdu->caData,sizeof(bool));

                if(ret){
                    Index::getInstance().getFile()->flushFile();
                }else{
                    QMessageBox::information(&Client::getInstance(),"提示","移动文件失败");
                }
                break;
            }
            case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
            {
                bool ret;

                memcpy(&ret,pdu->caData,sizeof(bool));

                if(ret){
                    Uploader* uploader =new Uploader(Index::getInstance().getFile()->m_strUploadFilePath);
                    connect(uploader,&Uploader::errorHandle,&Client::getInstance(),&Client::uploadError);
                    connect(uploader,&Uploader::uploadPDU,&Client::getInstance(),&Client::sendMsg);
                    uploader->start();
                }else{
                    QMessageBox::information(&Client::getInstance(),"提示","上传文件失败");
                }
                break;
            }
            case ENUM_MSG_TYPE_UPLOAD_FILE_DATA_RESPOND:
            {
                bool ret;

                memcpy(&ret,pdu->caData,sizeof(bool));

                if(ret){
                    Index::getInstance().getFile()->flushFile();
                }else{
                    QMessageBox::information(&Client::getInstance(),"提示","上传文件失败");
                }
                break;

            }

    default:
        break;
    }
}

void Client::showConnect()
{
    qDebug()<<"连接服务器成功";
}

void Client::recvMsg()
{
//    qDebug() << "recvMsg 接收消息长度：" << my_qtcpSocket.bytesAvailable();
//    uint uiPDULen = 0;
//    my_qtcpSocket.read((char*)&uiPDULen, sizeof(uint)); //读协议总长度

//    uint uiMsgLen = uiPDULen - sizeof(PDU);
//    PDU* pdu = mkPDU(0, uiMsgLen);
//    my_qtcpSocket.read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));//读取除了协议总长度以外剩余的内容
//    pdu->caMsg[pdu->uiMsgLen]='\0';
//    qDebug() << "\n\nrecvMsg uiMsgType:" << pdu->uiMsgType
//             << "uiPDULen:" << pdu->uiPDULen
//             << "uiMsgLen:" << pdu->uiMsgLen
//             << "caData:" << pdu->caData
//             << "caMsg:" << pdu->caMsg;

    qDebug() << "\n\recvMsg 接收消息长度：" << my_qtcpSocket.bytesAvailable();
    QByteArray data = my_qtcpSocket.readAll();
    buffer.append(data);//解决半包：如果不是完整数据，我们把接收到的半包数据先存下来,等待后半部分


    while(buffer.size()>=int(sizeof(PDU))){//判断buffer中的数据够不够一个PDU结构体
        PDU* pdu = (PDU*)buffer.data();
        if(buffer.size()<int(pdu->uiPDULen)){//判断buffer中的数据够不够一个完整包
            break;
        }
        //处理一个完整包数据
        handleMsg(pdu);

        //移除处理完的包数据
        buffer.remove(0,pdu->uiPDULen);
    }


}

void Client::uploadError(const QString &error)
{
    QMessageBox::information(this,"提示",error);
}

void Client::sendPDU(PDU *pdu)
{
    my_qtcpSocket.write((char*)pdu, pdu->uiPDULen);
    qDebug() << "sendMsg uiMsgType:" << pdu->uiMsgType
                << "uiPDULen:" << pdu->uiPDULen
                << "uiMsgLen:" << pdu->uiMsgLen
                << "caData:" << pdu->caData
                << "caData+32:" << pdu->caData+32
                << "caMsg:" << pdu->caMsg;

}










//void Client::on_pushButton_clicked()
//{
//    QString strMsg = ui->lineEdit->text();
//    qDebug() << "strMsg:" << strMsg;
//    PDU* pdu = mkPDU(1, strMsg.toStdString().size());
//    memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.toStdString().size());
//    sendMsg(pdu);
//}


void Client::on_regist_PB_clicked()
{
    QString strName = ui->name_LE->text();
    QString strPwd =ui->pwd_LE->text();

    PDU* pdu =mkPDU(ENUM_MSG_TYPE_REGIST_REQUEST,0);

    memcpy(pdu->caData,strName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
    sendMsg(pdu);
}

void Client::on_login_PB_clicked()
{
    QString strName = ui->name_LE->text();
    QString strPwd =ui->pwd_LE->text();
    m_strLoginName = strName;
    PDU* pdu =mkPDU(ENUM_MSG_TYPE_LOGIN_REQUEST,0);

    memcpy(pdu->caData,strName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
    sendMsg(pdu);
}
