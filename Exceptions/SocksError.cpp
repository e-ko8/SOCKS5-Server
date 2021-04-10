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


