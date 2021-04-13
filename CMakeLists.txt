cmake_minimum_required(VERSION 3.8)
project(SOCKS5-Server)

set(CMAKE_CXX_STANDARD 17)

set(    INCLUDE_PATHS

        Server
        Client
        Demultiplexer
        Server/ClientManager
        Exceptions
        Logger
        Server/ThreadWorker
        ProgrammOptions
        )


set(    SOURCE_FILES

        main.cpp
        Server/Server.cpp
        Demultiplexer/Demultiplexer.cpp
        Client/Client.cpp
        Server/ClientManager/ClientManager.cpp
        Exceptions/SocksError.cpp
        Logger/Logger.cpp
        Server/ThreadWorker/ThreadWorker.cpp
        ProgrammOptions/Starter.cpp
        )


add_executable(SOCKS5-Server ${SOURCE_FILES})

target_include_directories(SOCKS5-Server PUBLIC ${INCLUDE_PATHS})

find_package(Boost 1.71.0 REQUIRED COMPONENTS chrono system thread program_options)
target_link_libraries(SOCKS5-Server PRIVATE Boost::chrono Boost::system Boost::thread Boost::program_options)
