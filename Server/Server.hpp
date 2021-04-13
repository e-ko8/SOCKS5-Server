#pragma once
#include <map>
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
    std::uint16_t port;
    std::size_t threads;
};

class Server : public ThreadWorker
{

public:

    explicit Server(ServerParameters server_params,  CommonObjects& common);
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

    std::vector<ThreadWorker> workers;
    std::vector<std::thread> threads;
    std::map<int, ThreadWorker*> workers_references;
};




