#ifndef _MAJ_PLAYING_H
#define _MAJ_PLAYING_H
#include "maj_codec.h"
#include "maj_pilegen.h"
#include "maj_mysql.h"

#include <muduo/base/ThreadPool.h>
#include <vector>
#include <set>
using namespace muduo;
using namespace muduo::net;


class MajPlaying
{
public:
    MajPlaying(){
        pool_.start(4);
    }
    ~MajPlaying(){
        pool_.stop();
    }

    string start(ConnectionSet connSet, string roomName);
    string playMaj(TcpConnectionPtr &conn, string roomName, string maj);
    string getMyMaj(TcpConnectionPtr &conn, string roomName);
    string rongHe(TcpConnectionPtr &conn, string roomName);
    string endGame(TcpConnectionPtr &conn, string roomName);
private:
    //typedef std::set<string> cardinHand;
    typedef struct Room
    {
        string roomName;
        std::vector<TcpConnectionPtr> connList;
        std::vector<std::vector<string>> majInHand;
        std::vector<std::vector<string>> majRiver;
        int inTurn;
        int dealer;
        int remainCards;
        int remainMountCards;
        MajPilegen pile;

        ~Room()
        {}
    } Room;
    typedef std::map<string, Room> PlayingRoomMap;

    PlayingRoomMap playingRoom_;
    std::set<TcpConnectionPtr> connSet_;
    TailSignEncode encode_;
    ThreadPool pool_;
    //EventLoop loop_;

    string majInHandtoString(std::vector<string> majSet);
    void broadCast(Room &room, string msg);
    void InitRoom(Room &room, ConnectionSet list, string roomName);
    void dealCard(Room &room);
    void playCard(Room &room, int no, string maj);
    void dealNextCard(Room &room);
    void broadMajRiver(Room &room);
    void sortCard(std::vector<string> &maj);
    void gameOver(Room &room);
    

};


#endif //_MAJ_PLAYING_H