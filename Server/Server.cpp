#include "Server.hpp"
#include <boost/exception/diagnostic_information.hpp>
#include <csignal>

Server* Server::me = nullptr;

Server::Server(ServerParameters server_params, CommonObjects& common) : ThreadWorker(common)
{
    boost::asio::ip::tcp::endpoint ep{boost::asio::ip::address::from_string("127.0.0.1"), server_params.port};

    objects.listener.acceptor.open(ep.protocol());
    objects.listener.acceptor.bind(ep);
    objects.listener.acceptor.listen();

    objects.listener.acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address{true});
    objects.listener.acceptor.non_blocking(true);

    objects.listener.descriptor = objects.listener.acceptor.native_handle();
    kqueue_manager.WaitForReadEvent(objects.listener.descriptor);

    signal_manager.SetHandler(&Server::Interrupt);
    signal_manager.RegisterSignal(SIGINT);
    signal_manager.RegisterSignal(SIGTSTP);
    signal_manager.RegisterSignal(SIGTERM);

    me = this;

    for(std::size_t i = 0; i < server_params.threads - 1; i++)
    {
        workers.emplace_back(ThreadWorker{objects});
    }
}

void Server::Work()
{
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

    for(auto& worker : workers)
    {
        u_long size = worker.GetThreadLoad();
        if(std::min(min_size,size) != min_size)
        {
            min_size = size;
            id = worker.kqueue_manager.GetKqueueId();
        }
    }

    workers_references[id]->AcceptClient();
}

void Server::StartListening()
{
    for(auto& worker : workers)
    {
        auto task = [&]()
        {
            worker.Work();
        };

        workers_references[worker.kqueue_manager.GetKqueueId()] = &worker;
        threads.emplace_back(std::thread{task});
    }

    workers_references[kqueue_manager.GetKqueueId()] = this;

    {
        std::lock_guard lock(objects.mutex);
        objects.logger.Log("Listening...");
    }

    Work();

    for (auto& thread : threads)
    {
        thread.join();
    }
}

void Server::Interrupt(int sig)
{
    std::lock_guard lock(me->objects.mutex);

    me->objects.logger.Log("Server interrupted with signal " + std::to_string(sig));

    for(auto& worker : me->workers)
    {
        worker.StopWork();
    }

    me->StopWork();
}
