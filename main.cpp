#include "Server.hpp"

int main(int argc, char* argv[])
{
    ServerParameters input{.port = 5002, .threads = 1};
    std::string logpath;
    Logger logger(logpath);
    Server proxy(input, logger);
    return 0;
}