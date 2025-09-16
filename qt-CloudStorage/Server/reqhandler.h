#ifndef REQHANDLER_H
#define REQHANDLER_H

#include "protocol.h"

#include <QObject>
#include <qfile.h>

class ReqHandler : public QObject
{
    Q_OBJECT
public:
    explicit ReqHandler(QObject *parent = nullptr);
    PDU* pdu;
    QFile m_fUploadFile;
    qint64 m_iUploadFileSize;
    qint64 m_iReceived;
    PDU* handleRegist();
    PDU* handleLogin(QString& loginName);
    PDU* handleFindUser();
    PDU* handleOnlineUser();
    PDU* handleAddFriend();
    PDU* handleAddFriendAgree();
    PDU* handleFlushFriend();
    PDU* handleChat();
    PDU* handleMkdir();
    PDU* handleFlushFile();
    PDU* handleDelDir();
    PDU* handleMvFile();
    PDU* handleUploadInit();
    PDU* handleUploadFileData();
    PDU* handleShareFile();
signals:

};

#endif // REQHANDLER_H
