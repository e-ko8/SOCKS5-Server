#pragma once
#include <string>
#include <stdexcept>


enum class ErrorType {InvalidRequest,MessageNotFull, NoError, NoSuchMethod};

class SocksError
{

public:

    SocksError() = default;
    SocksError(ErrorType type_, std::string msg);

    explicit operator bool() const;

    [[nodiscard]] std::string Message() const;

    [[nodiscard]] ErrorType Type() const;

private:

    bool is_occured = false;
    ErrorType type = ErrorType::NoError;
    std::string message = "success";
};


