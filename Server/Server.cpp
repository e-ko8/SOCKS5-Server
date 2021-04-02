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
            //TODO
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
                //TODO
                break;
            }

            case EVFILT_READ:
            {
                //TODO
                break;
            }

            case EVFILT_WRITE:
            {
                //TODO
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
        clients_manager.clients[client_descriptor]= std::make_unique<Client>(std::move(clients_manager.socket),server_params.ctx);
    }
}
