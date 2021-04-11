#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

class Logger
{
public:
    explicit Logger(std::string path);
    [[maybe_unused]] Logger(Logger&& other) noexcept;
    Logger(const Logger& other) = delete;

    Logger& operator=(Logger&& other) noexcept;
    Logger& operator=(const Logger& other) = delete;

    void Log(std::string msg);

    void Flush();

    ~Logger();

private:
    static std::string FormFilename();
    std::vector<std::string> logs;
    std::ofstream logfile;
};


