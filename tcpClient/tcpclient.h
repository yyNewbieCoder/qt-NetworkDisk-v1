#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include "opewidget.h"

//前向声明：意味着在UI命名空间中有一个TcpClient类，稍后会完整定义，现在先让我使用它（Ui::TcpClient *ui;）完成编译
QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE
//UI::TcpClient专门用来做界面设计，而TcpClient专门用来做业务逻辑。两部分操作分离开。

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    static TcpClient &getInstance();
    QTcpSocket &getTcpSocket();
    QString getLoginName();
    QString getCurPath();
    void setSaveFilePath(QString path);

public slots:
    void showConnect();
    void recvMsg();

private slots:
    // void on_send_pb_clicked();

    void on_login_pb_clicked();

    void on_register_pb_clicked();

    void on_logout_pb_clicked();

private:
    Ui::TcpClient *ui;      //设计师界面类，自动生成
    QString m_strIP;        //服务器IP
    quint16 m_usPort;       //服务器端口
    QTcpSocket m_tcpSocket; //套接字
    QString m_strLoginName; //登录的用户名
    QString m_strCurPath;   //记录当前路径

    QFile m_file;       //要下载的文件
    qint64 m_iTotal;    //文件总字节数
    qint64 m_iRecved;   //当前收到的字节数
    bool m_bDownload;
    QString m_strSaveFilePath;  //要下载的文件路径
};
#endif // TCPCLIENT_H
