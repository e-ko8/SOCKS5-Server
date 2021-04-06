#include "Client.hpp"
#include <boost/endian.hpp>

Client::Client(boost::asio::ip::tcp::socket &&sock, boost::asio::io_context &ctx)  : client_socket(std::move(sock)), server_socket(ctx), resolver(ctx)
{
    client_socket.non_blocking(true);
}

void Client::CompleteHandshake()
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
        error_occured = true;
        return;
    }

    if ( (client_buffer[0]!=0x05) || (( client_buffer.size() - 2 ) != client_buffer[1]))
    {
        client_buffer = {0x05, 0xFF};
        error_occured = true;
        return;
    }

    for (auto byte = client_buffer.begin() + 2; byte != client_buffer.end(); byte++)
    {
        if (*byte == 0x00)
        {
            client_buffer = {0x05, 0x00};
            error_occured = false;
            return;
        }
    }

    client_buffer = {0x05, 0xFF};
    error_occured = true;
}

void Client::CheckProtocolMessage()
{
    if ( client_buffer.size() < 5 )
    {
        error_occured = true;
        return;
    }

    if ( client_buffer[1] != 0x01 )
    {
        error_occured = true;
        return;
    }

    if ( client_buffer[0]!=0x05 || client_buffer[2]!=0x00 )
    {
        error_occured = true;
        return;
    }


    switch (client_buffer[3])
    {
        case 0x01 :
        {
            if (( client_buffer.size() - 4) != 6)
            {
                error_occured = true;
                return;
            }

            break;
        }

        case 0x03 :
        {
            if (( client_buffer.size() - 4) != client_buffer[4] + 3)
            {
                error_occured = true;
                return;
            }

            break;
        }

        case 0x04 :
        {
            if (( client_buffer.size() - 4) != 18)
            {
                error_occured = true;
                return;
            }

            break;
        }

        default:
        {
            error_occured = true;
            return;
        }
    }

    error_occured = false;
}

void Client::CompleteProtocolPart()
{
    if(error_occured)
    {
        //TODO
    }

    else
    {
        ConnectToThirdParty();
        WriteOnSocket(client_socket, client_buffer);
        protocol_part_completed = true;
    }
}

void Client::ConnectToThirdParty()
{
    auto MakeAnswer = [&](std::uint8_t code, int connection_type, std::vector<std::uint8_t> target, std::uint16_t port)
    {
        client_buffer = {0x05, code, 0x00};

        switch(connection_type)
        {
            case 0x01:
            {
                client_buffer.push_back(0x01);
                client_buffer.insert(client_buffer.end(), target.begin(), target.end());
                break;
            }

            case (0x03):
            {
                client_buffer.push_back(0x03);
                client_buffer.push_back(target.size());
                client_buffer.insert(client_buffer.end(), target.begin(), target.end());
                break;
            }

            case 0x04:
            {
                client_buffer.push_back(0x04);
                client_buffer.insert(client_buffer.end(), target.begin(), target.end());
                break;
            }

            default:
            {
                break;
            }
        }

        client_buffer.push_back(boost::endian::native_to_big(port) & 0xFF);
        client_buffer.push_back(boost::endian::native_to_big(port) >> 8);
    };

    switch (client_buffer[3])
    {
        case 0x01:
        {
            std::array<std::uint8_t,4> ip = {};
            std::copy(client_buffer.begin()+4, client_buffer.end()-2, ip.begin());

            std::uint16_t port = 0;
            std::memmove(&port, client_buffer.data()+(client_buffer.size()-2), 2);
            port = boost::endian::big_to_native(port);

            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4(ip), port);

            boost::system::error_code error;
            server_socket.connect(endpoint, error);
            if(!error)
            {
                server_socket.non_blocking(true);
                MakeAnswer(0x00,0x01, {ip.begin(), ip.end()}, port);
            }

            else
            {
                MakeAnswer(0x05,0x01, {ip.begin(), ip.end()}, port);
            }
            break;
        }

        case 0x03:
        {
            std::string domain = {client_buffer.begin()+5, client_buffer.end()-2};
            std::uint16_t port = 0;

            std::memmove(&port, client_buffer.data()+(client_buffer.size()-2), 2);
            port = boost::endian::big_to_native(port);

            auto it = resolver.resolve(boost::asio::ip::tcp::resolver::query(domain, std::to_string(port)));

            boost::system::error_code error;
            server_socket.connect(it->endpoint(),error);

            if(!error)
            {
                server_socket.non_blocking(true);
                MakeAnswer(0x00,0x03, {domain.begin(), domain.end()}, port);
            }

            else
            {
                MakeAnswer(0x05,0x03, {domain.begin(), domain.end()}, port);
            }

            break;
        }

        case 0x04:
        {
            std::array<std::uint8_t,16> ip = {};
            std::copy(client_buffer.begin()+4, client_buffer.end()-2, ip.begin());

            std::uint16_t port = 0;
            std::memmove(&port, client_buffer.data()+(client_buffer.size()-2), 2);
            port = boost::endian::big_to_native(port);

            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v6(ip), port);

            boost::system::error_code error;
            server_socket.connect(endpoint,error);

            if(!error)
            {
                server_socket.non_blocking(true);
                MakeAnswer(0x00,0x04, {ip.begin(), ip.end()}, port);
            }

            else
            {
                MakeAnswer(0x05,0x04, {ip.begin(), ip.end()}, port);
            }
            break;
        }

    }
}

void Client::ReadFromClient(int volume)
{
    ReadFromSocket(client_socket,client_buffer,volume);

    if(!handshake_completed)
    {
        CheckHandshakeMessage();
        return;
    }

    if(!protocol_part_completed)
    {
        CheckProtocolMessage();
        return;
    }
}

void Client::ReadFromServer(int volume)
{
    ReadFromSocket(server_socket,server_buffer,volume);
}

void Client::WriteToClient()
{
    WriteOnSocket(client_socket,server_buffer);
}

void Client::WriteToServer()
{
    WriteOnSocket(server_socket,client_buffer);
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

void Client::RaiseException(std::string msg)
{
    throw std::runtime_error("Client" + std::to_string(client_socket.native_handle()) + "error occured:" + msg);
}

