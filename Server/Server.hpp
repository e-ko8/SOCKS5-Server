#pragma once
#include <boost/asio.hpp>
#include <map>

#include "Demultiplexer.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "Logger.hpp"

struct ServerParameters
{
    boost::asio::io_context ctx;
    std::uint16_t port;
    std::size_t threads;
};

struct Listener
{
    explicit Listener(boost::asio::io_context& ctx) : acceptor{ctx} {};
    boost::asio::ip::tcp::acceptor acceptor;
    int descriptor = -1;
};

class Server
{

public:

    explicit Server(ServerParameters& input, Logger& logger_);
    Server(Server&& other) = delete;
    Server(const Server& other) = delete;

    Server& operator=(Server&& other) = delete;
    Server& operator=(const Server& other) = delete;

    [[noreturn]] void StartListening();

private:

    void AcceptClient();

    void RemoveClient(u_long desc);

    void ReadEventOccured(const struct kevent& event);

    void WriteEventOccured(const struct kevent& event);

    ServerParameters& server_params;
    ClientsManager clients_manager;
    Listener listener;
    Demultiplexer kqueue_manager;
    Logger& logger;
};


