#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

class Logger
{
public:
    explicit Logger(std::string path);
    Logger(Logger&& other) noexcept;
    Logger(const Logger& other) = delete;

    Logger& operator=(Logger&& other) noexcept;
    Logger& operator=(const Logger& other) = delete;

    void Log(std::string error);

    void Flush();

    ~Logger();

private:
    std::vector<std::string> logs;
    std::ofstream logfile;
};


