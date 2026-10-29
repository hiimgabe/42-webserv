
#include <csignal>

bool	endFlag = false;

void	stopServer(int signum)
{
	if (signum == SIGINT)
		endFlag = true;
}