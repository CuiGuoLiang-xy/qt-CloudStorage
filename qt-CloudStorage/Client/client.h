#ifndef CLIENT_H
#define CLIENT_H

#include "protocol.h"
#include "resqhandler.h"

#include <QWidget>
#include <QTcpSocket>
#include <QLineEdit>
QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QWidget
{
    Q_OBJECT

public:
    static Client &getInstance();
    ~Client();
    void loadConfig();

    QString m_strLoginName;
    QString getRootDir();
    QByteArray buffer;
    void handleMsg(PDU* pdu);
    ResqHandler* m_prh;
    QTcpSocket my_qtcpSocket;
public slots:
    void showConnect();
    void recvMsg();
    void uploadError(const QString& error);
    void sendPDU(PDU* pdu);
    void sendMsg(PDU *pdu);
private slots:
    void on_regist_PB_clicked();
    void on_login_PB_clicked();
private:
    Ui::Client *ui;
    QString my_strIP;
    quint16 my_strPORT;
    QString m_strRootDir;


    Client(QWidget *parent = nullptr);
    Client(const Client& instance) = delete;
    Client& operator = (const Client&) = delete;
};
#endif // CLIENT_H
