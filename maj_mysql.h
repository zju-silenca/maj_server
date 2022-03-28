#ifndef _MAJ_MYSQL_H
#define _MAJ_MYSQL_H

#define DB_NAME "maj_database"
#define DB_USER "maj_admin"
#define DB_PASSWORD "123456"
#define DB_HOST "localhost"
#define DB_PORT 3306

#include <iostream>
#include <cstring>
#include <map>
#include <set>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/md5.h>
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/ThreadLocalSingleton.h>
#include <muduo/base/Mutex.h>



using namespace std;
using namespace muduo;
using namespace muduo::net;

typedef std::set<TcpConnectionPtr> ConnectionSet;
typedef std::vector<TcpConnectionPtr> ConnectionVector;
typedef std::map<TcpConnectionPtr, string> ConnectionMap;
typedef std::map<string, ConnectionSet> RoomMap;

static RoomMap room_connSet_;
static ConnectionMap conn_name_;
void flushLoginList();
void truncateRoom();
bool isLogin(const TcpConnectionPtr &conn);
string getNamefromConn(const TcpConnectionPtr &conn);
ConnectionSet getConnSetfromRoom(string room);


class MajMysql
{
public:
    MajMysql();

    ~MajMysql()
    {
        mysql_close(mysql_);
    }
    
    string getStrMD5(string str);
    bool sqlQuery(const char *query);
    string regist(string name, string password);
    string login(const TcpConnectionPtr &conn, string name, string password);
    string logout(const TcpConnectionPtr &conn);
    string creatRoom(const TcpConnectionPtr &conn, string roomName);
    string getRoomlist();
    string enterRoom(const TcpConnectionPtr &conn, string roomName);
    string exitRoom(const TcpConnectionPtr &conn, string roomName);
    string startGame(const TcpConnectionPtr &conn, string roomName);
    

    
private:
    
    

	MYSQL *mysql_;
    MYSQL_RES *resptr_;
    MYSQL_ROW sqlrow_;
};


#endif //_MAJ_MYSQL_H