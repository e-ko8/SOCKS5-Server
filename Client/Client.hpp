#pragma once
#include <boost/asio.hpp>
#include <vector>

class Client
{
public:
    Client(boost::asio::ip::tcp::socket&& sock, boost::asio::io_context& ctx);

    void ReadFromClient(int volume);
    void ReadFromServer(int volume);

    void MakeHandshake();
    void StartProtocolPart();

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
    void CheckHandshakeMessage();
    void CheckProtocolMessage();
    void SendException();
    void ConnectToThirdParty();

    boost::asio::ip::tcp::socket client_socket, server_socket;
    std::vector<std::uint8_t> client_buffer, server_buffer;

    boost::asio::ip::tcp::resolver resolver;

    bool handshake_completed = false;
    bool protocol_part_completed = false;
    bool error_occured = false;

};


