#include "../include/Log.hpp"

void    Log::console(e_log log, const std::string &msg) {
    struct  tm   *local_time;
    time_t              rawtime;
    char                buffer[32];

    time(&rawtime);
    local_time = localtime(&rawtime);
    strftime(buffer, 32, "%T", local_time);  
	switch (log)
	{
		case ERROR:
                std::cerr << RED << "[" << buffer << "] " << msg << RESET << std::endl;
			break;
		case WARNING:
                std::cout << YELLOW << "[" << buffer << "] " << msg << RESET << std::endl;
			break;
		case MESSAGE:
                std::cout << "[" << buffer << "] " << msg << std::endl;
			break;
    }
}