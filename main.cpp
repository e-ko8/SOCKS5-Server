#include "Server.hpp"

int main(int argc, char* argv[])
{
    ServerParameters input{.port = 5000, .threads = 1};
    Server proxy(input);
    return 0;
}