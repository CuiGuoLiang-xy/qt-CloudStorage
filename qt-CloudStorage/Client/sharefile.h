#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>

namespace Ui {
class ShareFile;
}

class ShareFile : public QWidget
{
    Q_OBJECT

public:
    explicit ShareFile(QWidget *parent = nullptr);
    ~ShareFile();
    void updateFriend_LW();
private slots:
    void on_allSelect_PB_clicked();

    void on_cancelSelected_PB_clicked();

    void on_ok_clicked();

private:
    Ui::ShareFile *ui;
};

#endif // SHAREFILE_H
