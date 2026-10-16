#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include <mytcpsokect.h>
#include "protocol.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT    // 需要这个宏来启用信号槽机制
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);//QTcpServer 类的一个保护虚函数，作用：只要有新的客户端连接，服务端就会调用一次此函数，每个连接获得唯一的 socketDescriptor
    void resend(const char *pername, PDU *pdu);
    void resendAddFriendResult(const char *applicantName, bool result);

public slots:
    void deleteSocket(MyTcpSokect *mysocket);

private:
    QList<MyTcpSokect*> m_tcpSokectList;
};

#endif // MYTCPSERVER_H
