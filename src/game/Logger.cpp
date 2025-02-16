#include <Logger.hpp>

void Logger::init_logger(std::string log_name)
{
	logger = std::ofstream("logs/" + log_name);
	do_log = true;
}

void Logger::log(std::string message)
{
	logger << message << std::endl;
	logger.flush();
}
