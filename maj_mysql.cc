#include "maj_mysql.h"


MajMysql::MajMysql():mysql_(nullptr), resptr_(nullptr), sqlrow_()
{
    mysql_ = mysql_init(mysql_);
    mysql_ = mysql_real_connect(mysql_, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, DB_PORT, nullptr, 0);
    if(mysql_ != nullptr)
    {
        LOG_INFO << "Maj_database connection success.";
    }
    else
    {
        LOG_ERROR << "Maj_database connection false" ;
    }
}


bool MajMysql::sqlQuery(const char *query)
{
    int result = mysql_query(mysql_, query);
    if(result)
    {
        LOG_ERROR << "Sql query error. Error query: " << query;
        return false;
    }else
    {
        return true;
    }
    
}

string MajMysql::regist(string name, string password)
{
    if(name.empty() || password.empty() || name == "_vacancy")
    {
        return "Invalid name or password.";
    }
    char query[1024];
    if(static_cast<int>(name.size()) > 20)
        return "Name too long!";
    sprintf(query, "SELECT name FROM userInfo WHERE name = '%s'", name.c_str());
    if(!sqlQuery(query))
        return "Database error.";
    resptr_ = mysql_store_result(mysql_);
    sqlrow_ = mysql_fetch_row(resptr_);

    if(sqlrow_ == nullptr)
    {
        mysql_free_result(resptr_);
        memset(query, 0, sizeof(query));
        
		sprintf(query, "INSERT INTO userInfo (name, password) values('%s', '%s')", name.c_str(), getStrMD5(password).c_str());
        if(sqlQuery(query))
        {
            return "REGISTSUCCESS";
        }
        else
        {
            return "Regist error.";
        }
        
    }
    else
    {
        mysql_free_result(resptr_);
        return "Name has been registed!";
    }
}

string MajMysql::login(const TcpConnectionPtr &conn, string name, string password)
{
    if(name.empty() || password.empty())
    {
        return "Invalid name or password.";
    }
    if(conn_name_.find(conn) != conn_name_.end())
    {
        return "You have logged.";
    }
    char query[1024];
    if(static_cast<int>(name.size()) > 20)
        return "Name too long.";
    sprintf(query, "SELECT password FROM userInfo WHERE name = '%s'", name.c_str());
    if(!sqlQuery(query))
        return "";
    resptr_ = mysql_store_result(mysql_);
    sqlrow_ = mysql_fetch_row(resptr_);
    mysql_free_result(resptr_);

    if(sqlrow_ == nullptr)
    {
        return "Please regist first.";
    }
    else
    {
        
        string result;
        result.append(sqlrow_[0]);
        if(getStrMD5(password) == result)
        {
            auto res = conn_name_.insert(pair<TcpConnectionPtr, string>(conn, name));
            if(res.second)
            {
                return "LOGINSUCCESS";
            }else
            {
                return "Account has been logged in.";
            }
            
        }else
        {
            return "Password error.";            
        }

    }
}

string MajMysql::logout(const TcpConnectionPtr &conn)
{
    auto res = conn_name_.find(conn);
    if(res == conn_name_.end())
    {
        return "Please Login.";
    }else
    {
        conn_name_.erase(res);
        return "Logout success.";
    }
}

string MajMysql::creatRoom(const TcpConnectionPtr &conn, string roomName)
{

    if(roomName.empty())
    {
        return "Invalid name.";
    }
    string ownerName = conn_name_.find(conn)->second;
    char query[1024];
    if(static_cast<int>(roomName.size()) > 40)
        return "Name too long.";

    sprintf(query, "SELECT roomname FROM roomList WHERE roomname = '%s' OR owner = '%s'", roomName.c_str(),ownerName.c_str());
    if(!sqlQuery(query))
        return "";
    resptr_ = mysql_store_result(mysql_);
    sqlrow_ = mysql_fetch_row(resptr_);
    mysql_free_result(resptr_);
    if(sqlrow_ == nullptr)
    {
        memset(query, 0, sizeof(query));
        sprintf(query, "INSERT INTO roomList (roomname, owner) values('%s', '%s')", roomName.c_str(), ownerName.c_str());
        if(sqlQuery(query))
        {
            std::set<TcpConnectionPtr> cs;
            cs.insert(conn);
            room_connSet_.insert(pair<string, ConnectionSet>(roomName, cs));
            return "CREATESUCCESS";
        }
        else
        {
            return "Create error.";
        }
    }else
    {
        return "Roomname has existed or you have a room.";
    }
}

string MajMysql::getRoomlist( )
{
    char query[1024];
    sprintf(query, "SELECT roomname FROM roomList");
    if(!sqlQuery(query))
        return "";
    resptr_ = mysql_store_result(mysql_);
    string result;
    result.append("List: ");
    sqlrow_ = mysql_fetch_row(resptr_);
    while(sqlrow_ != nullptr)
    {
        result.append(sqlrow_[0]).append(" ");
        sqlrow_ = mysql_fetch_row(resptr_);
    }
    mysql_free_result(resptr_);
    return result;
}
 
string MajMysql::enterRoom(const TcpConnectionPtr &conn, string roomName)
{
    if(room_connSet_.find(roomName) == room_connSet_.end())
        return "No such room";
    auto res = room_connSet_.find(roomName)->second.find(conn);
    if(res != room_connSet_.find(roomName)->second.end())
        return "You are in room.";

    char query[1024];
    sprintf(query, "SELECT member1,member2,member3 FROM roomList WHERE roomname = '%s'",roomName.c_str());
    if(!sqlQuery(query))
        return "";
    resptr_ = mysql_store_result(mysql_);
    sqlrow_ = mysql_fetch_row(resptr_);
    mysql_free_result(resptr_);
    for(int i = 1; i <= 3; i++)
    {
        if(!strcmp(sqlrow_[i-1], "_vacancy"))
        {
            memset(query, 0, sizeof(query));
            sprintf(query, "UPDATE `roomList` SET `member%s` = '%s' WHERE roomname = '%s'",
                    to_string(i).c_str(), conn_name_.find(conn)->second.c_str(), roomName.c_str());
            if(!sqlQuery(query))
                return "";
            room_connSet_.find(roomName)->second.insert(conn);
            return "ENTERROOMSUCCESS";
        }
    }
    return "Room is full.";
}

string MajMysql::exitRoom(const TcpConnectionPtr &conn, string roomName)
{
    if(room_connSet_.find(roomName) == room_connSet_.end())
        return "No such room";
    char query[1024];
    sprintf(query, "SELECT owner,member1,member2,member3 FROM roomList WHERE roomname = '%s'",roomName.c_str());
    if(!sqlQuery(query))
        return "";
    resptr_ = mysql_store_result(mysql_);
    sqlrow_ = mysql_fetch_row(resptr_);
    mysql_free_result(resptr_);
    if(!strcmp(sqlrow_[0], conn_name_.find(conn)->second.c_str()))
    {
        memset(query, 0, sizeof(query));
        sprintf(query, "DELETE FROM `roomList` WHERE `roomList`.`roomname` = '%s'", roomName.c_str());
        room_connSet_.find(roomName)->second.clear();
        room_connSet_.erase(roomName);
        if(!sqlQuery(query))
            return "";
        return "Delete room success.";
    }
    for(int i = 1; i <= 3; i++)
    {
        if(!strcmp(sqlrow_[i], conn_name_.find(conn)->second.c_str()))
        {
            memset(query, 0, sizeof(query));
            sprintf(query, "UPDATE `roomList` SET member%s = '_vacancy' WHERE roomname = '%s'",
                    to_string(i).c_str(), roomName.c_str());
            if(!sqlQuery(query))
                return "";
            room_connSet_.find(roomName)->second.erase(conn);
            return "EXITROOMSUCCESS";
        }
    }
    return "Exit fail.";
}

string MajMysql::startGame(const TcpConnectionPtr &conn, string roomName)
{
    //if(static_cast<int>(room_connSet_.find(roomName)->second.size()) < 4)
        //return "Number of people is not enough.";
    if(room_connSet_.find(roomName) == room_connSet_.end())
    {
        return "No such room.";
    }
    
    char query[1024];
    sprintf(query, "SELECT owner FROM roomList WHERE roomname = '%s'",roomName.c_str());
    if(!sqlQuery(query))
        return "";
    resptr_ = mysql_store_result(mysql_);
    sqlrow_ = mysql_fetch_row(resptr_);
    mysql_free_result(resptr_);
    
    if(sqlrow_[0] != conn_name_.find(conn)->second)
    {
        return "You are not owner.";
    }

    if(static_cast<int>(room_connSet_.find(roomName)->second.size()) != 4)
    {
        return "Number of people is not enough.";
    }

    {
        return "STARTSUCCESS";
    }


}

string MajMysql::getStrMD5(string str)
{
	unsigned char result[17] = {0};
	unsigned char *res_p = result;
 
	MD5_CTX md5;
 
	MD5_Init (&md5);
	MD5_Update (&md5, str.c_str(), str.size());
	MD5_Final (res_p,&md5);
	char output[128] = {0};
    //十六进制转换
	for(int i = 0; i < MD5_DIGEST_LENGTH; i++) 
	{
		sprintf(&output[i*2], "%02x", res_p[i]);
	}
	return output;
}

bool isLogin(const TcpConnectionPtr &conn)
{
    auto res = conn_name_.find(conn);
    if(res == conn_name_.end())
        return false;
    else
        return true;
}

string getNamefromConn(const TcpConnectionPtr &conn)
{
    auto res = conn_name_.find(conn);
    if(res == conn_name_.end())
    {
        return "_Unknown";
    }
    else
    {
        return res->second;
    }
}

ConnectionSet getConnSetfromRoom(string room)
{
    ConnectionSet emptySet;
    if(room_connSet_.find(room) == room_connSet_.end())
    {
        return emptySet;
    }else
    {
        return room_connSet_.find(room)->second;
    }
}

void flushLoginList()
{
    std::map<TcpConnectionPtr,string>::iterator it;
    int num=0;
    for(it = conn_name_.begin(); it!=conn_name_.end(); )
    {
        if(!it->first->connected())
        {
            conn_name_.erase(it++);
            num++;
        }else
        {
            it++;
        }
        
    }
    LOG_INFO << "Erase "<< num <<" connection(s) from Loginlist.";
}

void truncateRoom()
{
    MajMysql mysql;
    char query[1024];
    sprintf(query, "TRUNCATE TABLE `roomList`");
    if(mysql.sqlQuery(query))
    {
        LOG_INFO << "Truncate all room success.";
    }
}