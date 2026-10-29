
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <ctime>
#include <sys/time.h>
#include "MainHeader.hpp"
#include "Request.hpp"

class Request;

extern bool endFlag;
void	stopServer(int signum);
bool isListeningSocket(int fd, const std::vector<ListeningSocket*>& sockets);
std::string getExtension(const std::string &uri);
Location    getLocation(Request &request, const std::map<std::string, Location> &locations);
std::string getWorkingDir();
std::string customTrailCheck(const std::string  &str, char c);
std::pair<std::string, std::string>	setReasonAndMessage(int code);
double  getTime();

template <typename T>
std::string toString(T value)
{
    std::stringstream stream;
    stream << value;
    return (stream.str());
}

struct MatchFd
{
    int match;
    
    MatchFd(int fd) : match(fd) {}

    bool    operator()(const pollfd &pfd) const
    {
        return pfd.fd == match;
    }
};



#endif