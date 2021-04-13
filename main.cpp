#include "Server.hpp"
#include "Starter.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        Starter starter("SOCKS5-Server");
        starter.AddArgument("help", "Arguments description");
        starter.AddRequiredArgument<std::uint16_t>("port","Listening port");
        starter.AddRequiredArgument<std::size_t>("threads","Threads number");
        starter.AddArgument("logpath","Logger path", std::string{});

        starter.ParseArguments(argc,argv);

        if(!starter.Failed())
        {

            ServerParameters input{};
            starter.GetArgValue("port", input.port);
            starter.GetArgValue("threads", input.threads);

            boost::asio::io_context ctx;
            std::mutex mutex;
            Listener listener{ctx};

            std::string logpath;
            starter.GetArgValue("logpath", logpath);
            Logger logger{logpath};

            CommonObjects obj{listener, logger,mutex, ctx};

            Server proxy(input, obj);
            proxy.StartListening();
        }
    }

    catch(std::exception& error)
    {
        std::cerr << error.what() << "\n";
    }

    return 0;
}