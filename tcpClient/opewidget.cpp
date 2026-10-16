#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent)
    : QWidget{parent}
{
    //以下的成员，都会成为OpeWidget的子对象。当父对象被销毁时，会自动销毁所有子对象。所以不需要手动去delete

    // 创建列表控件
    m_pListW = new QListWidget(this);
    // 添加简单文本项
    m_pListW->addItem("好友");
    m_pListW->addItem("图书");

    m_pFriend = new Friend;
    m_pBook = new Book;

    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pSW);

    setLayout(pMain);

    connect(m_pListW, SIGNAL(currentRowChanged(int)), m_pSW, SLOT(setCurrentIndex(int)));   //让好友和图书跳到其对应的窗口
}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}
