#include "Server.hpp"

Server::Server(ServerParameters &input) : server_params{input}, listener{server_params.ctx}, clients_manager{server_params.ctx}
{
    boost::asio::ip::tcp::endpoint ep{boost::asio::ip::address::from_string("127.0.0.1"), server_params.port};

    listener.acceptor.open(ep.protocol());
    listener.acceptor.bind(ep);
    listener.acceptor.listen();

    listener.acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address{true});
    listener.acceptor.non_blocking(true);

    listener.descriptor = listener.acceptor.native_handle();
    kqueue_manager.WaitForReadEvent(listener.descriptor);

    StartListening();
}

[[noreturn]] void Server::StartListening()
{
    while(true)
    {
        auto event = kqueue_manager.WaitEvent();

        if ( event.flags & EV_EOF )
        {
            RemoveClient(event.ident);
            continue;
        }

        if (event.ident == listener.descriptor)
        {
            AcceptClient();
            continue;
        }

        switch(event.filter)
        {
            case EVFILT_TIMER:
            {
                RemoveClient(event.ident);
                break;
            }

            case EVFILT_READ:
            {
                ReadEventOccured(event);
                break;
            }

            case EVFILT_WRITE:
            {
                WriteEventOccured(event);
                break;
            }

            default: break;
        }

    }
}

void Server::AcceptClient()
{
    boost::system::error_code error;
    listener.acceptor.accept(clients_manager.socket,error);

    if (!error)
    {
        int client_descriptor = clients_manager.socket.native_handle();
        kqueue_manager.WaitForReadEvent(client_descriptor);
        clients_manager.clients[client_descriptor] = std::make_unique<Client>(std::move(clients_manager.socket),server_params.ctx);
    }
}

void Server::RemoveClient(int desc)
{
    kqueue_manager.StopAllEventsWaiting(desc);

    int third_party_desc = clients_manager.routes[desc];
    kqueue_manager.StopAllEventsWaiting(third_party_desc);

    clients_manager.EraseRoute(desc, third_party_desc);

    if(clients_manager.clients.count(desc)!=0)
    {
        clients_manager.clients.extract(desc);
    }
}

void Server::ReadEventOccured(const struct kevent& event)
{
    if (clients_manager.clients.count(event.ident)!=0)
    {
        auto& client = clients_manager.clients[event.ident];
        client->ReadFromClient(event.data);

        if(!client->IsProtocolPartCompleted())
        {
            kqueue_manager.WaitForWriteEvent(event.ident);
        }

        else
        {

            if(clients_manager.routes.count(event.ident) == 0)
            {
                int third_party_desc = client->GetServerDescriptor();
                clients_manager.AddRoute(event.ident, third_party_desc);
                kqueue_manager.WaitForReadEvent(third_party_desc);
            }

            kqueue_manager.WaitForWriteEvent(clients_manager.routes[event.ident]);
        }

    }

    else
    {
        int client_desc = clients_manager.routes[event.ident];
        auto& client = clients_manager.clients[client_desc];

        client->ReadFromServer(event.data);
        kqueue_manager.WaitForWriteEvent(client_desc);
    }
}

void Server::WriteEventOccured(const struct kevent& event)
{
    if (clients_manager.clients.count(event.ident)!=0)
    {
        auto& client = clients_manager.clients[event.ident];

        if(!client->IsHandshakeCompleted())
        {
            client->MakeHandshake();
        }

        else if(!client->IsProtocolPartCompleted())
        {
            client->StartProtocolPart();
        }

        else
        {
            client->WriteToClient();
        }

        kqueue_manager.StopWriteWaiting(event.ident);
    }

    else
    {
        int client_desc = clients_manager.routes[event.ident];
        auto& client = clients_manager.clients[client_desc];

        client->WriteToServer();
    }
}
