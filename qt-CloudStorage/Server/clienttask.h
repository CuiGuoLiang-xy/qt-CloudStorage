#ifndef CLIENTTASK_H
#define CLIENTTASK_H

#include "mytcpsocket.h"

#include <QObject>
#include <QRunnable>

class ClientTask : public QObject,public QRunnable
{
    Q_OBJECT
public:
    ClientTask();
    ClientTask(MyTcpSocket* socket);
    MyTcpSocket* m_pSocket;
    void run() override;
};

#endif // CLIENTTASK_H
