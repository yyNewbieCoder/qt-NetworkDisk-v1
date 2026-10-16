#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "opewidget.h"
#include "sharefile.h"

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_strEnterPath.clear();

    m_pTimer = new QTimer;
    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名文件");
    m_pFlushFilePB = new QPushButton("刷新文件");

    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);    //addWidget添加控件: 按钮、标签、文本框等可见控件
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUploadPB = new QPushButton("上传文件");
    m_pDownLoadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("共享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("设为移动目录");
    m_pSelectDirPB->setEnabled(false);

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);     //addLayout, 添加子布局
    pMain->addLayout(pFileVBL);

    setLayout(pMain);   //设置为总布局

    connect(m_pCreateDirPB, SIGNAL(clicked(bool)), this, SLOT(createDir()));
    connect(m_pFlushFilePB, SIGNAL(clicked(bool)), this, SLOT(flushFile()));
    connect(m_pDelDirPB, SIGNAL(clicked(bool)), this, SLOT(delDir()));
    connect(m_pRenamePB, SIGNAL(clicked(bool)), this, SLOT(renameFile()));
    connect(m_pBookListW, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(enterDir(QModelIndex)));   //双击信号
    connect(m_pReturnPB, SIGNAL(clicked(bool)), this, SLOT(returnPreDir()));
    connect(m_pUploadPB, SIGNAL(clicked(bool)), this, SLOT(uploadFile()));
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(uploadFileData()));
    connect(m_pDelFilePB, SIGNAL(clicked(bool)), this, SLOT(delRegFile()));
    connect(m_pDownLoadPB, SIGNAL(clicked(bool)), this, SLOT(downloadFile()));
    connect(m_pShareFilePB, SIGNAL(clicked(bool)), this, SLOT(shareFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked(bool)), this, SLOT(moveFile()));
    connect(m_pSelectDirPB, SIGNAL(clicked(bool)), this, SLOT(selectDestDir()));
}

void Book::updateFileList(const PDU *pdu)
{
    if (NULL == pdu)
    {
        return;
    }
    //先清屏
    QListWidgetItem *pItemTmp = NULL;
    int row = m_pBookListW->count();
    while (row > 0)
    {
        pItemTmp = m_pBookListW->item(row-1);   //从后往前 清屏
        m_pBookListW->removeItemWidget(pItemTmp);
        delete pItemTmp;
        row--;
    }

    //再展示
    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);

    for (int i=0; i<iCount; i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMSG) + i;
        qDebug() << iCount << pFileInfo->caFileName << pFileInfo->iFileType;
        QListWidgetItem *pItem = new QListWidgetItem;
        if (0 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/photo/dir.jpeg")));     //setIcon为控件添加一个图形化的图标（通常是小图片），增强界面的可视化效果和用户体验
        }
        else if (1 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/photo/file.jpeg")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}

QString Book::getEnterPath()
{
    return m_strEnterPath;
}

void Book::clearEnterPath()
{
    m_strEnterPath.clear();
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this, "新建文件夹", "新文件夹名字"); //QInputDialog 获取输入框的内容
    if (!strNewDir.isEmpty())
    {
        if (strNewDir.size() <= FILE_MAX_LEN)
        {
            QString strName = TcpClient::getInstance().getLoginName();
            QString strCurPath = TcpClient::getInstance().getCurPath();
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData, strName.toStdString().c_str(), strName.size());
            strncpy(pdu->caData+NAME_MAX_LEN, strNewDir.toStdString().c_str(), strNewDir.size());
            memcpy((char*)(pdu->caMSG), strCurPath.toStdString().c_str(), strCurPath.size());

            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(NULL);
            pdu = NULL;
        }
        else
        {
            QMessageBox::warning(this, "新建文件夹", "新文件夹名字过长(不能大于32字节)");
        }

    }
    else
    {
        QMessageBox::warning(this, "新建文件夹", "新文件夹名字不能为空");
    }

}

void Book::flushFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMSG), strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(NULL);
    pdu = NULL;
}

void Book::delDir()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem)
    {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
    }
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
        memcpy(pdu->caMSG, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(NULL);
        pdu = NULL;
    }
}

void Book::renameFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem)
    {
        QMessageBox::warning(this, "重命名文件", "请选择要重命名的文件");
    }
    else
    {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this, "重命名文件", "请输入新的文件名");
        if (!strNewName.isEmpty() && strNewName.size() <= FILE_MAX_LEN)
        {
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.size());
            strncpy(pdu->caData+FILE_MAX_LEN, strNewName.toStdString().c_str(), strNewName.size());
            memcpy(pdu->caMSG, strCurPath.toStdString().c_str(), strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(NULL);
            pdu = NULL;
        }
        else
        {
            QMessageBox::warning(this, "重命名文件", "新文件名字不能为空, 并且长度不能大于32");
        }
    }

}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();   //得到双击文件的文件名
    QString strCurPath = TcpClient::getInstance().getCurPath();
    m_strEnterPath = strCurPath + "/" + strDirName;
    qDebug() << "strCurPath : " << strCurPath;
    qDebug() << "m_strEnterPath : " << m_strEnterPath;
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData, strDirName.toStdString().c_str(), strDirName.size());
    memcpy(pdu->caMSG, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(NULL);
    pdu = NULL;
}

void Book::returnPreDir()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QString strUsrPath = "./" + TcpClient::getInstance().getLoginName();
    if (strUsrPath == strCurPath)   //已经在./usrname/下，不能再继续返回
    {
        QMessageBox::warning(this, "返回", "已在用户初始目录，不能继续返回");
    }
    else
    {
        QString strPrePath = strCurPath.section('/', 0, -2);    //无论路径有多深，section('/', 0, -2) 都会删除最后一个 / 之后的部分，返回前面的所有部分
        m_strEnterPath = strPrePath;
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
        strncpy((char*)(pdu->caMSG), strPrePath.toStdString().c_str(), strPrePath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(NULL);
        pdu = NULL;
    }
}

//1.上传文件的名字  2.上传多大的文件  3.上传到哪里？
void Book::uploadFile()
{
    m_strUploadFilePath = QFileDialog::getOpenFileName();     //getOpenFileName 打开一个选取文件的窗口，选中后点击打开，会返回选中的文件的“绝对路径”
    qDebug() << m_strUploadFilePath;
    if (!m_strUploadFilePath.isEmpty())
    {
        // 假如a/bb/ccc, 要提取ccc, 此处lastIndexOf会返回最后一个/之前字符的位置，为4
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);    // 完整路径从右开始取，不要/，得到文件名

        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();  // 获得文件大小

        QString strCurPath = TcpClient::getInstance().getCurPath();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMSG, strCurPath.toStdString().c_str(), strCurPath.size());
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), fileSize);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(NULL);
        pdu = NULL;

        m_pTimer->start(1000);  //一秒后，开始上传文件数据。（避免消息和数据一起发，造成粘包）
    }
    else
    {
        QMessageBox::warning(this, "上传文件", "上传文件名字不能为空");
    }
}

void Book::uploadFileData()
{
    m_pTimer->stop();   //先关闭定时间，不然会继续计时

    QFile file(m_strUploadFilePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }

    char *pBuffer = new char[4096]; //每次读4096，综合实测，效率较高
    qint64 ret = 0;
    while (1)
    {
        ret = file.read(pBuffer, 4096);
        if (ret > 0 && ret <= 4096) //正常读到数据
        {
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);
        }
        else if (0 == ret)  //数据读完了
        {
            break;
        }
        else    //读错
        {
            QMessageBox::warning(this, "上传文件", "上传文件失败:读文件失败");
            break;
        }
    }
    file.close();
    delete[] pBuffer;
    pBuffer = NULL;
}

void Book::delRegFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem)
    {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
    }
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
        memcpy(pdu->caMSG, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(NULL);
        pdu = NULL;
    }
}

void Book::downloadFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem)
    {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
    }
    else
    {
        QString strCurPath = TcpClient::getInstance().getCurPath();
        QString strFileName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        strncpy(pdu->caData, strFileName.toStdString().c_str(), strFileName.size());
        memcpy(pdu->caMSG, strCurPath.toStdString().c_str(), strCurPath.size());


        QString strSaveFilePath = QFileDialog::getSaveFileName();   //会弹出窗口，选择要保存的路径
        if (strSaveFilePath.isEmpty())
        {
            QMessageBox::warning(this, "下载文件", "请指定要保存的位置");
            //m_strSaveFilePath.clear();
        }
        else
        {
            TcpClient::getInstance().setSaveFilePath(strSaveFilePath);  //客户端会把文件内容下载到此文件中
        }

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(NULL);
        pdu = NULL;
    }
}

void Book::shareFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem)
    {
        QMessageBox::warning(this, "分享文件", "请选择要分享的文件");
        return;
    }
    else
    {
        m_strShareFileName = pItem->text();
    }

    Friend *pFriend = OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList = pFriend->getFriendList();    //获得好友列表
    ShareFile::getInstance().updateFriend(pFriendList);     //将好友列表更新到“分享文件”的窗口中
    if (ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().show();
    }

}

void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if (NULL != pCurItem)
    {
        m_strMoveFileName = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().getCurPath();
        m_strMoveFilePath = strCurPath + '/' + m_strMoveFileName;
        m_pSelectDirPB->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if (NULL != pCurItem)
    {
        QString strDestDir = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().getCurPath();
        m_strDestDir = strCurPath + '/' + strDestDir;

        int srcLen = m_strMoveFilePath.size();  //从哪里移动
        int destLen = m_strDestDir.size();      //移动到哪里

        PDU *pdu = mkPDU(srcLen+destLen+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData, "%d %d %s", srcLen, destLen, m_strMoveFileName.toStdString().c_str());

        memcpy(pdu->caMSG, m_strMoveFilePath.toStdString().c_str(), srcLen);
        memcpy((char*)(pdu->caMSG)+srcLen+1, m_strDestDir.toStdString().c_str(), destLen);  //+1 给一个\0

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    }

    //m_pSelectDirPB->setEnabled(false);
}
