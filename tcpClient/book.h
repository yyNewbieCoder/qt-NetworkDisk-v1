#ifndef BOOK_H
#define BOOK_H

#include <QWidget>  //所有用户界面对象的基类
#include <QListWidget>  //列表控件，用于显示和管理项目列表，支持单选/多选、拖放
#include <QPushButton>
#include <QVBoxLayout>  //Qt Horizontal Box Layout,	Qt水平盒子布局
#include <QHBoxLayout>  //Qt Vertical Box Layout,	Qt垂直盒子布局
#include "protocol.h"
#include <QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU *pdu);
    QString getEnterPath();
    void clearEnterPath();
    QString getShareFileName();


signals:

public slots:
    void createDir();
    void flushFile();
    void delDir();
    void renameFile();
    void enterDir(const QModelIndex &index);    //槽函数的参数类型必须与信号参数类型兼容
    void returnPreDir();
    void uploadFile();
    void uploadFileData();
    void delRegFile();
    void downloadFile();
    void shareFile();
    void moveFile();
    void selectDestDir();

private:
    QListWidget *m_pBookListW;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;

    QString m_strEnterPath;
    QString m_strUploadFilePath;

    QTimer *m_pTimer;

    QString m_strShareFileName;

    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;
};

#endif // BOOK_H
