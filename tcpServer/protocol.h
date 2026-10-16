#ifndef PROTOCOL_H
#define PROTOCOL_H
typedef unsigned int uint;
#define NAME_MAX_LEN 32
#define PWD_MAX_LEN 32
#define FILE_MAX_LEN 32

#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed : name exited"
#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed : name/pwd error or relogin"

#define SEARCH_USR_NO "no such person"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOW_ERROR "unknow error"
#define EXIST_FRIEND "friend exist"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NO_EXIST "usr no exist"
#define ADD_FRIEND_SUCCESS "add friend success"
#define ADD_FRIEND_FAILED "add friend failed"
#define DEL_FRIEND_OK "delete friend ok"

#define DIR_NO_EXIST "dir no exist"
#define FILE_NAME_EXIST "file name exist"
#define CREATE_DIR_OK "create dir ok"
#define DEL_DIR_OK "delete dir ok"
#define DEL_DIR_FAILED "delete dir failed: is a reguler file"
#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILED "rename file failed"
#define ENTER_DIR_OK "enter dir ok"
#define ENTER_DIR_FAILED "enter dir failed: is a reguler file"

#define UPLOAD_FILE_OK  "upload file ok"
#define UPLOAD_FILE_FAILED "upload file failed"
#define DEL_FILE_OK  "delete file ok"
#define DEL_FILE_FAILED "delete file failed: is directory"
#define MOVE_FILE_OK  "move file ok"
#define MOVE_FILE_FAILED "move file failed: is directory"

#define COMMON_ERR "operate failed: system is busy"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,
    //注册
    ENUM_MSG_TYPE_REGIST_REQUEST,
    ENUM_MSG_TYPE_REGIST_RESPOND,
    //登录
    ENUM_MSG_TYPE_LOGIN_REQUEST,
    ENUM_MSG_TYPE_LOGIN_RESPOND,
    //在线用户
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,
    //搜索用户
    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,
    //加好友
    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,
    //加好友结果
    ENUM_MSG_TYPE_ADD_FRIEND_AGREE,
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,
    //刷新好友
    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,
    //删除好友
    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,
    //私聊
    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,
    //群聊
    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,
    //创建文件夹（目录）
    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,
    //刷新（查看）文件
    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,
    //删除文件夹（目录）
    ENUM_MSG_TYPE_DEL_DIR_REQUEST,
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,
    //重命名文件
    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,
    //进入文件夹（目录）
    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,
    //上传文件
    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,
    //删除文件
    ENUM_MSG_TYPE_DELETE_FILE_REQUEST,
    ENUM_MSG_TYPE_DELETE_FILE_RESPOND,
    //下载文件
    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,
    //共享文件
    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,
    //共享文件通知
    ENUM_MSG_TYPE_SHARE_FILE_NOTIFY_REQUEST,
    ENUM_MSG_TYPE_SHARE_FILE_NOTIFY_RESPOND,
    //移动文件
    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,
    //注销
    ENUM_MSG_TYPE_LOGOUT_REQUEST,
    ENUM_MSG_TYPE_LOGOUT_RESPOND,
    ENUM_MSG_TYPE_MAX=0xffffffff,
};

struct FileInfo
{
    char caFileName[FILE_MAX_LEN];    //文件名
    int  iFileType;                   //文件类型
};

struct PDU
{
    uint uiPDULen;
    uint uiMsgType;
    char caData[64];
    uint uiMsgLen;
    int caMSG[];
};

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
