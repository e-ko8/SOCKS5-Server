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
    void EraseRoute(int from, int to)
    {
        if (routes.count(from)!=0)
        {
            routes.extract(from);
        }

        if(routes.count(to)!=0)
        {
            routes.extract(to);
        }
    }

    void AddRoute(int from, int to)
    {
        routes[from] = to;
        routes[to] = from;
    }

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

    void RemoveClient(int desc);

    void ReadEventOccured(const struct kevent& event);

    void WriteEventOccured(const struct kevent& event);

    ServerParameters& server_params;
    ClientsManager clients_manager;
    Listener listener;
    Demultiplexer kqueue_manager;

};


