#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include "privatechat.h"
#include <QInputDialog>
#include <QDebug>
#include <QMessageBox>

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit;
    m_pFriendListWidget = new QListWidget;
    m_pInputMsgLE = new QLineEdit;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户");
    m_pMsgSendPB = new QPushButton("群发");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    //将按钮添加到垂直布局中
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;
    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();
    setLayout(pMain);
    //qDebug() << "Online widget visible:" << m_pOnline->isVisible();
    //qDebug() << "Online widget size:" << m_pOnline->size();   //m_pOnline窗口太大，导致无法弹出
    connect(m_pShowOnlineUsrPB, SIGNAL(clicked(bool)), this, SLOT(showOnline()));   //手动关联槽函数
    connect(m_pSearchUsrPB, SIGNAL(clicked(bool)), this, SLOT(searchUsr()));
    connect(m_pFlushFriendPB, SIGNAL(clicked(bool)), this, SLOT(flushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked(bool)), this, SLOT(delFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked(bool)), this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked(bool)), this, SLOT(groupChat()));
}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if (NULL == pdu)
    {
        return;
    }

    m_pOnline->showUsr(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if (NULL == pdu)
    {
        return;
    }

    m_pFriendListWidget->clear();   //先清除之前的内容

    uint uiSize = pdu->uiMsgLen/NAME_MAX_LEN;
    char caName[32] = {0};
    for (uint i=0; i<uiSize; i++)
    {
        memcpy(caName, (char*)(pdu->caMSG)+i*NAME_MAX_LEN, NAME_MAX_LEN);
        m_pFriendListWidget->addItem(caName);
    }
}

void Friend::showPrivateChatText(PDU *pdu)
{
    PrivateChat::getInstance().updateMsg(pdu);
}

void Friend::showGroupChatText(PDU *pdu)
{
    QString strMsg = QString("%1 says: %2").arg(pdu->caData).arg((char*)(pdu->caMSG));
    m_pShowMsgTE->append(strMsg);
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()
{
    if (m_pOnline->isHidden())
    {
        m_pOnline->show();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名:");   //弹出输入框，输用户名进行查找。“搜索”是标头，“用户名”是标签
    if (!m_strSearchName.isEmpty())
    {
        qDebug() << m_strSearchName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        memcpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::flushFriend()
{
    QString strName = TcpClient::getInstance().getLoginName();
    PDU * pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, strName.toStdString().c_str(), strName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::delFriend()
{
    if (NULL != m_pFriendListWidget->currentItem()) //如果没有选中，就去调用text，程序会崩溃
    {
        //获取到“选中”的好友
        QString strFriendName = m_pFriendListWidget->currentItem()->text();
        QString strMyName = TcpClient::getInstance().getLoginName();
        //qDebug() << strFriendName << strMyName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;

        memcpy(pdu->caData, strMyName.toStdString().c_str(), NAME_MAX_LEN);
        memcpy(pdu->caData+NAME_MAX_LEN, strFriendName.toStdString().c_str(), NAME_MAX_LEN);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::privateChat()
{
    if (NULL != m_pFriendListWidget->currentItem())
    {
        QString strChatName = m_pFriendListWidget->currentItem()->text(); //取出选中的好友，作为私聊对象
        PrivateChat::getInstance().setChatName(strChatName);

        if (PrivateChat::getInstance().isHidden())
        {
            PrivateChat::getInstance().show();
        }
    }
    else
    {
        QMessageBox::warning(this, "私聊", "请选择私聊对象");
    }
}

void Friend::groupChat()
{
    QString strMsg = m_pInputMsgLE->text(); //获取聊天框内容
    m_pInputMsgLE->clear(); //清除聊天框内容
    if (!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strName = TcpClient::getInstance().getLoginName();
        strncpy(pdu->caData, strName.toStdString().c_str(), strName.size());
        strncpy((char*)(pdu->caMSG), strMsg.toStdString().c_str(), strMsg.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this, "群聊", "信息不能为空");
    }
}

