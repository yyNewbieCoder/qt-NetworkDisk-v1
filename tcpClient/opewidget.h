#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include <QListWidget>  //控件类，主要用于显示和管理项目列表
#include <QStackedWidget>   //堆栈窗口，可容纳多个窗口，但每次只显示一个
#include "friend.h"
#include "book.h"

class OpeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);
    static OpeWidget &getInstance();
    Friend *getFriend();
    Book *getBook();

signals:

private:
    QListWidget *m_pListW;
    Friend *m_pFriend;
    Book *m_pBook;
    QStackedWidget *m_pSW;
};

#endif // OPEWIDGET_H
