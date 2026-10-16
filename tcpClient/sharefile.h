#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>  //浏览/展示区域，一个画面放不下内容，则通过滑动条往下拖
#include <QListWidget>

class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);
    static ShareFile &getInstance();
    void updateFriend(QListWidget *pFriendList);

signals:

public slots:
    void cancelSelect();
    void selectAll();

    void okShare();
    void cancelShare();


private:
    QPushButton *m_pSelectAllPB;        //全选
    QPushButton *m_pCancelSelectAllPB;  //取消全选

    QPushButton *m_pOKPB;
    QPushButton *m_pCanCelPB;

    QScrollArea *m_pSA;
    QWidget     *m_pFriendW;            //实际的名字是放在Widget里,Widget交给展示区，去进行展示
    QVBoxLayout *m_pFriendWVBL;
    QButtonGroup *m_pButtonGroup;
};

#endif // SHAREFILE_H
