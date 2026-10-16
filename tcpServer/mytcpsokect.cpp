#include "mytcpsokect.h"
#include <QDebug>
#include "protocol.h"
#include "opedb.h"
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>

MyTcpSokect::MyTcpSokect(QObject *parent)
    : QTcpSocket{parent}
{
    m_bUpload = false;
    m_pTimer = new QTimer;

    connect(this, SIGNAL(readyRead()), this, SLOT(recvMsg()));      //收到套接字数据，就有readyRead信号，就调用一下recvMsg
    connect(this, SIGNAL(disconnected()), this, SLOT(clientOffline()));   //只要客户端与服务端断开连接，就有disconnected信号，服务端就调用一下clientOffline
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(sendFileToClient()));
}

QString MyTcpSokect::getName()
{
    return m_strName;
}

void MyTcpSokect::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);   //先产生一个新目录

    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();

    QString srcTmp;
    QString destTmp;
    for (int i=0; i<fileInfoList.size(); i++)
    {
        qDebug() << "fileName :" << fileInfoList[i].fileName();
        if (fileInfoList[i].isFile())
        {
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            QFile::copy(srcTmp, destTmp);
        }
        else if (fileInfoList[i].isDir())
        {
            if (QString(".") == fileInfoList[i].fileName() || QString("..") == fileInfoList[i].fileName())
            {
                continue;
            }

            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            copyDir(srcTmp, destTmp);   //写成递归
        }
    }
}

void MyTcpSokect::recvMsg()
{
    if (!m_bUpload) //目前没有上传文件，则接受消息，进行处理。文件上传是二进制，不走消息逻辑
    {
        qDebug() << this->bytesAvailable(); //获取当前可立即读取的字节数
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(uint)); //调用当前套接字对象的 read() 方法, 每个套接字对象内部都维护着自己的连接和数据缓冲区
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));     //第一次调用read已经读走了uiPDULen（uint ），所以此处少读一个uint的长度
        switch (pdu->uiMsgType)
        {
            case ENUM_MSG_TYPE_REGIST_REQUEST:  //注册请求
            {
                char caName[32] = {0};
                char caPwd[32] = {0};
                strncpy(caName, pdu->caData, NAME_MAX_LEN);
                strncpy(caPwd, pdu->caData+NAME_MAX_LEN, PWD_MAX_LEN);

                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;

                bool ret = OpeDB::getInstance().handleRegist(caName, caPwd);
                qDebug() << ret;
                if (ret)
                {
                    strcpy(respdu->caData, REGIST_OK);
                    QDir dir;
                    qDebug() << "create dir: " << dir.mkdir(QString("./%1").arg(caName));//QT会以构建目录（build-tcpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Debug）作为根目录
                }
                else
                {
                    strcpy(respdu->caData, REGIST_FAILED);
                }

                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_LOGIN_REQUEST:   //登录请求
            {
                char caName[32] = {0};
                char caPwd[32] = {0};
                strncpy(caName, pdu->caData, NAME_MAX_LEN);
                strncpy(caPwd, pdu->caData+NAME_MAX_LEN, PWD_MAX_LEN);

                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;

                bool ret = OpeDB::getInstance().handleLogin(caName, caPwd);
                qDebug() << "handle login ret" << ret;
                if (ret)
                {
                    strcpy(respdu->caData, LOGIN_OK);
                    m_strName = caName;
                }
                else
                {
                    strcpy(respdu->caData, LOGIN_FAILED);
                }

                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
            {
                QStringList ret = OpeDB::getInstance().handAllOnline();
                uint msgLen = ret.size() * NAME_MAX_LEN;  //每个名字最长32字节
                PDU *resPdu = mkPDU(msgLen);
                resPdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
                for (int i = 0; i < ret.size(); i++)
                {
                    memcpy((char*)(resPdu->caMSG)+NAME_MAX_LEN*i, ret.at(i).toStdString().c_str(), ret.at(i).size());
                }
                write((char*)resPdu, resPdu->uiPDULen);
                free(resPdu);
                resPdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
            {
                int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
                PDU *resPdu = mkPDU(0);
                resPdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;

                if (-1 == ret)
                {
                    strcpy(resPdu->caData, SEARCH_USR_NO);
                }
                else if (1 == ret)
                {
                    strcpy(resPdu->caData, SEARCH_USR_ONLINE);
                }
                else if (0 == ret)
                {
                    strcpy(resPdu->caData, SEARCH_USR_OFFLINE);
                }
                write((char*)resPdu, resPdu->uiPDULen);
                free(resPdu);
                resPdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
            {
                char caPerName[32] = {0};
                char caName[32] = {0};
                strncpy(caPerName, pdu->caData, NAME_MAX_LEN);
                strncpy(caName, pdu->caData+NAME_MAX_LEN, NAME_MAX_LEN);
                int ret = OpeDB::getInstance().handleAddFriend(caPerName, caName);
                PDU *respdu = NULL;
                if (-1 == ret)
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                    strcpy(respdu->caData, UNKNOW_ERROR);
                    write((char*)respdu, respdu->uiPDULen);
                    free(respdu);
                    respdu = NULL;
                }
                else if (0 == ret)
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                    strcpy(respdu->caData, EXIST_FRIEND);
                    write((char*)respdu, respdu->uiPDULen);
                    free(respdu);
                    respdu = NULL;
                }
                else if (1 == ret)
                {
                    MyTcpServer::getInstance().resend(caPerName, pdu);
                }
                else if (2 == ret)
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                    strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
                    write((char*)respdu, respdu->uiPDULen);
                    free(respdu);
                    respdu = NULL;
                }
                else if (3 == ret)
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                    strcpy(respdu->caData, ADD_FRIEND_NO_EXIST);
                    write((char*)respdu, respdu->uiPDULen);
                    free(respdu);
                    respdu = NULL;
                }
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
            {
                char caReceiverName[32] = {0};
                char caApplicantName[32] = {0};
                strncpy(caReceiverName, pdu->caData, NAME_MAX_LEN);
                strncpy(caApplicantName, pdu->caData+NAME_MAX_LEN, NAME_MAX_LEN);
                OpeDB::getInstance().handleAddFriendResult(caReceiverName, caApplicantName);
                MyTcpServer::getInstance().resendAddFriendResult(caApplicantName, true);
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
            {
                char caApplicantName[32] = {0};
                strncpy(caApplicantName, pdu->caData+NAME_MAX_LEN, NAME_MAX_LEN);
                MyTcpServer::getInstance().resendAddFriendResult(caApplicantName, false);
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
            {
                char caName[32] = {0};
                strncpy(caName, pdu->caData, NAME_MAX_LEN);
                QStringList ret = OpeDB::getInstance().handleFlushFriend(caName);
                uint uiMsgLen = ret.size() * NAME_MAX_LEN;
                PDU *respdu = mkPDU(uiMsgLen);
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
                for (int i = 0; i < ret.size(); i++)
                {
                    memcpy((char*)(respdu->caMSG)+i*NAME_MAX_LEN, ret.at(i).toStdString().c_str(), ret.at(i).size());
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
            {
                char caMyName[32] = {0};
                char caFriendName[32] = {0};
                strncpy(caMyName, pdu->caData, NAME_MAX_LEN);
                strncpy(caFriendName, pdu->caData+NAME_MAX_LEN, NAME_MAX_LEN);
                OpeDB::getInstance().handleDelFrinend(caMyName, caFriendName);

                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
                strcpy(respdu->caData, DEL_FRIEND_OK);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;

                MyTcpServer::getInstance().resend(caFriendName, pdu);
                break;
            }
            case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
            {
                char caChatName[32] = {0};
                strncpy(caChatName, pdu->caData+NAME_MAX_LEN, NAME_MAX_LEN);
                MyTcpServer::getInstance().resend(caChatName, pdu);
                break;
            }
            case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
            {
                char caName[32] = {0};
                memcpy(caName, pdu->caData, NAME_MAX_LEN);
                QStringList onlineFriends = OpeDB::getInstance().handleFlushFriend(caName);
                QString tmpFriendName;
                for (int i=0; i<onlineFriends.size(); i++)
                {
                    tmpFriendName = onlineFriends.at(i);
                    MyTcpServer::getInstance().resend(tmpFriendName.toStdString().c_str(), pdu);
                }
                break;
            }
            case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
            {
                QDir dir;
                QString strCurPath = QString ("%1").arg((char*)(pdu->caMSG));
                bool ret = dir.exists(strCurPath);
                PDU *respdu = NULL;
                if (ret)    //当前目录存在
                {
                    char caNewDir[32] = {0};
                    memcpy(caNewDir, pdu->caData+NAME_MAX_LEN, FILE_MAX_LEN);
                    QString strNewPath = strCurPath+"/"+caNewDir;   // "/"代表字符串字面量，必须用双引号
                    qDebug() << strNewPath;
                    ret = dir.exists(strNewPath);
                    if (ret)    //创建的文件名已存在，不能再创建
                    {
                        respdu = mkPDU(0);
                        respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                        strcpy(respdu->caData, FILE_NAME_EXIST);
                    }
                    else        //创建的文件名不存在，可以创建
                    {
                        dir.mkdir(strNewPath);
                        respdu = mkPDU(0);
                        respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                        strcpy(respdu->caData, CREATE_DIR_OK);
                    }
                }
                else        //当前目录不存在
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, DIR_NO_EXIST);
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
            {
                char *pCurPath = new char[pdu->uiPDULen];
                memcpy(pCurPath, pdu->caMSG, pdu->uiPDULen);
                QDir dir(pCurPath);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                PDU *respdu = mkPDU(sizeof(FileInfo)*(iFileCount-2));
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                FileInfo *tmpFileInfo = NULL;
                QString strFileName;
                int count = 0;
                for (int i=0; i<iFileCount; i++)
                {
                    if (QString(".") == fileInfoList[i].fileName() || QString("..") == fileInfoList[i].fileName())
                    {
                        continue;     //. 和 .. 不需要传到客户端
                    }

                    tmpFileInfo = (FileInfo*)(respdu->caMSG) + count;

                    strFileName = fileInfoList[i].fileName();
                    memcpy(tmpFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                    if (fileInfoList[i].isDir())    //目录（文件夹）
                    {
                        tmpFileInfo->iFileType = 0;
                    }
                    else if (fileInfoList[i].isFile())  //常规文件
                    {
                         tmpFileInfo->iFileType = 1;
                    }

                    count++;
                    //qDebug() << iFileCount << count << "---" << fileInfoList[i].fileName() << "---" << fileInfoList[i].size();
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
            {
                char caName[32] = {0};
                strncpy(caName, pdu->caData, NAME_MAX_LEN);
                char *pPath = new char[pdu->uiMsgLen];
                memcpy(pPath, pdu->caMSG, pdu->uiMsgLen);
                QString strPath = QString ("%1/%2").arg(pPath).arg(caName);
                delete[] pPath;
                pPath = NULL;

                QFileInfo fileInfo(strPath);
                bool ret = false;
                if (fileInfo.isDir())
                {
                    QDir dir;
                    dir.setPath(strPath);
                    ret = dir.removeRecursively();    //removeRecursively 删除这个目录，以及这个目录下的所有子目录
                }
                else if (fileInfo.isFile())
                {
                    ret = false;
                }

                PDU *respdu = NULL;
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                if (ret)
                {

                    memcpy(respdu->caData, DEL_DIR_OK, strlen(DEL_DIR_OK));
                }
                else
                {
                    memcpy(respdu->caData, DEL_DIR_FAILED, strlen(DEL_DIR_FAILED));
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
            {
                char caOldName[FILE_MAX_LEN] = {0};
                char caNewName[FILE_MAX_LEN] = {0};
                strncpy(caOldName, pdu->caData, FILE_MAX_LEN);
                strncpy(caNewName, pdu->caData+FILE_MAX_LEN, FILE_MAX_LEN);

                char *pPath = new char[pdu->uiMsgLen];
                memcpy(pPath, pdu->caMSG, pdu->uiMsgLen);

                QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
                QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);
                delete[] pPath;
                pPath = NULL;

                QDir dir;
                bool ret = dir.rename(strOldPath, strNewPath);
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
                if (ret)
                {
                    strcpy(respdu->caData, RENAME_FILE_OK);
                }
                else
                {
                    strcpy(respdu->caData, RENAME_FILE_FAILED);
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
            {
                char caEnterName[FILE_MAX_LEN] = {0};
                strncpy(caEnterName, pdu->caData, FILE_MAX_LEN);
                char *pPath = new char[pdu->uiMsgLen];
                memcpy(pPath, pdu->caMSG, pdu->uiMsgLen);

                QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);
                delete[] pPath;
                pPath = NULL;
                //延用刷新文件夹的代码
                QFileInfo fileInfo(strPath);
                PDU *respdu = NULL;
                if (fileInfo.isDir())
                {
                    QDir dir(strPath);
                    QFileInfoList fileInfoList = dir.entryInfoList();
                    int iFileCount = fileInfoList.size();
                    respdu = mkPDU(sizeof(FileInfo)*(iFileCount-2));
                    respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                    FileInfo *tmpFileInfo = NULL;
                    QString strFileName;
                    int count = 0;

                    for (int i=0; i<iFileCount; i++)
                    {
                        if (QString(".") == fileInfoList[i].fileName() || QString("..") == fileInfoList[i].fileName())
                        {
                            continue;     //. 和 .. 不需要传到客户端
                        }

                        tmpFileInfo = (FileInfo*)(respdu->caMSG) + count;

                        strFileName = fileInfoList[i].fileName();
                        memcpy(tmpFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                        if (fileInfoList[i].isDir())    //目录（文件夹）
                        {
                            tmpFileInfo->iFileType = 0;
                        }
                        else if (fileInfoList[i].isFile())  //常规文件
                        {
                            tmpFileInfo->iFileType = 1;
                        }

                        count++;
                        //qDebug() << iFileCount << count << "---" << fileInfoList[i].fileName() << "---" << fileInfoList[i].size();
                    }
                    write((char*)respdu, respdu->uiPDULen);
                    free(respdu);
                    respdu = NULL;
                }
                else if (fileInfo.isFile())
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                    strcpy(pdu->caData, ENTER_DIR_FAILED);
                    write((char*)respdu, respdu->uiPDULen);
                    free(respdu);
                    respdu = NULL;
                }
                break;
            }
            case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
            {
                char caFileName[FILE_MAX_LEN] = {0};
                qint64 fileSize = 0;
                sscanf(pdu->caData, "%s %lld", caFileName, &fileSize);

                char *pPath = new char[pdu->uiMsgLen];
                memcpy(pPath, pdu->caMSG, pdu->uiMsgLen);
                QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
                qDebug() << "server :" << strPath;
                delete[] pPath;
                pPath = NULL;

                m_file.setFileName(strPath);
                if (m_file.open(QIODevice::WriteOnly))  //以只写方式打开文件，文件不存在则创建
                {
                    m_bUpload = true;
                    m_iTotal = fileSize;
                    m_iRecved = 0;
                }
                else
                {
                    qDebug() << "m_file open failed";
                }
                break;
            }
            case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:
            {
                char caName[32] = {0};
                strncpy(caName, pdu->caData, NAME_MAX_LEN);
                char *pPath = new char[pdu->uiMsgLen];
                memcpy(pPath, pdu->caMSG, pdu->uiMsgLen);
                QString strPath = QString ("%1/%2").arg(pPath).arg(caName);
                delete[] pPath;
                pPath = NULL;

                QFileInfo fileInfo(strPath);
                bool ret = false;
                if (fileInfo.isDir())
                {
                    ret = false;
                }
                else if (fileInfo.isFile())
                {
                    //QDir 和 QFile都能用
                    // QDir dir;
                    // ret = dir.remove(strPath);
                    QFile file(strPath);
                    ret = file.remove();
                }

                PDU *respdu = NULL;
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
                if (ret)
                {

                    memcpy(respdu->caData, DEL_FILE_OK, strlen(DEL_FILE_OK));
                }
                else
                {
                    memcpy(respdu->caData, DEL_FILE_FAILED, strlen(DEL_FILE_FAILED));
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
            {
                //拼接完整下载路径
                char caFileName[FILE_MAX_LEN] = {0};
                strcpy(caFileName, pdu->caData);
                char *pPath = new char[pdu->uiMsgLen];
                memcpy(pPath, pdu->caMSG, pdu->uiMsgLen);
                QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
                delete[] pPath;
                pPath = NULL;

                //告诉客户端下载的文件名和大小
                QFileInfo fileInfo(strPath);
                qint64 fileSize = fileInfo.size();
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
                sprintf(respdu->caData, "%s %lld", caFileName, fileSize);

                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;

                m_file.setFileName(strPath);
                m_file.open(QIODevice::ReadOnly);
                m_pTimer->start(1000);
                break;
            }
            case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
            {
                char caSendName[NAME_MAX_LEN] = {0};
                int num = 0;
                sscanf(pdu->caData, "%s %d", caSendName, &num);
                qDebug() << "caSendName:"<<caSendName<<"num:"<<num;
                int size = num * NAME_MAX_LEN;
                PDU *respdu = mkPDU(pdu->uiMsgLen-size);    //路径所占空间
                respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTIFY_REQUEST;
                strcpy(respdu->caData, caSendName); //拷贝分享人的名字
                memcpy((char*)(respdu->caMSG), (char*)(pdu->caMSG)+size, pdu->uiMsgLen-size);   //拷贝路径
                qDebug() << "respdu->caMSG"<<(char*)(respdu->caMSG) << "pdu->uiMsgLen:"<<pdu->uiMsgLen;
                //将消息转发给被分享人
                char caRecvName[NAME_MAX_LEN] = {0};
                for (int i=0; i < num; i++)
                {
                    memcpy(caRecvName, (char*)(pdu->caMSG)+i*NAME_MAX_LEN, NAME_MAX_LEN);
                    qDebug() << "caRecvName:"<<caRecvName;
                    MyTcpServer::getInstance().resend(caRecvName, respdu);
                }
                free(respdu);
                respdu = NULL;

                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
                strcpy(respdu->caData, "share file ok");
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;

                break;
            }
            case ENUM_MSG_TYPE_SHARE_FILE_NOTIFY_RESPOND:
            {
                QString strRecvPath = QString("./%1").arg(pdu->caData); //接收者名字（路径）
                QString strShareFilePath = QString("%1").arg((char*)(pdu->caMSG));
                int index = strShareFilePath.lastIndexOf('/');
                QString strFileName = strShareFilePath.right(strShareFilePath.length() - index - 1);
                strRecvPath = strRecvPath + '/' + strFileName; //接收者的路径+文件名

                QFileInfo fileInfo(strShareFilePath);
                if (fileInfo.isFile())
                {
                    QFile::copy(strShareFilePath, strRecvPath);
                }
                else if (fileInfo.isDir())
                {
                    copyDir(strShareFilePath, strRecvPath);
                }
                break;
            }
            case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
            {
                char caFileName[NAME_MAX_LEN] = {0};
                int srcLen = 0;
                int destLen = 0;
                sscanf(pdu->caData, "%d%d%s", &srcLen, &destLen, caFileName);
                qDebug()<<"server---"<<"srcLen"<<srcLen<<"destLen"<<destLen<<"caFileName"<<caFileName;
                char *pSrcPath = new char[srcLen+1]();
                char *pDestPath = new char[destLen+1+FILE_MAX_LEN]();

                memcpy(pSrcPath, pdu->caMSG, srcLen);
                memcpy(pDestPath, (char*)(pdu->caMSG)+(srcLen+1), destLen);

                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
                QFileInfo fileInfo(pDestPath);
                if (fileInfo.isDir())
                {
                    strcat(pDestPath, "/");
                    strcat(pDestPath, caFileName);

                    bool ret = QFile::rename(pSrcPath, pDestPath);
                    if (ret)
                    {
                        strcpy(respdu->caData, MOVE_FILE_OK);
                    }
                    else
                    {
                        strcpy(respdu->caData, COMMON_ERR);
                    }
                }
                else if (fileInfo.isFile())
                {
                    strcpy(respdu->caData, MOVE_FILE_FAILED);
                }

                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            default:break;
        }
        free(pdu);
        pdu = NULL;
        //qDebug() << pdu->uiMsgType << (pdu->caData) << (pdu->caData+NAME_MAX_LEN);  //此处要用(char*)强转pdu->caMSG，才能打印出来
    }
    else
    {
        PDU *respdu = NULL;

        QByteArray buff = readAll();    //读取所有剩余可用的数据
        m_file.write(buff);             //m_file已经执行过setFileName(strPath)，此处直接写文件就行
        m_iRecved += buff.size();

        if (m_iTotal > m_iRecved)   //文件没写完
        {
            return;
        }
        else if (m_iTotal == m_iRecved)  //文件已经写完
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(respdu->caData, UPLOAD_FILE_OK);
        }
        else    //文件写错
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(respdu->caData, UPLOAD_FILE_FAILED);
        }
        m_file.close();
        m_bUpload = false;
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
    }
}

void MyTcpSokect::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this); //emit用于发送信号
}

void MyTcpSokect::sendFileToClient()
{
    char *pData = new char[4096]();
    qint64 ret = 0;
    while (true)
    {
        ret = m_file.read(pData, 4096);
        if (ret > 0 && ret <= 4096)
        {
            this->write(pData, ret);
        }
        else if (0 == ret)
        {
            qDebug() << "文件发送完毕";
            break;
        }
        else if (ret < 0)
        {
            qDebug() << "发送文件内容给客户端过程中失败!!!";
            break;
        }
    }

    m_file.close();
    m_pTimer->stop();
    delete[] pData;
    pData = NULL;
}
