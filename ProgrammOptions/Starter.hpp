#pragma once
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

class Starter
{
public:

    explicit Starter(const char* starter_name);

    void ParseArguments(int argc, char** argv);

    void AddArgument(const char *arg_name, const char *arg_description);

    template<typename ArgType>
    void AddArgument(const char* arg_name, const char* arg_description, ArgType default_value);

    template<typename ArgType>
    void AddRequiredArgument(const char* arg_name, const char* arg_description);

    void PrintDescription();

    bool IsExist(const char *arg_name);

    template<typename ArgType>
    void GetArgValue(const char* arg_name, ArgType& to);

    [[nodiscard]] bool Failed() const;

private:

    bool failed = false;
    boost::program_options::options_description description;
    boost::program_options::variables_map parsed_arguments;

};

template<typename ArgType>
void Starter::AddArgument(const char* arg_name, const char* arg_description, ArgType default_value)
{
    description.add_options()(arg_name, boost::program_options::value<ArgType>()->default_value(default_value), arg_description);
}

template<typename ArgType>
void Starter::GetArgValue(const char* arg_name, ArgType& to)
{
    if(parsed_arguments.empty())
    {
        throw std::logic_error{"Cannot get option" + std::string{arg_name} + ". Arguments are not parsed yet"};
    }


    if (IsExist(arg_name))
    {
        to = parsed_arguments[arg_name].as<ArgType>();
    }

    else throw std::runtime_error{ "Argument " + std::string{arg_name} + " doesn't exist" };
}

template<typename ArgType>
void Starter::AddRequiredArgument(const char* arg_name, const char* arg_description)
{
    description.add_options()(arg_name, boost::program_options::value<ArgType>()->required(), arg_description);
}


