#include "client.h"
#include "index.h"
#include "protocol.h"
#include <QApplication>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client::getInstance().show();
//    Client w;
//    w.show();
    return a.exec();
}
