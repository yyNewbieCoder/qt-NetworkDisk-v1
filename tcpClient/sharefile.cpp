#include "sharefile.h"
#include "tcpclient.h"
#include "opewidget.h"
#include <QCheckBox>

ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectAllPB = new QPushButton("取消选择");

    m_pOKPB = new QPushButton("确定");
    m_pCanCelPB = new QPushButton("取消");

    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);    //
    m_pButtonGroup = new QButtonGroup(m_pFriendW);  // QButtonGroup的父对象是m_pFriendW
    m_pButtonGroup->setExclusive(false);     //设为多选。设置按钮组是否互斥，true为单选，false为多选。
    //全选，取消全选。
    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectAllPB);
    pTopHBL->addStretch();  //右边加个弹簧
    //确定，取消。
    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCanCelPB);
    //垂直布局
    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);   //上方放pTopHBL
    pMainVBL->addWidget(m_pSA);     //中间放展示区域
    pMainVBL->addLayout(pDownHBL);  //下方放pDownHBL

    setLayout(pMainVBL);    //最后，将垂直布局设定为总布局

    connect(m_pCancelSelectAllPB, SIGNAL(clicked(bool)), this, SLOT(cancelSelect()));
    connect(m_pSelectAllPB, SIGNAL(clicked(bool)), this, SLOT(selectAll()));
    connect(m_pOKPB, SIGNAL(clicked(bool)), this, SLOT(okShare()));
    connect(m_pCanCelPB, SIGNAL(clicked(bool)), this, SLOT(cancelShare()));
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if (NULL == pFriendList)
    {
        return;
    }
    //先移除旧的
    QAbstractButton *tmp = NULL;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    for (int i=0; i<preFriendList.size(); i++)
    {
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }

    QCheckBox *pCB = NULL;
    for (int i=0; i<pFriendList->count(); i++)
    {
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0; i<cbList.size(); i++)
    {
        if (cbList[i]->isChecked())
        {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0; i<cbList.size(); i++)
    {
        if (!cbList[i]->isChecked())
        {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShare()
{
    QString strName = TcpClient::getInstance().getLoginName();
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();

    QString strPath = strCurPath + "/" + strShareFileName;

    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    int num = 0;
    for (int i=0; i<cbList.size(); i++)
    {
        if (cbList[i]->isChecked())
        {
            num++;
        }
    }

    PDU *pdu = mkPDU(NAME_MAX_LEN*num+strPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData, "%s %d", strName.toStdString().c_str(), num);  //放分享人自己的名字 和 被分享人的人数
    int j = 0;
    for (int i=0; i<cbList.size(); i++)
    {
        if (cbList[i]->isChecked())
        {
            memcpy((char*)(pdu->caMSG)+j*NAME_MAX_LEN, cbList[i]->text().toStdString().c_str(), cbList[i]->text().size());  //放被分享人的名字
            j++;
        }
    }

    memcpy((char*)(pdu->caMSG)+num*NAME_MAX_LEN, strPath.toStdString().c_str(), strPath.size());    //放分享的文件（完整名）
    qDebug() <<"strPath:" << strPath;
    qDebug() <<"pdu->caMSG:" << (char*)(pdu->caMSG);

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

    free(pdu);
    pdu = NULL;
}

void ShareFile::cancelShare()
{
    hide();
}
