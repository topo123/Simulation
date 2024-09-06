#ifndef LOGGER_H
#define LOGGER_H
#include <fstream>

class Logger
{
	std::ofstream logger;
public:
	bool do_log = true;
	void init_logger(std::string log_name);
	void log(std::string message);
};

#endif
