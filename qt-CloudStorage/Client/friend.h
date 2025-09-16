#ifndef FRIEND_H
#define FRIEND_H

#include "onlineuser.h"

#include <QWidget>
#include "chat.h"
namespace Ui {
class Friend;
}

class Friend : public QWidget
{
    Q_OBJECT

public:
    explicit Friend(QWidget *parent = nullptr);
    ~Friend();
    OnlineUser* m_pOnlineUser;
    void updateListWidget(QStringList nameList);
    void flushFriend();
    Chat* m_pChat;
    QListWidget* getFriend_LW();
private slots:
    void on_findUser_PB_clicked();

    void on_onlineUser_PB_clicked();

    void on_flush_PB_clicked();

    void on_chat_PB_clicked();

private:
    Ui::Friend *ui;
};

#endif // FRIEND_H
