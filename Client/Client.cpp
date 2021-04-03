#include "Client.hpp"

Client::Client(boost::asio::ip::tcp::socket &&sock, boost::asio::io_context &ctx)  : client_socket(std::move(sock)), server_socket(ctx)
{
    client_socket.non_blocking(true);
}

void Client::MakeHandshake()
{
    WriteOnSocket(client_socket,client_buffer);

    if (client_buffer.back() != 0xFF)
    {
        handshake_completed = true;
    }
}

void Client::CheckHandshakeMessage()
{
    if ( client_buffer.size() < 3 )
    {
        client_buffer = {0x05, 0xFF};
        return;
    }

    if ( (client_buffer[0]!=0x05) || (( client_buffer.size() - 2 ) != client_buffer[1]))
    {
        client_buffer = {0x05, 0xFF};
        return;
    }

    for (auto byte = client_buffer.begin() + 2; byte != client_buffer.end(); byte++)
    {
        if (*byte == 0x00)
        {
            client_buffer = {0x05, 0x00};
            return;
        }
    }

    client_buffer = {0x05, 0xFF};
}

void Client::CheckProtocolMessage()
{
    //TODO
}

void Client::StartProtocolPart()
{
    //TODO
}

void Client::ReadFromClient(int volume)
{
    ReadFromSocket(client_socket,client_buffer,volume);

    if(!handshake_completed)
    {
        CheckHandshakeMessage();
    }

    if(!protocol_part_completed)
    {
        CheckProtocolMessage();
    }
}

void Client::ReadFromServer(int volume) {

}

void Client::WriteToClient() {

}

void Client::WriteToServer() {

}

void Client::WriteOnSocket(boost::asio::ip::tcp::socket &to, std::vector<std::uint8_t> &buf)
{
    boost::system::error_code error;

    while (!buf.empty())
    {
        std::size_t bytes_written = to.write_some(boost::asio::buffer(buf), error);

        if (error)
        {
            if (error.value() == boost::system::errc::operation_would_block)
            {
                break;
            }
        }

        buf.erase(buf.begin(), buf.begin() + bytes_written);
    }
}

void Client::ReadFromSocket(boost::asio::ip::tcp::socket &from, std::vector<std::uint8_t> &buf, int volume)
{
    std::size_t iter = buf.size();
    buf.resize(iter + volume);

    boost::system::error_code error;
    while( iter < buf.size() )
    {
        auto bytes_received = from.read_some(boost::asio::buffer(&buf[iter], buf.size()-iter), error);

        if (error)
        {
            if (error.value() == boost::system::errc::operation_would_block) break;
        }

        if (bytes_received == 0)
        {
            break;
        }

        iter += bytes_received;
    }
}

void Client::SendException()
{
    throw std::runtime_error("Client error occured");
}

