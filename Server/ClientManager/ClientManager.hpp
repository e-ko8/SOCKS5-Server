#pragma once
#include <boost/asio.hpp>
#include "Client.hpp"
#include <memory>

struct AcceptorCtx
{
    boost::asio::io_context& ctx;
    boost::asio::ip::tcp::acceptor& acceptor;
};

class ClientsManager
{
public:

    explicit ClientsManager(boost::asio::ip::tcp::acceptor& acceptor_, boost::asio::io_context& ctx_);

    void EraseRoute(int from, int to);

    void AddRoute(int from, int to);

    [[nodiscard]] bool IsClient(int desc);

    int AddClient();

    void DeleteClient(int desc);

    std::unique_ptr<Client>& GetClient(int desc);

    int GetRoute(int from);

    bool IsRouteExist(int from);

private:

    boost::asio::ip::tcp::socket socket;
    std::map<int, std::unique_ptr<Client>> clients;
    std::map<int,int> routes;

    AcceptorCtx acceptor_ctx;

};


