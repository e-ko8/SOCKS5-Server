#include "ThreadWorker.hpp"
#include <boost/exception/diagnostic_information.hpp>
#include <iostream>

ThreadWorker::ThreadWorker(CommonObjects& obj) : objects(obj),
clients_manager(obj.listener.acceptor, obj.ctx, obj.logger)
{

}

void ThreadWorker::Work()
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

void ThreadWorker::RemoveClient(u_long desc)
{
    std::lock_guard lock(objects.mutex);
    kqueue_manager.StopAllEventsWaiting(desc);

    u_long third_party_desc = clients_manager.GetRoute(desc);
    kqueue_manager.StopAllEventsWaiting(third_party_desc);

    clients_manager.DeleteClient(desc);
}

void ThreadWorker::ReadEventOccured(const struct kevent &event)
{
    if (clients_manager.IsClient(event.ident))
    {
        kqueue_manager.StopTimer(event.ident);
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

        kqueue_manager.StartTimer(event.ident, 30, TimerCoeffs::MinutesCoeff);

    }

    else
    {
        u_long client_desc = clients_manager.GetRoute(event.ident);
        auto& client = clients_manager.GetClient(client_desc);

        client->ReadFromServer(event.data);
        kqueue_manager.WaitForWriteEvent(client_desc);
    }
}

void ThreadWorker::WriteEventOccured(const struct kevent &event)
{
    if (clients_manager.IsClient(event.ident))
    {
        kqueue_manager.StopTimer(event.ident);

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
        kqueue_manager.StartTimer(event.ident, 30, TimerCoeffs::MinutesCoeff);
    }

    else
    {
        u_long client_desc = clients_manager.GetRoute(event.ident);
        auto& client = clients_manager.GetClient(client_desc);

        client->WriteToServer();
    }
}

void ThreadWorker::AcceptClient()
{
    u_long client_desc = clients_manager.AddClient();

    if(client_desc!=-1)
    {
        kqueue_manager.WaitForReadEvent(client_desc);
        kqueue_manager.StartTimer(client_desc, 30 ,TimerCoeffs::MinutesCoeff);
    }
}

u_long ThreadWorker::GetThreadLoad()
{
    return clients_manager.GetClientsCount();
}

ThreadWorker::ThreadWorker(ThreadWorker &&other) noexcept : objects(other.objects), clients_manager(std::move(other.clients_manager)),
                                                            kqueue_manager(std::move(other.kqueue_manager))
{
    work = other.work;
    other.work = false;
}


