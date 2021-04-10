#pragma once
#include <boost/asio.hpp>
#include <vector>

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

    void ReadFromClient(int volume);
    void ReadFromServer(int volume);

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

    [[nodiscard]] int GetServerDescriptor()
    {
        return server_socket.native_handle();
    }

private:

    void WriteOnSocket(boost::asio::ip::tcp::socket& to, std::vector<std::uint8_t>& buf);
    void ReadFromSocket(boost::asio::ip::tcp::socket& from, std::vector<std::uint8_t>& buf, int volume);
    SocksError CheckHandshakeMessage();
    SocksError CheckProtocolMessage();
    void FormProtoAnswer(const ConnectionInfo& connection_info);
    void RaiseException(std::string msg);
    void ConnectToThirdParty();
    void FormConnectionCode(ConnectionInfo& connection_info, boost::system::error_code& error);

    boost::asio::ip::tcp::socket client_socket, server_socket;
    std::vector<std::uint8_t> client_buffer, server_buffer;

    boost::asio::ip::tcp::resolver resolver;

    bool handshake_completed = false;
    bool protocol_part_completed = false;
    SocksError socks_error;

};


