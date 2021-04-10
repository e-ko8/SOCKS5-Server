#include "Logger.hpp"

Logger::Logger(std::string path)
{
    std::string filepath;

    if(path.empty())
    {
        filepath = "server.log";
    }

    else if(path.back() == '/')
    {
        filepath = {std::move(path) + "server.log"};
    }

    else
    {
        filepath = {std::move(path) + "/server.log"};
    }

    logfile.open(filepath);

    if(!logfile)
    {
        throw std::runtime_error("Can't open logfile " + filepath);
    }
}

void Logger::Log(std::string error)
{
    logs.emplace_back(std::move(error));
}

void Logger::Flush()
{
    if(logfile)
    {
        for(const auto& log : logs)
        {
            logfile << log;
        }
    }

    logs.clear();
}

Logger::~Logger()
{
    Flush();
}

Logger &Logger::operator=(Logger &&other) noexcept
{
    if(this != &other)
    {
        logs = std::move(other.logs);
        logfile = std::move(other.logfile);
    }

    return *this;
}

Logger::Logger(Logger &&other) noexcept
{
    logs = std::move(other.logs);
    logfile = std::move(other.logfile);
}
