#ifndef _MAJ_ROOM_H
#define _MAJ_ROOM_H
#include "maj_pilegen.h"

#include <muduo/net/TcpConnection.h>
#include <vector>
class MajRoom
{
public:
    MajRoom(){}
    MajRoom(const muduo::net::TcpConnectionPtr& master,const int model, const std::string name)
        :master_(master), model_(model), roomName_(name)
    {
        member_.reserve(4);
        member_.push_back(master);
    }
    ~MajRoom() {}

private:
    muduo::net::TcpConnectionPtr master_;
    int model_;
    std::string roomName_;
    
    
    std::vector<muduo::net::TcpConnectionPtr> member_;
};

#endif //_MAJ_ROOM_H
