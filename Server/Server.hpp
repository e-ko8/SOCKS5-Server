#pragma once
#include <boost/asio.hpp>
#include <map>
#include <vector>

#include "Demultiplexer.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "Logger.hpp"
#include "ThreadWorker.hpp"

struct ServerParameters
{
    boost::asio::io_context ctx;
    std::uint16_t port;
    std::size_t threads;
};

/*struct Listener
{
    explicit Listener(boost::asio::io_context& ctx) : acceptor{ctx} {};
    boost::asio::ip::tcp::acceptor acceptor;
    int descriptor = -1;
};*/

class Server : public ThreadWorker
{

public:

    explicit Server(ServerParameters& input, Logger& logger_, CommonObjects& common);
    Server(Server&& other) = delete;
    Server(const Server& other) = delete;

    Server& operator=(Server&& other) = delete;
    Server& operator=(const Server& other) = delete;

    virtual void Work() override;
    void BalanceNewClient();
    void StartListening();

private:

    ServerParameters& server_params;
    std::vector<ThreadWorker> usual_workers;
    CommonObjects& objects;
    std::vector<std::thread> threads;
};


