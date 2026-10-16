#include <QDebug>
#include <QMessageBox>
#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"

Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu)
{
    if (NULL == pdu)
    {
        return;
    }

    uint uiSise = pdu->uiMsgLen/NAME_MAX_LEN;
    char caTmp[NAME_MAX_LEN];
    for (uint i = 0; i < uiSise; i++)
    {
        memcpy(caTmp, (char*)pdu->caMSG + i*NAME_MAX_LEN, NAME_MAX_LEN);
        qDebug() << caTmp;
        ui->Online_lw->addItem(caTmp);
    }
}

void Online::on_addFriend_pb_clicked()
{
    QListWidgetItem *pItem =  ui->Online_lw->currentItem(); //获取当前选中的那一项
    //qDebug() << pItem->text();
    QString strPerUsrName = pItem->text();  //获取此项的内容，也就是用户名(将要添加的好友)
    QString strLoginName = TcpClient::getInstance().getLoginName(); //自己的用户名

    if (strPerUsrName == strLoginName)
    {
        QMessageBox::information(this, "添加好友", "不可以添加自己");
        return;
    }

    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    strncpy(pdu->caData, strPerUsrName.toStdString().c_str(), strPerUsrName.size());
    strncpy(pdu->caData + NAME_MAX_LEN, strLoginName.toStdString().c_str(), strLoginName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

