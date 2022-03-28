#ifndef _MAJ_CODEC_H
#define _MAJ_CODEC_H

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
// using namespace muduo;
// using namespace muduo::net;

class TailSignEncode : muduo::noncopyable
{
public:
    typedef std::function<void (const muduo::net::TcpConnectionPtr&,
                                const muduo::string &message,
                                muduo::Timestamp)> StringMessageCallback;
    //explicit 关键词，修饰的类不能发生隐式类型转换
    explicit TailSignEncode(){}
    explicit TailSignEncode(const StringMessageCallback &messageCallback):
            messageCallback_(messageCallback)
        {}
   
    //解析数据
    void onMessage(const muduo::net::TcpConnectionPtr &conn,
                    muduo::net::Buffer *buf,
                    muduo::Timestamp receiveTime)
    {
        //LOG_INFO << "message come!";

        while(buf->findCRLF() != nullptr)
        {
            //LOG_DEBUG << "Decoding...";
            const char *data = buf->peek();
            const char *end = buf->findCRLF();
            if(data >= end)
            {
                LOG_ERROR << "Empty information or error";
                buf->retrieveAll();
                messageCallback_(conn, "Invalid message!", receiveTime);
                break;
            }
            muduo::string message(data, static_cast<int>(end - data));
            messageCallback_(conn, message, receiveTime);
            buf->retrieveUntil(end);
            buf->retrieve(2);
        }
    }
    //发送数据
    void send(muduo::net::TcpConnection *conn,
                const muduo::StringPiece &message)
    {
        muduo::net::Buffer buf;
        buf.append(message.data(), message.size());
        buf.append("\r\n");
        conn->send(&buf);
    }
private:

    StringMessageCallback messageCallback_;

};

//const static std::string tailSign_ = "\r\n";

#endif //_MAJ_CODEC_H
