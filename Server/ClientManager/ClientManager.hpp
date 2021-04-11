#pragma once
#include <boost/asio.hpp>
#include "Client.hpp"
#include <memory>
#include "Logger.hpp"

struct AcceptorCtx
{
    boost::asio::io_context& ctx;
    boost::asio::ip::tcp::acceptor& acceptor;
};

class ClientsManager
{
public:

    explicit ClientsManager(boost::asio::ip::tcp::acceptor& acceptor_, boost::asio::io_context& ctx_, Logger& logger_);
    ClientsManager(ClientsManager&& other) = delete;
    ClientsManager(const ClientsManager& other) = delete;

    ClientsManager& operator=(ClientsManager&& other) = delete;
    ClientsManager& operator=(const ClientsManager& other) = delete;

    void EraseRoute(u_long from, u_long to);

    void AddRoute(u_long from, u_long to);

    [[nodiscard]] bool IsClient(u_long desc);

    u_long AddClient();

    void DeleteClient(u_long desc);

    std::unique_ptr<Client>& GetClient(u_long desc);

    u_long GetRoute(u_long from);

    bool IsRouteExist(u_long from);

private:

    boost::asio::ip::tcp::socket socket;
    std::map<u_long, std::unique_ptr<Client>> clients;
    std::map<u_long,u_long> routes;

    AcceptorCtx acceptor_ctx;
    Logger& logger;
};


