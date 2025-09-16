#include "server.h"
#include "ui_server.h"
#include "mytcpserver.h"
#include <QFile>
Server::Server(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Server)
{
    ui->setupUi(this);
    //加载配置
    loadConfig();
    //监听
    MyTcpServer::getInstance().listen(QHostAddress(my_strIP),my_strPORT);

}
void Server::showConnect()
{
    qDebug()<<"连接服务器成功";
}
void Server::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly)){
       QByteArray data1= file.readAll();
       QString data2=QString(data1);
       QStringList strList =data2.split("\r\n");
       my_strIP = strList.at(0);
       my_strPORT = strList.at(1).toUShort();
       m_strRootDir=strList.at(2);
       file.close();
       qDebug()
               <<"loadConfig my_strIP"<<my_strIP
               <<"my_strPORT"<<my_strPORT
                <<"m_strRootDir"<<m_strRootDir;
    }else{
       qDebug()<<"配置失败";
    }
}

QString Server::getRootDir()
{
    return m_strRootDir;
}
Server& Server::getInstance(){
    static Server instance;
    return instance;
}
Server::~Server()
{
    delete ui;
}

