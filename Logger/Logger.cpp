#include "Logger.hpp"
#include <chrono>
#include <sstream>

Logger::Logger(std::string path)
{
    std::string filename = FormFilename();
    std::string filepath;

    if(path.empty())
    {
        filepath = filename;
    }

    else if(path.back() == '/')
    {
        filepath = {std::move(path) + filename};
    }

    else
    {
        filepath = {std::move(path) + "/" + filename};
    }

    logfile.open(filepath);

    if(!logfile)
    {
        throw std::runtime_error("Can't open logfile " + filepath);
    }
}

void Logger::Log(std::string msg)
{
    logs.emplace_back(std::move(msg) + "\n");
    if(logs.size() > 100)
    {
        Flush();
    }
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

std::string Logger::FormFilename()
{
    std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* local_time = localtime(&current_time);

    std::ostringstream os;
    os << "server_"
    << std::setw(2) << std::setfill('0') << std::to_string(local_time->tm_mday) << "."
    << std::setw(2) << std::setfill('0') << std::to_string(local_time->tm_mon + 1) << "."
    << std::to_string(local_time->tm_year+1900) << "_"
    << std::setw(2) << std::setfill('0') << std::to_string(local_time->tm_hour) << ":"
    << std::setw(2) << std::setfill('0')  << std::to_string(local_time->tm_min) << ":"
    << std::setw(2) << std::setfill('0') << std::to_string(local_time->tm_sec) << ".log";

    return os.str();
}
