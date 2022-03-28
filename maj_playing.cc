#include "maj_playing.h"

string MajPlaying::start(ConnectionSet connSet, string roomName)
{
    auto res = playingRoom_.find(roomName);
    if(res != playingRoom_.end())
    {
        return "Room is playing.";
    }
    Room room;
    InitRoom(room, connSet, roomName);
    //auto res = playingRoom_.insert(std::pair<string, Room>(roomName, room));
    broadCast(room, "GAMESTART");
    //pool_.run(std::bind(&MajPlaying::dealCard, this, room));
    dealCard(room);
    playingRoom_.insert(std::pair<string, Room>(roomName, room));

    return "STARTSUCCESS";
}

string MajPlaying::playMaj(TcpConnectionPtr &conn, string roomName, string maj)
{
    auto res = playingRoom_.find(roomName);
    if(res == playingRoom_.end())
        return "No such room.";
    
    int no = -1;
    for(int i = 0; i < 4; i++)
    {
        if(res->second.connList[i] == conn)
        {
            no = i;
            break;
        }
    }
    if(no == -1)
        return "Illegal command.";
    if(no != res->second.inTurn)
        return "Not your turn.";

    std::vector<string>::iterator it = res->second.majInHand[no].begin();
    for( ; it != res->second.majInHand[no].end(); it++)
    {
        if(!(*it).compare(maj))
        {
            res->second.majRiver[no].push_back(maj);
            res->second.majInHand[no].erase(it);
            sortCard(res->second.majInHand[no]);
            res->second.inTurn++;
            broadMajRiver(res->second);
            dealNextCard(res->second);
            return "PLAYMAJSUCCESS";
        }
    }

    if(it == res->second.majInHand[no].end())
    {
        return "No such Maj.";
    }

    
    return "Something wrong...";

}

string MajPlaying::getMyMaj(TcpConnectionPtr &conn, string roomName)
{
    auto res = playingRoom_.find(roomName);
    if(res == playingRoom_.end())
        return "No such room.";
    
    auto *room = &(res->second);
    int no = -1;
    for(int i = 0; i < 4; i++)
    {
        if(room->connList[i] == conn)
        {
            no = i;
            break;
        }
    }
    if(no == -1)
        return "Illegal command.";

    encode_.send(get_pointer(room->connList[no]), majInHandtoString(room->majInHand[no]));
    return "GETMAJSUCCESS";
}

string MajPlaying::rongHe(TcpConnectionPtr &conn, string roomName)
{
    auto res = playingRoom_.find(roomName);
    if(res == playingRoom_.end())
        return "No such room.";
    
    auto *room = &(res->second);
    for(int i = 0; i < 4; i++)
    {
        if(room->connList[i] == conn)
        {
            string msg;
            msg.append(getNamefromConn(conn)).append(" announces RongHe. His maj is ")
                .append(majInHandtoString(room->majInHand[i]));
            broadCast(*room, msg);
            gameOver(*room);
            return "RONGHESUCCESS";
        }
    }
    return "Illegal command.";
}

string MajPlaying::endGame(TcpConnectionPtr &conn, string roomName)
{
    auto res = playingRoom_.find(roomName);
    if(res == playingRoom_.end())
        return "No such room.";
    auto *room = &(res->second);
    
    if(room->connList[0] == conn)
    {
        broadCast(*room, "GAMEEND");
        //room->~Room();
        playingRoom_.erase(res);
        return "GAMEENDSUCCESS";
    }else
    return "Illegal command.";
}

void MajPlaying::InitRoom(Room &room, ConnectionSet list, string roomName)
{
    room.connList.clear();
    room.pile.upsetPile();
    room.roomName = roomName;
    room.remainCards = 69;
    room.remainMountCards = 4;
    for(ConnectionSet::iterator it = list.begin(); it != list.end(); ++it)
    {
        room.connList.push_back(*it);
    }
    room.majInHand.resize(4);
    room.majRiver.resize(4);

    int64_t seed = muduo::Timestamp::now().microSecondsSinceEpoch();
    default_random_engine e(seed);
    uniform_int_distribution<int> u(0, 3);
    room.inTurn = u(e);
    room.dealer = room.inTurn;
}

string MajPlaying::majInHandtoString(std::vector<string> majSet)
{
    string res;
    res += "|";
    for (std::vector<string>::iterator it = majSet.begin();
        it != majSet.end();
        ++it)
    {
        res += (*it);
        res.append(" ");
    }
    res += "|";
    return res;
}

void MajPlaying::broadCast(Room &room, string msg)
{
    for (ConnectionVector::iterator it = room.connList.begin();
        it != room.connList.end();
        ++it)
    {
        encode_.send(get_pointer(*it), msg);
    }
}

void MajPlaying::dealCard(Room &room)
{
    for(int i = 0; i < 4; i++)
    {
        if(room.inTurn > 3)
            room.inTurn = 0;
        room.majInHand[room.inTurn].clear();
        room.inTurn++;
    }

    for(int i = 0; i < 12; i++)
    {
        if(room.inTurn > 3)
            room.inTurn = 0;
        for(int j = 0; j < 4; j++)
        {
            room.majInHand[room.inTurn].push_back(room.pile.dealMaj());
        }
        room.inTurn++;
    }

    for(int i = 0; i < 4; i++)
    {
        if(room.inTurn > 3)
            room.inTurn = 0;
        room.majInHand[room.inTurn].push_back(room.pile.dealMaj());
        //std::sort(room.majInHand[room.inTurn].begin(), room.majInHand[room.inTurn].end());
        sortCard(room.majInHand[room.inTurn]);
        room.inTurn++;
    }


    if(room.inTurn > 3)
        room.inTurn = 0;
    room.majInHand[room.inTurn].push_back(room.pile.dealMaj());


    for(int i = 0; i < 4; i++)
    {
        encode_.send(get_pointer(room.connList[i]), majInHandtoString(room.majInHand[i]));
    }
    encode_.send(get_pointer(room.connList[room.inTurn]), "YOURTURN");
}


void MajPlaying::dealNextCard(Room &room)
{
    if(room.remainCards < 1)
    {
        gameOver(room);
        return;
    }
    if(room.inTurn > 3)
        room.inTurn = 0;
    
    if(!room.connList[room.inTurn]->connected())
    {
        playMaj(room.connList[room.inTurn], room.roomName, room.pile.dealMaj());
        room.remainCards--;
        return;
    }
    encode_.send(get_pointer(room.connList[room.inTurn]), "YOURTURN");
    room.majInHand[room.inTurn].push_back(room.pile.dealMaj());
    room.remainCards--;
    encode_.send(get_pointer(room.connList[room.inTurn]), majInHandtoString(room.majInHand[room.inTurn]));
}

void MajPlaying::broadMajRiver(Room &room)
{
    string res;
    for(int i = 0; i < 4; i++)
    {
        res.append(getNamefromConn(room.connList[i])).append(":")
        .append(majInHandtoString(room.majRiver[i])).append("\n");
    }
    broadCast(room, res);
}

void MajPlaying::sortCard(std::vector<string> &maj)
{
    int size = static_cast<int>(maj.size());
    for(int i = size-1; i > 0; i--)
    {
        for(int j = 0; j < i; j++)
        {
            if(static_cast<int>(maj[j][0])+static_cast<int>(maj[j][1])*100
                > static_cast<int>(maj[j+1][0])+static_cast<int>(maj[j+1][1])*100)
            {
                swap(maj[j], maj[j+1]);
            }
        }
    }
}

void MajPlaying::gameOver(Room &room)
{
    broadCast(room, "GAMEOVER");
    //room->~Room();
    playingRoom_.erase(playingRoom_.find(room.roomName));
}

// string MajPlaying::start()
// {
//     if(static_cast<int>(connList_.size()) != 4)
//     {
//         string res;
//         res.append("There is ").append(to_string(connList_.size())).append(" people.");
//         return res;
//     }else
//     {
//         return "STARTSUCCESS";
//     }
// }