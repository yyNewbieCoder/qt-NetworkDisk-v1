#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    ~OpeDB();
    static OpeDB& getInstance();
    void init();

    bool handleRegist(const char *name, const char *pwd);
    bool handleLogin(const char *name, const char *pwd);
    void handleOffline(const char *name);
    QStringList handAllOnline();
    int handleSearchUsr(const char *name);
    int handleAddFriend(const char *pername, const char *name);
    void handleAddFriendResult(const char *receiver, const char *applicant);
    QStringList handleFlushFriend(const char *name);
    bool handleDelFrinend(const char *name, const char *friendName);

signals:

public slots:
private:
    QSqlDatabase m_db;  //连接数据库
};

#endif // OPEDB_H
