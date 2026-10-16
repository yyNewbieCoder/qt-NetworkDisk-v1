#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB::~OpeDB()
{

}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("C:\\QtProjects\\NetworkDisk(written_by_myself)\\tcpServer\\cloud.db");
    if (m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while (query.next())
        {
            QString data = QString("%1 %2 %3")
                               .arg(query.value(0).toString())
                               .arg(query.value(1).toString())
                               .arg(query.value(2).toString());
            //ui->textEdit->append(data);
            qDebug() << data;
        }
    }
    else
    {
        QMessageBox::critical(NULL, "打开数据库·操作", "打开数据库失败");
    }
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if (NULL == name || NULL == pwd)
    {
        return false;
    }
    //把注册信息拼接成sql语句
    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\', \'%2\')").arg(name).arg(pwd);
    qDebug() << data;
    QSqlQuery query;
    return query.exec(data);    //name的索引是unique，无重名的返回成功
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    // 有效性判断
    if(NULL == name || NULL == pwd)
    {
        return false;
    }
    // 数据库查询
    QString strQuery = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' "
                                "and online = 0").arg(name).arg(pwd); // online = 0 可以判定用户是否未登录，不允许重复登陆

    QSqlQuery query;

    //qDebug() << "query command= " << strQuery;
    query.exec(strQuery);

    if(query.next()) // 每次调用next都会读取一条数据(再调next会读取下一条数据)，并将结果放入query中，返回值为true，无数据则返回false
    {
        // 如果登录成功，需要设置online = 1，并返回true
        strQuery = QString("update usrInfo set online = 1 where name = \'%1\' and pwd = \'%2\' ").arg(name).arg(pwd);
        return query.exec(strQuery);
    }
    else
    {
        return false;
    }
}

void OpeDB::handleOffline(const char *name)
{
    // 有效性判断
    if(NULL == name)
    {
        return;
    }

    QString data = QString("update usrInfo set online=0 where name=\'%1\'").arg(name);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handAllOnline()
{
    QString data = QString("select name from usrInfo where online=1");
    QSqlQuery query;
    query.exec(data);

    QStringList result;
    result.clear();
    while (query.next())
    {
        result.append(query.value(0).toString());
    }

    return result;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if (NULL == name)
    {
        return -1;
    }

    QString data = QString("select online from usrInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);

    if (query.next())
    {
        int ret = query.value(0).toInt();
        // if (1 == ret)
        // {
        //     return 1;
        // }
        // else
        // {
        //     return 0;
        // }
        return ret;
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if (NULL == pername || NULL == name)
    {
        return -1;
    }
    //判断双方是否已经是好友
    QString data = QString("select * from friend where (id = (select id from usrInfo where name = \'%1\') and friendId = (select id from usrInfo where name = \'%2\'))"
                           "or (id = (select id from usrInfo where name = \'%3\') and friendId = (select id from usrInfo where name = \'%4\'))").arg(pername).arg(name).arg(name).arg(pername);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if (query.next())
    {
        return 0;   //双方已是好友
    }
    else    //判断是否在线
    {
        data = QString("select online from usrInfo where name=\'%1\'").arg(pername);
        QSqlQuery query;
        query.exec(data);

        if (query.next())
        {
            int ret = query.value(0).toInt();
            if (1 == ret)   //在线
            {
                return 1;
            }
            else if (0 == ret)  //不在线
            {
                return 2;
            }
        }
        else    //不存在这个人
        {
            return 3;
        }
    }
}

void OpeDB::handleAddFriendResult(const char *receiver, const char *applicant)
{
    if (NULL == receiver || NULL == applicant)
    {
        return;
    }

    QString sql = QString("SELECT id FROM usrInfo WHERE name IN (\'%1\', \'%2\')").arg(receiver).arg(applicant);

    QSqlQuery query;
    query.exec(sql);

    int receiverId = -1;
    int applicantId = -1;
    if (query.next()) {
        // 获取第一个ID（假设第一个是receiver）
        receiverId = query.value(0).toInt();
        query.next(); // 移动到下一个结果
        // 获取第二个ID（假设第二个是applicant）
        applicantId = query.value(0).toInt();
    }

    // 确保receiverId和applicantId都已成功获取
    if (receiverId != -1 && applicantId != -1) {
        QString data = QString("INSERT INTO friend(id, friendId) VALUES(\'%1\', \'%2\')").arg(applicantId).arg(receiverId);
        query.exec(data);
    }

    //以下方式不行，暂不知原因，无法进入if中
    // QSqlQuery queryReceiverId;
    // queryReceiverId.exec(receiverIdSql);
    // QSqlQuery queryApplicantId;
    // queryApplicantId.exec(applicantIdSql);
    // qDebug() << receiverIdSql << '\n' << applicantIdSql;

    // if (queryReceiverId.next() && queryReceiverId.next())
    // {
    //     qDebug() << "aaa";
    //     int receiverId = queryReceiverId.value(0).toInt();
    //     int applicantId = queryApplicantId.value(0).toInt();
    //     qDebug() << receiverId << applicantId;
    //     QString data = QString("insert into friend(id, friendId) values(\'%1\', \'%2\')").arg(applicantId).arg(receiverId);
    //     QSqlQuery query;
    //     query.exec(data);
    // }
}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;  //存好友的名字
    strFriendList.clear();
    if (NULL == name)
    {
        return strFriendList;
    }
    //自己作为friendId, 来进行查询其他的id(注意子语句查出的id，是一个集合，所以要用in， 才能完整查出所有好友名字)
    QString data = QString("select name from usrInfo where online=1 and id in (select id from friend where friendId=(select id from usrInfo where name = \'%1\'))").arg(name);
    QSqlQuery query;
    query.exec(data);
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
    }

    //自己作为id, 来进行查询其他的friendId
    data = QString("select name from usrInfo where online=1 and id in (select friendId from friend where id=(select id from usrInfo where name = \'%1\'))").arg(name);
    query.exec(data);
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
    }

    return strFriendList;
}

bool OpeDB::handleDelFrinend(const char *name, const char *friendName)
{
    if (NULL == name || NULL == friendName)
    {
        return false;
    }

    QString data = QString("delete from friend where id=(select id from usrInfo where name=\'%1\') and friendId = (select id from usrInfo where name=\'%2\')").arg(name).arg(friendName);
    QSqlQuery query;
    query.exec(data);

    data = QString("delete from friend where id=(select id from usrInfo where name=\'%1\') and friendId = (select id from usrInfo where name=\'%2\')").arg(friendName).arg(name);
    query.exec(data);

    return true;
}
