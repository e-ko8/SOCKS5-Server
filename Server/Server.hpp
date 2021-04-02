#pragma once
#include <boost/asio.hpp>
#include <map>

#include "Demultiplexer.hpp"
#include "Client.hpp"

struct ServerParameters
{
    boost::asio::io_context ctx;
    std::uint16_t port;
    std::size_t threads;
};

struct ClientsManager
{
    explicit ClientsManager(boost::asio::io_context& ctx) : socket{ctx} {};
    boost::asio::ip::tcp::socket socket;
    std::map<int, std::unique_ptr<Client>> clients;
    std::map<int,int> routes;
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

    explicit Server(ServerParameters& input);

    [[noreturn]] void StartListening();

private:

    void AcceptClient();

    ServerParameters& server_params;
    ClientsManager clients_manager;
    Listener listener;
    Demultiplexer kqueue_manager;

};


