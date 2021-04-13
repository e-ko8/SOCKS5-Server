#include "Starter.hpp"
#include <iostream>

Starter::Starter(const char* starter_name)  : description(starter_name)
{

}

void Starter::ParseArguments(int argc, char **argv)
{
    try
    {
        boost::program_options::store( boost::program_options::parse_command_line(argc,argv,description),parsed_arguments);
        boost::program_options::notify(parsed_arguments);
    }

    catch(std::exception& error)
    {
        std::cout << error.what() << "\n";
        PrintDescription();
        failed = true;
    }

}

void Starter::AddArgument(const char *arg_name, const char *arg_description)
{
    description.add_options()(arg_name, arg_description);
}

void Starter::PrintDescription()
{
    std::cout << description << "\n";
}

bool Starter::IsExist(const char *arg_name)
{
    if(parsed_arguments.empty())
    {
        throw std::logic_error{"Argument " + std::string{arg_name} + " doesn't exist. Arguments are not parsed yet"};
    }

    return parsed_arguments.count(arg_name)!=0;
}

bool Starter::Failed() const
{
    return failed;
}
