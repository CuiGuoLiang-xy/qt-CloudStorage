#include "client.h"
#include "file.h"
#include "sharefile.h"
#include "ui_file.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <ui_file.h>

File::File(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::File)
{
    ui->setupUi(this);
    m_strUserPath = QString("%1/%2")
                            .arg(Client::getInstance().getRootDir())
                            .arg(Client::getInstance().m_strLoginName);
    m_strCurPath = m_strUserPath;
    m_pShareFile = new ShareFile;
    qDebug()<<"m_strCurPath"<<m_strCurPath;
    flushFile();
}

File::~File()
{
    delete ui;
    delete m_pShareFile;
}

void File::flushFile()
{
    PDU* pdu = mkPDU(ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,m_strCurPath.toStdString().size()+1);
    memcpy(pdu->caMsg,m_strCurPath.toStdString().c_str(),m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::updateFileList(QList<FileInfo *> pFileList)
{
    //每次更新前释放之前的文件信息
        foreach(FileInfo* pFileInfo, m_pFileInfoList) {
            delete pFileInfo;
        }
        m_pFileInfoList = pFileList;

        ui->listWidget->clear();
        //遍历文件信息，每个文件构建一个QListWidgetItem，放入QListWidget中
        foreach(FileInfo* pFileInfo, pFileList) {
            QListWidgetItem* pItem = new QListWidgetItem;
            pItem->setText(pFileInfo->caName);
            if (pFileInfo->iFileType == 0) {
                pItem->setIcon(QIcon(QPixmap(":/dir.png")));
            } else if  (pFileInfo->iFileType == 1) {
                pItem->setIcon(QIcon(QPixmap(":/file.png")));
            }
            ui->listWidget->addItem(pItem);
        }
}



void File::on_mkdir_PB_clicked()
{
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","新建文件夹名");
    if(strNewDir.isEmpty()||strNewDir.toStdString().size()>32){
        QMessageBox::information(this,"提示","文件名非法");
        return;
    }
    PDU* pdu = mkPDU(ENUM_MSG_TYPE_MKDIR_REQUEST,m_strCurPath.toStdString().size()+1);
    memcpy(pdu->caData,strNewDir.toStdString().c_str(),32);
    memcpy(pdu->caMsg,m_strCurPath.toStdString().c_str(),m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::on_flushFile_PB_clicked()
{
    flushFile();
}

void File::on_rmdir_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
       if (pItem == NULL) {
           QMessageBox::information(this, "提示", "请选择要删除的文件夹");
           return;
       }
       QString strFileName = pItem->text();
       foreach(FileInfo* pFileInfo, m_pFileInfoList) {
           if (strFileName == pFileInfo->caName && pFileInfo->iFileType != 0) {
               QMessageBox::information(this, "提示", "选择的不是文件夹");
               return;
           }
       }
       QString strPath = QString("%1/%2").arg(m_strCurPath).arg(strFileName);
       PDU* pdu = mkPDU(ENUM_MSG_TYPE_DEL_DIR_REQUEST, strPath.toStdString().size());
       memcpy(pdu->caMsg, strPath.toStdString().c_str(), strPath.toStdString().size());
       Client::getInstance().sendMsg(pdu);

}

void File::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString strFileName = item->text();
    foreach(FileInfo* pFileInfo, m_pFileInfoList) {
        if (strFileName == pFileInfo->caName && pFileInfo->iFileType != 0) {
            return;
        }
    }
    m_strCurPath = QString("%1/%2").arg(m_strCurPath).arg(strFileName);
    flushFile();
}

void File::on_return_PB_clicked()
{
    if(m_strCurPath == m_strUserPath ){
        return;
    }
    int index = m_strCurPath.lastIndexOf('/');
    m_strCurPath.remove(index,m_strCurPath.size() - index);
    flushFile();
}
bool File::isDir(QString strFileName)
{
    foreach(FileInfo* pFileInfo, m_pFileInfoList) {
            if (strFileName == pFileInfo->caName) {
                return pFileInfo->iFileType == 0;
            }
        }
        return false;

}

void File::uploadFile()
{
    //打开要上传的文件
        QFile file(m_strUploadFilePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "提示", "打开文件失败");
            return;
        }

        //构建pdu，每次发送4096
        PDU* pdu = mkPDU(ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST, 4096);


        //循环发送
        while (true) {
            int ret = file.read(pdu->caMsg, 4096);
            if (ret == 0) {
                break;
            }
            if (ret < 0) {
                QMessageBox::warning(this, "提示", "读取文件失败");
                break;;
            }
            //更新长度并发送
            pdu->uiMsgLen = ret;
            pdu->uiPDULen = ret + sizeof(PDU);
            Client::getInstance().my_qtcpSocket.write((char*)pdu, pdu->uiPDULen);
            qDebug() << "sendMsg uiMsgType:" << pdu->uiMsgType
                     << "uiPDULen:" << pdu->uiPDULen
                     << "uiMsgLen:" << pdu->uiMsgLen
                     << "caData:" << pdu->caData
                     << "caData+32:" << pdu->caData+32
                     << "caMsg:" << pdu->caMsg;
        }
        file.close();
        free(pdu);
        pdu = NULL;

}
void File::on_mv_PB_clicked()
{
    if (ui->mv_PB->text() == "移动文件") {
            QListWidgetItem* pItem = ui->listWidget->currentItem();
            if (pItem == NULL) {
                QMessageBox::information(this, "提示", "请选择要移动的文件夹");
                return;
            }
            //提示用户选择要移动到的目录
            QMessageBox::information(this, "提示", "请选择要移动到的目录");
            //按钮文字改成 确认/取消，在用户选择要移动到的目录后，让用户再次点击该按钮确认移动
            ui->mv_PB->setText("确认/取消");
            //记录要移动的文件名和完整路径，第二次点击时发送给服务器需要用这两个变量
            m_strMoveFileName = pItem->text();
            m_strMoveFilePath = m_strCurPath + '/' + m_strMoveFileName;
            return;
        }
        ui->mv_PB->setText("移动文件");
        QListWidgetItem* pItem = ui->listWidget->currentItem();
        if (pItem == NULL) {
            QMessageBox::information(this, "提示", "请选择要移动的文件夹");
            return;
        }
        if (!isDir(pItem->text())) {
            QMessageBox::information(this, "提示", "选择的文件不是目录");
            return;
        }
        int ret = QMessageBox::information(this, "提示", "是否确认移动到该目录下", "确认", "取消");
        if (ret != 0) {
            return;
        }
        //拼接目标路径：移动后文件的完整路径
        QString strTarPath = m_strCurPath + '/' + pItem->text() + '/' + m_strMoveFileName;
        //构建pdu，原路径和新路径放入caMsg，他们的长度放入caData
        int srcLen = m_strMoveFilePath.toStdString().size();
        int tarLen = strTarPath.toStdString().size();
        PDU* pdu = mkPDU(ENUM_MSG_TYPE_MV_FILE_REQUEST, srcLen+tarLen+1);
        memcpy(pdu->caData, &srcLen, sizeof(int));
        memcpy(pdu->caData+32, &tarLen, sizeof(int));
        memcpy(pdu->caMsg, m_strMoveFilePath.toStdString().c_str(), srcLen);
        memcpy(pdu->caMsg+srcLen, strTarPath.toStdString().c_str(), tarLen);
        Client::getInstance().sendMsg(pdu);

}

void File::on_upload_PB_clicked()
{
    //通过文件选择弹窗获取用户选择要上传的文件路径
    m_strUploadFilePath.clear();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if(m_strUploadFilePath.isEmpty()){
        return;
    }
    qDebug()<<"m_strUploadFilePath"<<m_strUploadFilePath;

    //从文件路径取出文件名
    int index =  m_strUploadFilePath.lastIndexOf('/');
    QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
    qDebug()<<"strFileName"<<strFileName;

    //通过QFile对象获取文件大小
    QFile file(m_strUploadFilePath);
    qint64 iFileSize = file.size();
    qDebug()<<"iFileSize "<<iFileSize;
    //发送文件名、文件大小、当前路径
    PDU* pdu =mkPDU(ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,m_strCurPath.toStdString().size()+1);
    memcpy(pdu->caData,strFileName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,&iFileSize,sizeof(qint64));
    memcpy(pdu->caMsg,m_strCurPath.toStdString().c_str(),m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::on_share_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
        if (pItem == NULL) {
            QMessageBox::information(this, "提示", "请选择要分享的文件");
            return;
        }
        m_strShareFilePath = QString("%1/%2").arg(m_strCurPath).arg(pItem->text());
        m_pShareFile->updateFriend_LW();
        if (m_pShareFile->isHidden()) {
            m_pShareFile->show();
        }

}
