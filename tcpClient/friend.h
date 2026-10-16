#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>    //信息的显示
#include <QListWidget>  //已添加的好友列表
#include <QLineEdit>    //信息的输入
#include <QPushButton>  //按钮
#include <QVBoxLayout>  //垂直布局
#include <QHBoxLayout>  //水平布局
#include "online.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUsr(PDU *pdu);
    void updateFriendList(PDU *pdu);
    void showPrivateChatText(PDU *pdu);
    void showGroupChatText(PDU *pdu);
    QString m_strSearchName;

    QListWidget *getFriendList();

signals:

public slots:
    void showOnline();
    void searchUsr();
    void flushFriend();
    void delFriend();
    void privateChat();
    void groupChat();

private:
    QTextEdit *m_pShowMsgTE;    //显示信息
    QListWidget *m_pFriendListWidget;//好友列表
    QLineEdit *m_pInputMsgLE;   //信息输入框

    QPushButton *m_pDelFriendPB;    //删除好友按钮
    QPushButton *m_pFlushFriendPB;  //刷新好友按钮
    QPushButton *m_pShowOnlineUsrPB;    //查看在线用户
    QPushButton *m_pSearchUsrPB;    //查找用户
    QPushButton *m_pMsgSendPB;      //发送信息
    QPushButton *m_pPrivateChatPB;  //私聊

    Online *m_pOnline;
};

#endif // FRIEND_H
