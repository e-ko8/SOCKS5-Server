#include "ClientManager.hpp"
#include <iostream>

ClientsManager::ClientsManager(boost::asio::ip::tcp::acceptor& acceptor_, boost::asio::io_context &ctx_): socket{ctx_}, acceptor_ctx{ctx_,acceptor_}
{

}

void ClientsManager::EraseRoute(int from, int to)
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

void ClientsManager::AddRoute(int from, int to)
{
    routes[from] = to;
    routes[to] = from;
}

bool ClientsManager::IsClient(int desc)
{
    return clients.count(desc)!=0;
}

int ClientsManager::AddClient()
{
    boost::system::error_code error;

    acceptor_ctx.acceptor.accept(socket,error);
    int client_descriptor = socket.native_handle();

    if (!error)
    {
        std::cerr << "Connected client with desc " << client_descriptor << "\n";
        clients[client_descriptor] = std::make_unique<Client>(std::move(socket),acceptor_ctx.ctx);
    }

    return client_descriptor;
}

std::unique_ptr<Client>& ClientsManager::GetClient(int desc)
{
    return clients.at(desc);
}

void ClientsManager::DeleteClient(int desc)
{
    int third_party_desc = GetRoute(desc);

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

int ClientsManager::GetRoute(int from)
{
    if(IsRouteExist(from))
    {
        return routes.at(from);
    }

    return -1;
}

bool ClientsManager::IsRouteExist(int from)
{
    return routes.count(from)!=0;
}


