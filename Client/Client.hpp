#pragma once
#include <boost/asio.hpp>
#include <vector>
#include "SocksError.hpp"

struct ConnectionInfo
{
    std::uint8_t code;
    int connection_type;
    std::vector<std::uint8_t> target;
    std::uint16_t port;
};

class Client
{
public:
    Client(boost::asio::ip::tcp::socket&& sock, boost::asio::io_context& ctx);
    Client(Client&& other) noexcept;
    Client(const Client& other) = delete;

    Client& operator=(Client&& other) noexcept;
    Client& operator=(const Client& other) = delete;

    void ReadFromClient(u_long volume);
    void ReadFromServer(u_long volume);

    void CompleteHandshake();
    void CompleteProtocolPart();

    void WriteToClient();
    void WriteToServer();

    [[nodiscard]] bool IsHandshakeCompleted() const
    {
        return handshake_completed;
    }

    [[nodiscard]] bool IsProtocolPartCompleted() const
    {
        return protocol_part_completed;
    }

    [[nodiscard]] u_long GetServerDescriptor()
    {
        return server_socket.native_handle();
    }

private:

    static void WriteOnSocket(boost::asio::ip::tcp::socket& to, std::vector<std::uint8_t>& buf);
    static void ReadFromSocket(boost::asio::ip::tcp::socket& from, std::vector<std::uint8_t>& buf, u_long volume);
    SocksError CheckHandshakeMessage();
    SocksError CheckProtocolMessage();
    void FormProtoAnswer(const ConnectionInfo& connection_info);
    void RaiseException(std::string msg);
    void ConnectToThirdParty();
    static void FormConnectionCode(ConnectionInfo& connection_info, boost::system::error_code& error);

    boost::asio::ip::tcp::socket client_socket, server_socket;
    std::vector<std::uint8_t> client_buffer, server_buffer;

    boost::asio::ip::tcp::resolver resolver;

    bool handshake_completed = false;
    bool protocol_part_completed = false;
    SocksError socks_error;

};


