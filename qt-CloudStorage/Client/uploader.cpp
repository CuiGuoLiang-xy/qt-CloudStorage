
#include "uploader.h"

#include <QFile>
#include <QDebug>
#include <QThread>
Uploader::Uploader(QObject *parent) : QObject(parent)
{

}

Uploader::Uploader(const QString &filepath):m_strUploadFilePath(filepath)
{

}

void Uploader::uploadFile()
{
    //打开要上传的文件
        QFile file(m_strUploadFilePath);
        if (!file.open(QIODevice::ReadOnly)) {
            emit errorHandle("打开文件失败");
            emit finished();
            return;
        }




        //循环发送
        while (true) {
            //构建pdu，每次发送4096
            PDU* pdu = mkPDU(ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST, 4096);
            int ret = file.read(pdu->caMsg, 4096);
            if (ret == 0) {
                break;
            }
            if (ret < 0) {
                emit errorHandle( "读取文件失败");
                break;;
            }
            //更新长度并发送
            pdu->uiMsgLen = ret;
            pdu->uiPDULen = ret + sizeof(PDU);
            emit uploadPDU(pdu);
//            Client::getInstance().my_qtcpSocket.write((char*)pdu, pdu->uiPDULen);
//            qDebug() << "sendMsg uiMsgType:" << pdu->uiMsgType
//                     << "uiPDULen:" << pdu->uiPDULen
//                     << "uiMsgLen:" << pdu->uiMsgLen
//                     << "caData:" << pdu->caData
//                     << "caData+32:" << pdu->caData+32
//                     << "caMsg:" << pdu->caMsg;
        }
        file.close();

        emit finished();
}

void Uploader::start()
{
    //创建线程对象，将上传类对象移入线程对象
    QThread* thread = new QThread;
    this->moveToThread(thread);

    //将上传函数的开始与结束和线程进行关联
    connect(thread,&QThread::started,this,&Uploader::uploadFile);
    connect(this,&Uploader::finished,thread,&QThread::quit);

    //当线程完成时释放掉
    connect(thread,&QThread::finished,thread,&QThread::deleteLater);
    //线程开始执行
    thread->start();
}
