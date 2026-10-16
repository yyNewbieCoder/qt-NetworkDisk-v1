#include "mytcpserver.h"
#include <QDebug>


MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer{parent}
{}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

//Qt 框架在检测到新连接时自动调用 incomingConnection
void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected";
    MyTcpSokect *pTcpSokect = new MyTcpSokect;
    pTcpSokect->setSocketDescriptor(socketDescriptor);
    m_tcpSokectList.append(pTcpSokect);

    connect(pTcpSokect, SIGNAL(offline(MyTcpSokect*)),
            this, SLOT(deleteSocket(MyTcpSokect*)));    //客户端断开连接时，与之对应的服务端套接字也应该被销毁，释放资源
}

void MyTcpServer::resend(const char *pername, PDU *pdu)
{
    if (NULL == pername || NULL == pdu)
    {
        return;
    }

    QString strName = pername;
    for (int i = 0; i < m_tcpSokectList.size(); i++)
    {
        if (strName == m_tcpSokectList.at(i)->getName())
        {
            m_tcpSokectList.at(i)->write((char*)pdu, pdu->uiPDULen);
            break;
        }
    }
}

void MyTcpServer::resendAddFriendResult(const char *applicantName, bool result)
{
    if (NULL == applicantName)
    {
        return;
    }

    QString strName = applicantName;
    for (int i = 0; i < m_tcpSokectList.size(); i++)
    {
        if (strName == m_tcpSokectList.at(i)->getName())
        {
            PDU *respdu = mkPDU(0);
            if (true == result)
            {
                strcpy(respdu->caData, ADD_FRIEND_SUCCESS);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
            }
            else
            {
                strcpy(respdu->caData, ADD_FRIEND_FAILED);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            }
            m_tcpSokectList.at(i)->write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
    }
}

//在m_tcpSokectList列表中进行匹配，匹配到了就删除掉此服务端套接字
void MyTcpServer::deleteSocket(MyTcpSokect *mysocket)
{
    QList<MyTcpSokect*>::iterator iter = m_tcpSokectList.begin();
    for (; iter != m_tcpSokectList.end(); iter++)
    {
        if (mysocket == *iter)
        {
            //delete *iter;                   //1.立即释放空间 此种方式释放空间，会导致服务器崩溃，涉及到 Qt 对象生命周期管理和事件循环机制的核心区别
            (*iter) -> deleteLater();    //1.延时释放空间 完整处理完所有事件，不会崩
            *iter = NULL;
            m_tcpSokectList.erase(iter); //2.移除迭代器 iter 指向的元素，维护列表结构
            break;
        }
    }

    for (int i = 0; i < m_tcpSokectList.size(); i++)
    {
        qDebug() << m_tcpSokectList.at(i)->getName();   //at函数: 返回列表中指定索引位置的元素的常量引用
    }
}
