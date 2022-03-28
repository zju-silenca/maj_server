#include "maj_codec.h"
#include "maj_command.h"

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/ThreadLocalSingleton.h>

#include <set>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class MajServer : muduo::noncopyable
{
public:
    MajServer(EventLoop *loop, const InetAddress &address)
            :server_(loop, address, "MajServer"),
            encode_(std::bind(&MajServer::onMessage, this, _1, _2, _3))
    {
        server_.setConnectionCallback(std::bind(&MajServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&TailSignEncode::onMessage, &encode_, _1, _2, _3));

    }
    ~MajServer()
    {
        truncateRoom();
    }
    
    void setThreadNums(int numThreads)
    {
        server_.setThreadNum(numThreads);
    }

    void start()
    {
        server_.setThreadInitCallback(std::bind(&MajServer::threadInit, this, _1));
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        LOG_INFO << conn->peerAddress().toIpPort() << " -> "
            << conn->localAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
        
        if(conn->connected())
        {
            LocalConnections::instance().insert(conn);
            
        }else
        {
            LocalConnections::instance().erase(conn);
            flushLoginList();
        }
    }

    void onMessage(const TcpConnectionPtr& conn,
                const muduo::string &message,
                muduo::Timestamp)
    {
        size_t pos = message.find(':');
        if(pos != string::npos && pos != 0)
        {
            string cmd = message.substr(0, pos);
            string msg = message.substr(pos+1);
            doCmd_.setValue(cmd, msg, conn);
            encode_.send(get_pointer(conn),doCmd_.doCommand());
        }else
        encode_.send(get_pointer(conn), "Invalid command.");
    }

    void distributeMessage(const string& message)
    {
        for (ConnectionList::iterator it = LocalConnections::instance().begin();
            it != LocalConnections::instance().end();
            ++it)
        {
            encode_.send(get_pointer(*it), message);
        }
    }



    void addFunctoLoop(EventLoop::Functor f)
    {
        MutexLockGuard lock(mutex_);
        for(std::set<EventLoop*>::iterator it = loops_.begin();
            it != loops_.end(); it++)
        {
            (*it)->queueInLoop(f);
        }
    }


    void threadInit(EventLoop* loop)
    {
        assert(LocalConnections::pointer() == NULL);
        LocalConnections::instance();
        assert(LocalConnections::pointer() != NULL);
        MutexLockGuard lock(mutex_);
        loops_.insert(loop);
    }
    TcpServer server_;
    TailSignEncode encode_;
    MajCommand doCmd_;
    typedef std::set<TcpConnectionPtr> ConnectionList;
    typedef ThreadLocalSingleton<ConnectionList> LocalConnections;
    
    MutexLock mutex_;
    std::set<EventLoop*> loops_ GUARDED_BY(mutex_);
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 1)
  {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    MajServer server(&loop, serverAddr);
    if (argc > 2)
    {
      server.setThreadNums(atoi(argv[2]));
    }
    server.start();
    loop.loop();
  }
  else
  {
    printf("Usage: %s port [thread_num]\n", argv[0]);
  }
}