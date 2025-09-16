#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QMainWindow
{
    Q_OBJECT

public:
    static Server& getInstance();
    ~Server();
    void loadConfig();
    QString getRootDir();
private:
    Server(QWidget *parent = nullptr);
    Server(const Server& instance)=delete;
    Server& operator=(const Server&)=delete;

    QString my_strIP;
    quint16 my_strPORT;
    QTcpSocket t_tcpsocket;
    QString m_strRootDir;
    void showConnect();

    Ui::Server *ui;
};
#endif // SERVER_H
