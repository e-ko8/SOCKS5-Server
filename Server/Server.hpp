#pragma once
#include <boost/asio.hpp>
#include <map>
#include <vector>

#include "Demultiplexer.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "Logger.hpp"
#include "ThreadWorker.hpp"

struct SignalManager
{
    template<typename HandlerType>
    void SetHandler(HandlerType handler)
    {
        manager.sa_handler = handler;
    }

    void RegisterSignal(int sig_type)
    {
        sigaction(sig_type, &manager, nullptr);
    }

    struct sigaction manager;
};

struct ServerParameters
{
    boost::asio::io_context ctx;
    std::uint16_t port;
    std::size_t threads;
};

class Server : public ThreadWorker
{

public:

    explicit Server(ServerParameters& input, Logger& logger_, CommonObjects& common);
    Server(Server&& other) = delete;
    Server(const Server& other) = delete;

    Server& operator=(Server&& other) = delete;
    Server& operator=(const Server& other) = delete;

    void StartListening();
    static void Interrupt(int sig);

private:

    void Work() override;
    void BalanceNewClient();

    static Server* me;
    SignalManager signal_manager{};

    ServerParameters& server_params;
    std::vector<ThreadWorker> workers;
    std::vector<std::thread> threads;
    std::map<int, ThreadWorker*> workers_references;
};




