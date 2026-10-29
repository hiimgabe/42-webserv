#include "../include/Utils.hpp"

bool isListeningSocket(int fd, const std::vector<ListeningSocket*>& sockets) {
    for (size_t i = 0; i < sockets.size(); ++i) {
        if (sockets[i]->get_sock() == fd) {
            return true;
        }
    }
    return false;
}

std::string getExtension(const std::string &uri) {
    size_t queryPos = uri.find("?");
    std::string extension = (queryPos != std::string::npos) ? uri.substr(0, queryPos) : uri;
    size_t dotPos = extension.find_first_of(".");

    if (dotPos == std::string::npos)
        return ("");
    if (dotPos == extension.size() - 1)
        return ("");
    extension = extension.substr(dotPos);
    extension = extension.substr(0, extension.find_first_of("/#"));
    return (extension);
}

std::string getMatchingUrl(const std::map<std::string, Location> &locations, const std::string &uri)
{
    std::string match = "";
    std::string currLoc;
    std::map<std::string, Location> locs = locations;
    std::map<std::string, Location>::const_iterator   itLoc = locs.begin();
    std::map<std::string, Location>::const_iterator   iteLoc = locs.end();

    for (; itLoc != iteLoc; itLoc++)
    {      
        currLoc = itLoc->second.url;
        if (uri.find(currLoc) == 0 && currLoc.size() > match.size())
            match = currLoc;
    }
    return (match);
}

Location    getLocation(Request &request, const std::map<std::string, Location> &locations)
{
    std::string matchingUrl = getMatchingUrl(locations, request.getUri());
    Location    loc;

    std::map<std::string, Location>::const_iterator   it = locations.begin();
    std::map<std::string, Location>::const_iterator   ite = locations.end();
    for (; it != ite; ++it)
    {
        if (matchingUrl == it->second.url)
            loc = it->second;
    }
    return (loc);
}

std::string	getWorkingDir()
{
	char	temp[4096];
	return (getcwd(temp, sizeof(temp)) ? std::string(temp) : std::string(""));
}

std::string customTrailCheck(const std::string  &str, char c)
{
    size_t  trail = str.find_last_of(c);

    if (trail != std::string::npos)
        return (str.substr(0, trail));
    return (str);
}

double getTime()
{
    struct timeval  tv;

    if (gettimeofday(&tv, NULL))
        throw   std::runtime_error("Failed to get time of day.\n");
    return (tv.tv_sec + (tv.tv_usec / 1e6));
}

std::pair<std::string, std::string>	setReasonAndMessage(int code)
{
	std::pair<std::string, std::string>	res;
    switch (code)
    {
    	case 404:
    	    res.first = "Not Found";
    	    res.second = "The requested resource was not found on this server.";
    	    break;
    	case 405:
    	    res.first = "Method Not Allowed";
    	    res.second = "Method isn't allowed here.";
    	    break;
    	case 403:
    	    res.first = "Forbidden";
    	    res.second ="You do not have permission to access the requested resource.";
    	    break;
    	case 400:
    	    res.first = "Bad Request";
    	    res.second = "The server could not understand the request due to invalid syntax.";
    	    break;
    	case 401:
    	    res.first = "Unauthorized";
    	    res.second = "Authentication is required and has failed or has not yet been provided.";
    	    break;
    	case 409:
    	    res.first = "Conflict";
    	    res.second = "The request could not be completed due to a conflict with the current state of the resource.";
    	    break;
    	case 413:
    	    res.first = "Content Too Large";
    	    res.second = "Maximum allowed upload size exceeded.";
    	    break;
    	case 500:
    	    res.first = "Internal Server Error";
    	    res.second = "The server encountered an internal error and was unable to complete your request.";
    	    break;
    	case 504:
    	    res.first = "Gateway Timeout";
    	    res.second = "The server, while acting as a gateway or proxy, did not receive a timely response from the upstream server.";
    	    break;
    	default:
    	    res.first = "Internal Server Error";
    	    res.second = "An internal server error occurred.";
    	    break;
    }
	return (res);
}