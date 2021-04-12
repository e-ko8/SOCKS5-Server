#pragma once
#include <thread>
#include <mutex>
#include "ClientManager.hpp"
#include "Demultiplexer.hpp"

struct Listener
{
    explicit Listener(boost::asio::io_context& ctx) : acceptor{ctx} {};
    boost::asio::ip::tcp::acceptor acceptor;
    int descriptor = -1;
};

struct CommonObjects
{
    Listener& listener;
    Logger& logger;
    std::mutex& mutex;
    boost::asio::io_context& ctx;
};

class ThreadWorker
{

public:

    ThreadWorker(CommonObjects& obj);
    //ThreadWorker(ThreadWorker&& other) = delete;
    //ThreadWorker(const ThreadWorker& other) = delete;

    //ThreadWorker& operator=(ThreadWorker&& other) = delete;
    //ThreadWorker& operator=(const ThreadWorker& other) = delete;

    virtual void Work();

    CommonObjects objects;

    ClientsManager clients_manager;
    Demultiplexer kqueue_manager;

    bool work = true;

    u_long GetThreadLoad();

    template<typename NodeType>
    void TakeClient(NodeType& node)
   {
       clients_manager.TakeClient(node);
   }

protected:

    void AcceptClient();

    void RemoveClient(u_long desc);

    void ReadEventOccured(const struct kevent& event);

    void WriteEventOccured(const struct kevent& event);

};


