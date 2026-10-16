#include <QMessageBox>
#include "privatechat.h"
#include "ui_privatechat.h"
#include "tcpclient.h"

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName;
    m_strLoginName = TcpClient::getInstance().getLoginName();
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if (NULL == pdu)
    {
        return;
    }

    if (getInstance().isHidden())
    {
        getInstance().show();
    }

    char caSendName[32] = {0};
    memcpy(caSendName, pdu->caData, 32);
    setChatName(caSendName);    //记录下是谁给我发的，以便回复
    QString strMsg = QString("%1 says: %2").arg(caSendName).arg((char*)(pdu->caMSG));
    ui->showMsg_TE->append(strMsg); //把聊天信息，显示到 信息展示框
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::on_sendMsg_PB_clicked()
{
    QString strMsg = ui->inputMsg_LE->text();
    ui->inputMsg_LE->clear();   //获取完聊天框信息后，就清除掉
    if (!strMsg.isEmpty())  //判断聊天框内容，是否为空
    {
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, m_strLoginName.toStdString().c_str(), m_strLoginName.size());
        memcpy(pdu->caData+NAME_MAX_LEN, m_strChatName.toStdString().c_str(), m_strChatName.size());
        strcpy((char*)(pdu->caMSG), strMsg.toStdString().c_str());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this, "私聊", "发送的聊天信息为空!");
    }
}

