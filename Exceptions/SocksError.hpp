#pragma once
#include <string>
#include <stdexcept>


enum class ErrorType {InvalidRequest, MessageNotFull, NoError, NoSuchMethod};

class SocksError
{

public:

    SocksError() = default;
    SocksError(ErrorType type_, std::string msg);
    SocksError(SocksError&& other) noexcept;
    SocksError(const SocksError& other);

    SocksError& operator=(SocksError&& other) noexcept;
    SocksError& operator=(const SocksError& other);

    explicit operator bool() const;

    [[nodiscard]] std::string Message() const;

    [[nodiscard]] ErrorType Type() const;

private:

    bool is_occured = false;
    ErrorType type = ErrorType::NoError;
    std::string message = "success";
};


