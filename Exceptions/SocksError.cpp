#include "SocksError.hpp"

SocksError::SocksError(ErrorType type_, std::string msg) : type(type_), message(std::move(msg)), is_occured(true)
{ }

SocksError::operator bool() const
{
    return is_occured;
}

std::string SocksError::Message() const
{
    return message;
}

ErrorType SocksError::Type() const
{
    return type;
}

SocksError::SocksError(SocksError &&other) noexcept
{
    is_occured = other.is_occured;
    other.is_occured = false;

    type = other.type;
    other.type = ErrorType::NoError;

    message = std::move(other.message);
}

SocksError::SocksError(const SocksError &other)
{
    is_occured = other.is_occured;
    type = other.type;
    message = other.message;
}

SocksError &SocksError::operator=(SocksError &&other) noexcept
{
    if(this!=&other)
    {
        is_occured = other.is_occured;
        other.is_occured = false;

        type = other.type;
        other.type = ErrorType::NoError;

        message = std::move(other.message);
    }

    return *this;
}

SocksError &SocksError::operator=(const SocksError &other)
{
    if(this!=&other)
    {
        is_occured = other.is_occured;
        type = other.type;
        message = other.message;
    }

    return *this;
}


