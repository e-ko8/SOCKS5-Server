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

SocksError Client::CheckHandshakeMessage()
{
    if ( client_buffer.size() < 3 )
    {
        return {ErrorType::MessageNotFull, "Message is not completed"};
    }

    if ( (client_buffer[0]!=0x05) || (( client_buffer.size() - 2 ) != client_buffer[1]))
    {
        client_buffer = {0x05, 0xFF};
        return {ErrorType::InvalidRequest, "Invalid handshake"};
    }

    for (auto byte = client_buffer.begin() + 2; byte != client_buffer.end(); byte++)
    {
        if (*byte == 0x00)
        {
            client_buffer = {0x05, 0x00};
            return {};
        }
    }

    client_buffer = {0x05, 0xFF};
    return {ErrorType::NoSuchMethod,"No such auth method"};
}

SocksError Client::CheckProtocolMessage()
{
    if ( client_buffer.size() < 5 )
    {
        return {ErrorType::MessageNotFull, "Message is not completed"};
    }

    if ( client_buffer[0]!=0x05 || client_buffer[1] != 0x01 || client_buffer[2]!=0x00)
    {
        return {ErrorType::InvalidRequest, "Invalid protocol request"};
    }

    switch (client_buffer[3])
    {
        case 0x01 :
        {
            if (( client_buffer.size() - 4) != 6)
            {
                return {ErrorType::MessageNotFull, "Message is not completed"};
            }

            break;
        }

        case 0x03 :
        {
            if (( client_buffer.size() - 4) != client_buffer[4] + 3)
            {
                return {ErrorType::MessageNotFull, "Message is not completed"};
            }

            break;
        }

        case 0x04 :
        {
            if (( client_buffer.size() - 4) != 18)
            {
                return {ErrorType::MessageNotFull, "Message is not completed"};
            }

            break;
        }

        default:
        {
            return {ErrorType::NoSuchMethod,"No such connection method"};
        }
    }

    return {};
}

void Client::CompleteProtocolPart()
{
    if(socks_error)
    {
        RaiseException(socks_error.Message());
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
    ConnectionInfo connection_info;
    boost::system::error_code error;

    switch (client_buffer[3])
    {
        case 0x01:
        {
            connection_info.connection_type = 0x01;

            std::array<std::uint8_t,4> ip = {};
            std::copy(client_buffer.begin()+4, client_buffer.end()-2, ip.begin());
            connection_info.target = {ip.begin(), ip.end()};

            connection_info.port = 0;
            std::memmove(&connection_info.port, client_buffer.data()+(client_buffer.size()-2), 2);
            connection_info.port = boost::endian::big_to_native(connection_info.port);

            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4(ip), connection_info.port);

            server_socket.connect(endpoint, error);

            if(!error)
            {
                server_socket.non_blocking(true);
            }

            FormConnectionCode(connection_info,error);
            FormProtoAnswer(connection_info);

            break;
        }

        case 0x03:
        {
            connection_info.connection_type = 0x03;

            std::string domain = {client_buffer.begin()+5, client_buffer.end()-2};

            connection_info.target = {domain.begin(), domain.end()};
            connection_info.port = 0;

            std::memmove(&connection_info.port, client_buffer.data()+(client_buffer.size()-2), 2);
            connection_info.port = boost::endian::big_to_native(connection_info.port);

            auto it = resolver.resolve(boost::asio::ip::tcp::resolver::query(domain, std::to_string(connection_info.port)));

            server_socket.connect(it->endpoint(),error);

            if(!error)
            {
                server_socket.non_blocking(true);
            }

            FormConnectionCode(connection_info,error);
            FormProtoAnswer(connection_info);

            break;
        }

        case 0x04:
        {
            connection_info.code = 0x04;

            std::array<std::uint8_t,16> ip = {};
            std::copy(client_buffer.begin()+4, client_buffer.end()-2, ip.begin());

            connection_info.target = {ip.begin(), ip.end()};

            connection_info.port = 0;
            std::memmove(&connection_info.port, client_buffer.data()+(client_buffer.size()-2), 2);
            connection_info.port = boost::endian::big_to_native(connection_info.port);

            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v6(ip), connection_info.port);

            server_socket.connect(endpoint,error);

            if(!error)
            {
                server_socket.non_blocking(true);
            }

            FormConnectionCode(connection_info,error);
            FormProtoAnswer(connection_info);

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
    throw std::runtime_error("Client" + std::to_string(client_socket.native_handle()) + "error occured: " + msg);
}

void Client::FormConnectionCode(ConnectionInfo &connection_info, boost::system::error_code &error)
{
    switch (error.value())
    {
        case boost::system::errc::success:
        {
            connection_info.code = 0x00;
            break;
        }

        case boost::system::errc::permission_denied:
        {
            connection_info.code = 0x02;
            break;
        }

        case boost::system::errc::network_unreachable:
        {
            connection_info.code = 0x03;
            break;;
        }

        case boost::system::errc::host_unreachable:
        {
            connection_info.code = 0x04;
            break;
        }

        case boost::system::errc::connection_refused:
        {
            connection_info.code = 0x05;
            break;
        }

        case boost::system::errc::protocol_error:
        {
            connection_info.code = 0x07;
            break;
        }

        case boost::system::errc::bad_address:
        {
            connection_info.code = 0x08;
            break;
        }

        default:
        {
            connection_info.code = 0x05;
            break;
        }
    }
}

void Client::FormProtoAnswer(const ConnectionInfo &connection_info)
{
    client_buffer = {0x05, connection_info.code, 0x00};

    switch(connection_info.connection_type)
    {
        case 0x01:
        {
            client_buffer.push_back(0x01);
            break;
        }

        case (0x03):
        {
            client_buffer.push_back(0x03);
            client_buffer.push_back(connection_info.target.size());
            break;
        }

        case 0x04:
        {
            client_buffer.push_back(0x04);
            break;
        }

    }

    client_buffer.insert(client_buffer.end(), connection_info.target.begin(), connection_info.target.end());

    client_buffer.push_back(boost::endian::native_to_big(connection_info.port) & 0xFF);
    client_buffer.push_back(boost::endian::native_to_big(connection_info.port) >> 8);
}

