#include "operatedb.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
OperateDB::OperateDB(QObject * parent):QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");

}

OperateDB &OperateDB::getInstance()//单例模式
{
    static OperateDB instance;
    return instance;
}

void OperateDB::connect()//连接数据库
{
    m_db.setHostName("localhost");
    m_db.setPort(3307);
    m_db.setUserName("root");
    m_db.setPassword("");//密码不一定一样
    m_db.setDatabaseName("mydb2401");
    if (m_db.open()) {
           qDebug() << "数据库连接成功";
       } else {
           qDebug() << "数据库连接失败" << m_db.lastError().text();
       }
}

OperateDB::~OperateDB()
{
    m_db.close();
}

bool OperateDB::handleRegist(const char *name, const char *pwd)
{
    if(name==NULL||pwd==NULL){
        return false;
    }
    QString sql = QString("select * from user_info where name ='%1'").arg(name);
    QSqlQuery q;
    if(!q.exec(sql)||q.next()){
        return false;
    }
    //添加用户
    sql = QString("insert into user_info(name,pwd) values('%1','%2')").arg(name).arg(pwd);
    qDebug()<<"添加用户"<<sql;
    return q.exec(sql);
}

bool OperateDB::handleLogin(const char *name, const char *pwd)
{
    if(name==NULL||pwd==NULL){
        return false;
    }
    //查看用户名和密码是否正确
    QString sql = QString("select * from user_info where name ='%1' and pwd = '%2'").arg(name).arg(pwd);
    QSqlQuery q;
    if(!q.exec(sql)||!q.next()){
        return false;
    }
    //将登录成功的用户的online状态改为1
    sql = QString("update user_info set online =1 where name ='%1' and pwd = '%2' ").arg(name).arg(pwd);
    qDebug()<<"将登录成功的用户的online状态改为1"<<sql<<endl;
    return q.exec(sql);
}

void OperateDB::handleOffline(const char *name)
{
    if(name == NULL){
        return ;
    }
    QSqlQuery q;
    QString sql = QString("update user_info set online =0 where name ='%1' ").arg(name);
    qDebug()<<"将用户的online字段置为0"<<endl;
    q.exec(sql);
}

int OperateDB::handleFindUser(const char *name)
{   //-1代表失败，0代表离线，1代表在线，2代表不存在
    if(name==NULL){
        return -1;
    }
    QSqlQuery q;
    QString sql = QString("select online from user_info where name ='%1'").arg(name);
    qDebug()<<"按用户名查找用户："<<sql;
    if(!q.exec(sql)){
        return -1;
    }
    if(q.next()){
        return q.value(0).toInt();
    }
    return 2;
}

int OperateDB::handleAddFriend(const char *curName, const char *tarName)
{
    if(curName==NULL||tarName==NULL){
        return -1;
    }
    QString sql = QString(R"(
                      select * from friend where
                      (
                        user_id=(select id from user_info where name='%1')
                        and
                        friend_id=(select id from user_info where name='%2')
                      )
                      or
                      (
                        user_id=(select id from user_info where name='%2')
                        and
                        friend_id=(select id from user_info where name='%1')
                      )
                      )").arg(curName).arg(tarName);
    QSqlQuery q;
    q.exec(sql);
    if(q.next()){
        return -2;
    }
    sql =QString("select online from user_info where name ='%1'").arg(tarName);
    q.exec(sql);
    if(q.next()){
        return q.value(0).toInt();
    }
    return -1;
}

QStringList OperateDB::handleOnlineUser()
{
    QSqlQuery q;
    QString sql = QString("select name from user_info where online =1");
    qDebug()<<"查找在线用户名"<<sql;
    q.exec(sql);
    QStringList result;
    result.clear();
    while (q.next()) {
        result.append(q.value(0).toString());
    }
    return result;
}

bool OperateDB::handleAddFriendAgree(const char *curName, const char *tarName)
{
    if(curName==NULL||tarName==NULL){
        return -1;
    }
    QString sql = QString(R"(
                            insert into friend(user_id,friend_id)
                          select u1.id,u2.id from user_info u1,user_info u2
                          where u1.name='%1' and u2.name = '%2'
                          )").arg(curName).arg(tarName);
    qDebug()<<"插入好友"<<sql;
    QSqlQuery q;
    return q.exec(sql);
}

QStringList OperateDB::handleFlushFriend(const char *name)
{
    QSqlQuery q;
    QString sql = QString(R"(
              select name from user_info where online=1 and id in
              (
                select friend_id from friend where user_id =(select id from user_info where name = '%1')
                union
                select user_id from friend where friend_id =(select id from user_info where name = '%1')
              )
                              )").arg(name);

    qDebug()<<"查找好友"<<sql;
    q.exec(sql);
    QStringList result;
    result.clear();
    while (q.next()) {
        result.append(q.value(0).toString());
    }
    return result;
}
