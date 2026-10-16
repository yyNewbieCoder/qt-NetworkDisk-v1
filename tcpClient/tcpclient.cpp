#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)   //调用父类QWidget的构造函数，并传入parent。此处创建窗口对象，获取窗口句柄，设置默认属性
    , ui(new Ui::TcpClient) //在堆上分配一个UI助手对象。注意：Ui::TcpClient不是TcpClient，作用域不一样
{
    ui->setupUi(this);  //用这个UI助手对象，去装饰窗口（也就是我们提前通过拖拽设置好的布局参数）
    loadConfig();

    /*连接服务器的结果，需要一个函数来进行展示，故用connect绑定一个槽函数
     *QTcpSocket 类中有一个名为 connected() 的信号，调用 m_tcpSocket.connectToHost()
     *并成功连接到指定的主机和端口时，这个信号会被触发*/
    connect(&m_tcpSocket, SIGNAL(connected()), // 信号发送方（Socket变量），发送信号类型
            this, SLOT(showConnect())); // 信号处理方，用以处理的槽函数

    /*客户端向服务端发送消息后的回应，
     * 客户端套接字收到数据后会有一个readyRead()信号*/
    connect(&m_tcpSocket, SIGNAL(readyRead()),
            this, SLOT(recvMsg()));

    // 连接服务器。（连接建立后，客户端 write 操作，服务端就能接收到数据）
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);

    m_bDownload = false;
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();
        strData.replace("\r\n", " ");
        QStringList strList = strData.split(" ");
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() << "ip:" << m_strIP << "port:"<< m_usPort;
    }
    else
    {
        QMessageBox::critical(this, "open config", "open config failed");
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::getLoginName()
{
    return m_strLoginName;
}

QString TcpClient::getCurPath()
{
    return m_strCurPath;
}

void TcpClient::setSaveFilePath(QString path)
{
    m_strSaveFilePath = path;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

void TcpClient::recvMsg()
{
    if (!m_bDownload) //当前没有处于下载状态，则处理消息
    {
        qDebug() << m_tcpSocket.bytesAvailable();
        uint uiPDULen = 0;
        m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        m_tcpSocket.read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));
        switch (pdu->uiMsgType)
        {
            case ENUM_MSG_TYPE_REGIST_RESPOND:
            {
                if (0 == strcmp(pdu->caData, REGIST_OK))
                {
                    QMessageBox::information(this, "注册", REGIST_OK);
                }
                else if (0 == strcmp(pdu->caData, REGIST_FAILED))
                {
                    QMessageBox::warning(this, "注册", REGIST_FAILED);
                }
                break;
            }
            case ENUM_MSG_TYPE_LOGIN_RESPOND:
            {
                if (0 == strcmp(pdu->caData, LOGIN_OK))
                {
                    m_strCurPath = QString("./%1").arg(m_strLoginName);
                    QMessageBox::information(this, "登录", LOGIN_OK);
                    OpeWidget::getInstance().show();    //进入操作界面（二级界面）
                    this->hide();
                }
                else if (0 == strcmp(pdu->caData, LOGIN_FAILED))
                {
                    QMessageBox::warning(this, "登录", LOGIN_FAILED);
                }
                break;
            }
            case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
            {
                OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
                break;
            }
            case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
            {
                qDebug() << "222" << pdu->caData;
                if (0 == strcmp(SEARCH_USR_NO, pdu->caData))
                {
                    QMessageBox::information(this, "搜索", QString("%1: not exist").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
                }
                else if (0 == strcmp(SEARCH_USR_ONLINE, pdu->caData))
                {
                    QMessageBox::information(this, "搜索", QString("%1: online").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
                }
                else if (0 == strcmp(SEARCH_USR_OFFLINE, pdu->caData))
                {
                    QMessageBox::information(this, "搜索", QString("%1: offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
                }
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
            {
                char applicantName[32] = {0};
                char receiverName[32] = {0};
                strncpy(receiverName, pdu->caData, NAME_MAX_LEN);
                strncpy(applicantName, pdu->caData+NAME_MAX_LEN, NAME_MAX_LEN);

                int ret = QMessageBox::information(this, "添加好友", QString("%1 want to add you as friend ?").arg(applicantName), QMessageBox::Yes, QMessageBox::No);
                PDU *respdu = mkPDU(0);
                memcpy(respdu->caData, receiverName, NAME_MAX_LEN);
                memcpy(respdu->caData+NAME_MAX_LEN, applicantName, NAME_MAX_LEN);
                if (QMessageBox::Yes == ret)
                {
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
                }
                else
                {
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
                }
                m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
            {
                QMessageBox::information(this, "添加好友", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
            {
                QMessageBox::information(this, "添加好友", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
            {
                QMessageBox::information(this, "添加好友", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
            {
                OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
                break;
            }
            case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
            {
                char caName[32] = {0};
                memcpy(caName, pdu->caData, NAME_MAX_LEN);
                QMessageBox::information(this, "删除好友", QString("%1 删除你作为他的好友").arg(caName));
                break;
            }
            case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
            {
                QMessageBox::information(this, "删除好友", "删除好友成功");
                break;
            }
            case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
            {
                OpeWidget::getInstance().getFriend()->showPrivateChatText(pdu);
                break;
            }
            case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
            {
                OpeWidget::getInstance().getFriend()->showGroupChatText(pdu);
                break;
            }
            case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
            {
                QString strCreateResult = pdu->caData;
                QMessageBox::information(this, "创建文件夹", strCreateResult);
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
            {
                OpeWidget::getInstance().getBook()->updateFileList(pdu);
                //更新当前路径
                QString strCurrentPath = OpeWidget::getInstance().getBook()->getEnterPath();
                if (!strCurrentPath.isEmpty())
                {
                    m_strCurPath = strCurrentPath;
                    qDebug() << "进入路径:" << m_strCurPath;
                }

                break;
            }
            case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
            {
                QMessageBox::information(this, "删除文件夹", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
            {
                QMessageBox::information(this, "重命名文件", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
            {
                QMessageBox::information(this, "进入目录", pdu->caData);
                OpeWidget::getInstance().getBook()->clearEnterPath();
                break;
            }
            case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
            {
                QMessageBox::information(this, "上传文件", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_DELETE_FILE_RESPOND:
            {
                QMessageBox::information(this, "删除文件", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
            {
                qDebug() << pdu->caData;
                char caFileName[NAME_MAX_LEN] = {0};
                sscanf(pdu->caData, "%s %lld", caFileName, &m_iTotal);
                if (strlen(caFileName)>0 && m_iTotal>0)
                {
                    m_bDownload = true;
                    m_file.setFileName(m_strSaveFilePath);
                    if (!m_file.open(QIODevice::WriteOnly))
                    {
                         QMessageBox::warning(this, "下载文件", "获得保存文件的路径有误");
                    }
                }
                break;
            }
            case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
            {
                QMessageBox::information(this, "共享文件", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_SHARE_FILE_NOTIFY_REQUEST:
            {
                char *pPath = new char[pdu->uiMsgLen];
                memcpy(pPath, pdu->caMSG, pdu->uiMsgType);
                qDebug() << "pPath:" << pPath;
                char *pos = strrchr(pPath, '/');  //strrchr()查找一个字符在字符串中最后一次出现的位置
                if (NULL != pos)
                {
                    pos++;
                    QString strNote = QString("%1 share file->%2 \n Do you accept").arg(pdu->caData).arg(pos);
                    int ret = QMessageBox::question(this, "共享文件", strNote);
                    if (QMessageBox::Yes == ret)
                    {
                        PDU *respdu = mkPDU(pdu->uiMsgLen);
                        respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTIFY_RESPOND;
                        memcpy(respdu->caMSG, pdu->caMSG, pdu->uiMsgLen);
                        QString strName = TcpClient::getInstance().getLoginName();
                        strcpy(respdu->caData, strName.toStdString().c_str());
                        m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
                    }
                }
                break;
            }
            case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
            {
                QMessageBox::information(this, "移动文件", pdu->caData);
                break;
            }
            default:break;
        }
        free(pdu);
        pdu = NULL;
    }
    else    //当前处于下载状态，则收文件数据
    {
        QByteArray buff = m_tcpSocket.readAll();
        m_file.write(buff);
        m_iRecved += buff.size();
        if (m_iRecved == m_iTotal)  //收到的文件大小 = 文件总大小，代表已经文件已经下载完
        {
            m_file.close();
            m_iTotal = 0;
            m_iRecved = 0;
            m_bDownload = false;
            QMessageBox::information(this, "下载文件", "下载文件成功");
        }
        else if (m_iRecved > m_iTotal)
        {
            m_file.close();
            m_iTotal = 0;
            m_iRecved = 0;
            m_bDownload = false;
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }
    }
}

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty())
    {
        m_strLoginName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), NAME_MAX_LEN);
        strncpy(pdu->caData+NAME_MAX_LEN, strPwd.toStdString().c_str(), PWD_MAX_LEN);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "登录", "登录失败:用户名或密码为空");
    }
}


void TcpClient::on_register_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), NAME_MAX_LEN);
        strncpy(pdu->caData+NAME_MAX_LEN, strPwd.toStdString().c_str(), PWD_MAX_LEN);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
    }
    else
    {
        QMessageBox::critical(this, "注册", "注册失败:用户名或密码为空");
    }
}


void TcpClient::on_logout_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGOUT_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), NAME_MAX_LEN);
        strncpy(pdu->caData+NAME_MAX_LEN, strPwd.toStdString().c_str(), PWD_MAX_LEN);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
    }
    else
    {
        QMessageBox::critical(this, "注销", "注销失败:用户名或密码为空");
    }
}

