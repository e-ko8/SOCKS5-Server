#include "Client.hpp"

Client::Client(boost::asio::ip::tcp::socket &&sock, boost::asio::io_context &ctx)  : client_socket(std::move(sock)), server_socket(ctx)
{
    client_socket.non_blocking(true);
}

void Client::MakeHandshake()
{
    if (client_buffer[1] != 0xFF)
    {
        handshake_completed = true;
    }

    WriteOnSocket(client_socket,client_buffer);
}

void Client::StartProtocolPart() {

}

void Client::ReadFromClient(int volume)
{

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
