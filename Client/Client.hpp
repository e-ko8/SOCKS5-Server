#pragma once
#include <boost/asio.hpp>

class Client
{
public:
    Client(boost::asio::ip::tcp::socket&& sock, boost::asio::io_context& ctx) : client_socket(std::move(sock)), server_socket(ctx)
    {

    }

private:
    boost::asio::ip::tcp::socket client_socket, server_socket;

};


