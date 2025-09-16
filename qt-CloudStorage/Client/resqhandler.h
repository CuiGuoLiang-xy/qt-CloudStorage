#ifndef RESQHANDLER_H
#define RESQHANDLER_H

#include "protocol.h"

#include <QObject>

class ResqHandler : public QObject
{
    Q_OBJECT
public:
    explicit ResqHandler(QObject *parent = nullptr);
    PDU* pdu;
    void handleRegist();
    void handleLogin();
    void handleFindUser();
    void handleOnlineUser();
    void handleAddFriend();
    void handleAddFriendResend();
    void handleAddFriendAgree();
    void handleMvFile();
signals:


};

#endif // RESQHANDLER_H
