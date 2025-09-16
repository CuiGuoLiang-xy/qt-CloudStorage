#include "server.h"
#include <QApplication>
#include <QApplication>
#include "operatedb.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OperateDB::getInstance().connect();
    Server::getInstance();
    /*Server w;
//    w.show()*/;
    return a.exec();
}
