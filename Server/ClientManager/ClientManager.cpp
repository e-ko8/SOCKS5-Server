#include "ClientManager.hpp"
#include <iostream>

ClientsManager::ClientsManager(boost::asio::ip::tcp::acceptor& acceptor_, boost::asio::io_context &ctx_): socket{ctx_}, acceptor_ctx{ctx_,acceptor_}
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
        std::cerr << "Connected client with desc " << client_descriptor << "\n";
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
        std::cerr << "Disconnecting client with desc " << desc << "\n";
    }

    if(IsClient(third_party_desc))
    {
        clients.extract(third_party_desc);
        std::cerr << "Disconnecting third party with desc " << third_party_desc << "\n";
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


