#include "Server.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        ServerParameters input{.port = 5000, .threads = 4};
        Listener l{input.ctx};
        std::string logpath = "Logs";
        Logger logger(logpath);
        std::mutex m;
        CommonObjects obj{l, logger,m,input.ctx};

        Server proxy(input, logger,obj);

        proxy.StartListening();
    }

    catch(std::exception& error)
    {
        std::cerr << error.what() << "\n";
    }

    return 0;
}