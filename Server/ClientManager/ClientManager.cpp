#include "ClientManager.hpp"
#include <iostream>

ClientsManager::ClientsManager(boost::asio::ip::tcp::acceptor& acceptor_, boost::asio::io_context &ctx_, Logger& logger_): socket{ctx_}, acceptor_ctx{ctx_,acceptor_}, logger{logger_}
{

}

void ClientsManager::EraseRoute(u_long from, u_long to)
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

void ClientsManager::AddRoute(u_long from, u_long to)
{
    routes[from] = to;
    routes[to] = from;
}

bool ClientsManager::IsClient(u_long desc)
{
    return clients.count(desc)!=0;
}

u_long ClientsManager::AddClient()
{
    boost::system::error_code error;

    acceptor_ctx.acceptor.accept(socket,error);
    u_long client_descriptor = socket.native_handle();

    if (!error)
    {

        logger.Log("Connected client with ip " + socket.remote_endpoint().address().to_string() + " and desc " + std::to_string(client_descriptor));
        clients[client_descriptor] = std::make_unique<Client>(std::move(socket),acceptor_ctx.ctx);
    }

    return client_descriptor;
}

std::unique_ptr<Client>& ClientsManager::GetClient(u_long desc)
{
    return clients.at(desc);
}

void ClientsManager::DeleteClient(u_long desc)
{
    u_long third_party_desc = GetRoute(desc);

    EraseRoute(desc, third_party_desc);

    if(IsClient(desc))
    {
        clients.extract(desc);
        logger.Log("Disconnecting client with desc " + std::to_string(desc));
    }

    if(IsClient(third_party_desc))
    {
        clients.extract(third_party_desc);
        logger.Log("Disconnecting third party with desc " + std::to_string(third_party_desc));
    }
}

u_long ClientsManager::GetRoute(u_long from)
{
    if(IsRouteExist(from))
    {
        return routes.at(from);
    }

    return -1;
}

bool ClientsManager::IsRouteExist(u_long from)
{
    return routes.count(from)!=0;
}


