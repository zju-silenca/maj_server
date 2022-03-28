#ifndef _MAJ_COMMAND_H
#define _MAJ_COMMAND_H
#include "maj_mysql.h"
#include "maj_playing.h"
using namespace std;

enum COMMAND{
    ILLEGAL,
    LOGIN,
    LOGOUT,
    REGIST,
    CREATE,
    GETROM,
    ENTROM,
    EXTROM,
    START,
    PLAYMAJ,
    GETMAJ,
    RONGHE,
    ENDGAME
};


class MajCommand
{
public:
    MajCommand(){}
    MajCommand(string cmd, string msg, const TcpConnectionPtr& conn)
    {
        setValue(cmd,msg,conn);
    }
    string doCommand();
    
    void setValue(string cmd, string msg, const TcpConnectionPtr& conn)
    {
        msg_ = msg;
        conn_ = conn;
        cmd_ = ILLEGAL;
        if(cmd == "LOGIN") cmd_ = LOGIN;
        else if(cmd == "LOGOUT") cmd_ = LOGOUT;
        else if(cmd == "REGIST") cmd_ = REGIST;
        else if(cmd == "CREATE") cmd_ = CREATE;
        else if(cmd == "GETROM") cmd_ = GETROM;
        else if(cmd == "ENTROM") cmd_ = ENTROM;
        else if(cmd == "EXTROM") cmd_ = EXTROM;
        else if(cmd == "START") cmd_ = START;
        else if(cmd == "PLAYMAJ") cmd_ = PLAYMAJ;
        else if(cmd == "GETMAJ") cmd_ = GETMAJ;
        else if(cmd == "ENDGAME") cmd_ = ENDGAME;
        else if(cmd == "RONGHE") cmd_ = RONGHE;
    }
private:
    COMMAND cmd_;
    string msg_;
    TcpConnectionPtr conn_;
    MajMysql mysql_;
    MajPlaying play_;
};

#endif //_MAJ_COMMAND_H