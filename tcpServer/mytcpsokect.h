#ifndef MYTCPSOKECT_H
#define MYTCPSOKECT_H

#include <QTcpSocket>
#include <QFile>
#include <QTimer>

class MyTcpSokect : public QTcpSocket
{
       Q_OBJECT
public:
    explicit MyTcpSokect(QObject *parent = nullptr);
    QString getName();
    void copyDir(QString strSrcDir, QString strDestDir);

signals:    //自定义信号
    void offline(MyTcpSokect *mysocket);

public slots:
    void recvMsg();
    void clientOffline();   //客户端下线
    void sendFileToClient();

private:
    QString m_strName;
    QFile m_file;
    qint64 m_iTotal;    //文件总大小
    qint64 m_iRecved;   //文件已经增加了多少
    bool m_bUpload;     //当前是否处于上传文件的状态

    QTimer *m_pTimer;
};

#endif // MYTCPSOKECT_H
