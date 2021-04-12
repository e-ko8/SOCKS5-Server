#include "Server.hpp"
#include <iostream>
#include <boost/exception/diagnostic_information.hpp>

Server::Server(ServerParameters &input, Logger& logger_, CommonObjects& common) : server_params{input},//, listener{server_params.ctx},
 ThreadWorker(common), objects(common)
{
    boost::asio::ip::tcp::endpoint ep{boost::asio::ip::address::from_string("127.0.0.1"), server_params.port};

    /*listener.acceptor.open(ep.protocol());
    listener.acceptor.bind(ep);
    listener.acceptor.listen();

    listener.acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address{true});
    listener.acceptor.non_blocking(true);

    listener.descriptor = listener.acceptor.native_handle();
    kqueue_manager.WaitForReadEvent(listener.descriptor);*/

    objects.listener.acceptor.open(ep.protocol());
    objects.listener.acceptor.bind(ep);
    objects.listener.acceptor.listen();

    objects.listener.acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address{true});
    objects.listener.acceptor.non_blocking(true);

    objects.listener.descriptor = objects.listener.acceptor.native_handle();
    kqueue_manager.WaitForReadEvent(objects.listener.descriptor);

    for(std::size_t i = 0; i < server_params.threads - 1; i++)
    {
        usual_workers.emplace_back(ThreadWorker{objects});
    }
}

void Server::Work()
{
    std::cerr << "Listening\n";
    while(work)
    {
        auto event = kqueue_manager.WaitEvent();
        if(event.filter == 0) continue;

        try
        {
            if ( event.flags & EV_EOF )
            {
                RemoveClient(event.ident);
                continue;
            }

            if (event.ident == objects.listener.descriptor)
            {
                BalanceNewClient();
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

        catch (boost::exception& exception)
        {
            {
                std::lock_guard lock(objects.mutex);
                objects.logger.Log(boost::diagnostic_information(exception));
            }

            if(event.ident!=objects.listener.descriptor)
            {
                RemoveClient(event.ident);
            }
        }

        catch (std::exception& exception)
        {
            {
                std::lock_guard lock(objects.mutex);
                objects.logger.Log(exception.what());
            }

            if(event.ident!=objects.listener.descriptor)
            {
                RemoveClient(event.ident);
            }
        }
    }
}

void Server::BalanceNewClient()
{
    std::lock_guard lock(objects.mutex);
    u_long min_size = GetThreadLoad();
    int id = kqueue_manager.GetKqueueId();

    std::cerr << id << " = " << min_size << "\n";

    for(auto& w : usual_workers)
    {
        u_long size = w.GetThreadLoad();
        std::cerr << w.kqueue_manager.GetKqueueId() << " = " << size << "\n";
        if(std::min(min_size,size) != min_size)
        {
            min_size = size;
            id = w.kqueue_manager.GetKqueueId();
        }
    }

    std::cerr << "min load " << id << "\n";
    std::cerr << "\n";

    if(id == kqueue_manager.GetKqueueId())
    {
        AcceptClient();
        //std::cerr << "adding client to my que " << kqueue_manager.GetKqueueId() << " new value is " << clients_queue  <<"\n";
        return;
    }

    for(auto& w : usual_workers)
    {
        if(w.kqueue_manager.GetKqueueId() == id)
        {
            u_long client_desc = clients_manager.AddClient();

            if(client_desc!=-1)
            {
                auto node = clients_manager.ExtractClient(client_desc);
                w.TakeClient(node);
                w.kqueue_manager.WaitForReadEvent(client_desc);
            }
            return;
        }
    }

}

void Server::StartListening()
{
    for(auto& w : usual_workers)
    {
        auto task = [&]()
        {
            w.Work();
        };

        threads.emplace_back(std::thread{task});
    }

    objects.logger.Log("Listening...");
    Work();

    for (auto& th : threads)
    {
        th.join();
    }
}

/*void Server::AddClientToQueue()
{

}*/


/* #include "Server.hpp"
#include <iostream>
#include <boost/exception/diagnostic_information.hpp>

Server::Server(ServerParameters &input, Logger& logger_) : server_params{input}, listener{server_params.ctx}, clients_manager{ClientsManager{listener.acceptor,server_params.ctx}},
                                          logger(logger_)
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
    logger.Log("Listening...");
    while(true)
    {
        auto event = kqueue_manager.WaitEvent();

        try
        {
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

        catch (boost::exception& exception)
        {
            logger.Log(boost::diagnostic_information(exception));
            if(event.ident!=listener.descriptor)
            {
                RemoveClient(event.ident);
            }
        }

        catch (std::exception& exception)
        {
            logger.Log(exception.what());
            if(event.ident!=listener.descriptor)
            {
                RemoveClient(event.ident);
            }
        }
    }
}

void Server::AcceptClient()
{
    u_long client_desc = clients_manager.AddClient();

    if(client_desc!=-1)
    {
        kqueue_manager.WaitForReadEvent(client_desc);
    }
}

void Server::RemoveClient(u_long desc)
{
    kqueue_manager.StopAllEventsWaiting(desc);

    u_long third_party_desc = clients_manager.GetRoute(desc);
    kqueue_manager.StopAllEventsWaiting(third_party_desc);

    clients_manager.DeleteClient(desc);
}

void Server::ReadEventOccured(const struct kevent& event)
{
    if (clients_manager.IsClient(event.ident))
    {
        auto& client = clients_manager.GetClient(event.ident);
        client->ReadFromClient(event.data);

        if(!client->IsProtocolPartCompleted())
        {
            kqueue_manager.WaitForWriteEvent(event.ident);
        }

        else
        {

            if(!clients_manager.IsRouteExist(event.ident))
            {
                u_long third_party_desc = client->GetServerDescriptor();
                clients_manager.AddRoute(event.ident, third_party_desc);
                kqueue_manager.WaitForReadEvent(third_party_desc);
            }

            kqueue_manager.WaitForWriteEvent(clients_manager.GetRoute(event.ident));
        }

    }

    else
    {
        u_long client_desc = clients_manager.GetRoute(event.ident);
        auto& client = clients_manager.GetClient(client_desc);

        client->ReadFromServer(event.data);
        kqueue_manager.WaitForWriteEvent(client_desc);
    }
}

void Server::WriteEventOccured(const struct kevent& event)
{
    if (clients_manager.IsClient(event.ident))
    {
        auto& client = clients_manager.GetClient(event.ident);

        if(!client->IsHandshakeCompleted())
        {
            client->CompleteHandshake();
        }

        else if(!client->IsProtocolPartCompleted())
        {
            client->CompleteProtocolPart();
        }

        else
        {
            client->WriteToClient();
        }

        kqueue_manager.StopWriteWaiting(event.ident);
    }

    else
    {
        u_long client_desc = clients_manager.GetRoute(event.ident);
        auto& client = clients_manager.GetClient(client_desc);

        client->WriteToServer();
    }
}
*/