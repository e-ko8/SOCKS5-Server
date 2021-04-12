#pragma once
#include <boost/asio.hpp>
#include "Client.hpp"
#include <memory>
#include "Logger.hpp"
#include <iostream>

struct AcceptorCtx
{
    boost::asio::io_context& ctx;
    boost::asio::ip::tcp::acceptor& acceptor;
};

class ClientsManager
{
public:

    explicit ClientsManager(boost::asio::ip::tcp::acceptor& acceptor_, boost::asio::io_context& ctx_, Logger& logger_);
    ClientsManager(ClientsManager&& other) noexcept : acceptor(other.acceptor), ctx(other.ctx), socket(std::move(other.socket)), logger(other.logger)
    {
        routes = std::move(other.routes);
        clients = std::move(other.clients);
    }
    ClientsManager(const ClientsManager& other) = delete;

    ClientsManager& operator=(ClientsManager&& other) = delete;
    /*{
        if(this!=&other)
        {
            routes = std::move(other.routes);
            clients = std::move(other.clients);
            acceptor = std::move(other.acceptor);
            ctx = std::move(other.ctx);

        }

        return *this;
    }*/
    ClientsManager& operator=(const ClientsManager& other) = delete;

    void EraseRoute(u_long from, u_long to);

    void AddRoute(u_long from, u_long to);

    [[nodiscard]] bool IsClient(u_long desc);

    u_long AddClient();

    void DeleteClient(u_long desc);

    std::unique_ptr<Client>& GetClient(u_long desc);

    u_long GetRoute(u_long from);

    bool IsRouteExist(u_long from);

    u_long GetClientsCount();

private:

    boost::asio::ip::tcp::socket socket;
    std::map<u_long, std::unique_ptr<Client>> clients;
    std::map<u_long,u_long> routes;

    boost::asio::io_context& ctx;
    boost::asio::ip::tcp::acceptor& acceptor;
    Logger& logger;
};


