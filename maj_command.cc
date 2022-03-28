#include "maj_command.h"

string MajCommand::doCommand()
{
    if(!isLogin(conn_))
    {
        if(cmd_ != LOGIN && cmd_ != REGIST)
        {
            return "Please login.";
        }
    }
    switch(cmd_)
    {
        case ILLEGAL:
        {
            return "Illegal command.";
            break;
        }
        case LOGIN:
        {
            //msg format [name password]
            size_t pos = msg_.find(' ');
            if(pos != string::npos)
            {
                string name = msg_.substr(0, pos);
                string password = msg_.substr(pos+1);
                return mysql_.login(conn_, name, password);
            }
            break;
        }
        case LOGOUT:
        {
            return mysql_.logout(conn_);
            break;
        }
        case REGIST:
        {
            //msg format [name password]
            size_t pos = msg_.find(' ');
            if(pos != string::npos)
            {
                string name = msg_.substr(0, pos);
                string password = msg_.substr(pos+1);
                return mysql_.regist(name, password);
            }
            break;
        }
        case CREATE:
        {
            //msg format [roomname]
            return mysql_.creatRoom(conn_, msg_);
            break;
        }
        case GETROM:
        {
            //msg format [ ]
            return mysql_.getRoomlist();
            break;
        }
        case ENTROM:
        {
            //msg format [roomname]
            return mysql_.enterRoom(conn_, msg_);
            break;
        }
        case EXTROM:
        {
            //msg format [roomname]
            return mysql_.exitRoom(conn_,msg_);
            break;
        }
        case START:
        {
            //msg format [roomname]
            string res = mysql_.startGame(conn_,msg_);
            if(res == "STARTSUCCESS")
            {
                return play_.start(getConnSetfromRoom(msg_), msg_);                
            }else
            return res;

            break;
        }
        case PLAYMAJ:
        {
            //msg format [roomname majname]
            size_t pos = msg_.find(' ');
            if(pos != string::npos)
            {
                string name = msg_.substr(0, pos);
                string maj = msg_.substr(pos+1);
                return play_.playMaj(conn_, name, maj);
            }
            break;
        }
        case GETMAJ:
        {
            //msg format [roomname]
            return play_.getMyMaj(conn_, msg_);
            break;
        }
        case ENDGAME:
        {
            //msg format [roomname]
            return play_.endGame(conn_, msg_);
            break;
        }
        case RONGHE:
        {
            //msg format [roomname]
            return play_.rongHe(conn_, msg_);
            break;
        }
    }
    return "Illegal command.";
}