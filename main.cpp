#include "Server.hpp"

int main(int argc, char* argv[])
{
    ServerParameters input{.port = 5006, .threads = 1};
    Server proxy(input);
    return 0;
}