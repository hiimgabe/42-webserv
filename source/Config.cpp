#include "../include/Config.hpp"

ConfigParser::ConfigParser(const std::string& filename) : filename_(filename) {}

ConfigParser::~ConfigParser()
{
}

std::vector<ServerConfig>   ConfigParser::parse() {
    std::ifstream   file(filename_.c_str());
    if (!file.is_open()) {
        Log::console(ERROR, "parser error: Could not open file " + filename_);
        exit(1);
    }
    std::vector<ServerConfig>   servers;
    std::string line;
    while (std::getline(file, line)) {
        trim(line);        
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "server {") {
            servers.push_back(parseServer(file, line));
        }
        else if (!line.empty())
            throw std::runtime_error("Unexpected line in configuration (parse): " + line);
    }
    return servers;
}

void    ConfigParser::trim(std::string& str) {
    size_t  start = str.find_first_not_of(" \t");
    size_t  end = str.find_last_not_of(" \t");
    if (start == std::string::npos || end == std::string::npos) {
            str = "";
        } else {
            str = str.substr(start, end - start + 1);
        }
}

std::vector<std::string>    ConfigParser::split(const std::string& str, char delimiter) {  
    std::vector<std::string>    tokens;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        item = trailCheck(item);
        tokens.push_back(item);
    }
    return tokens;
}

int  ConfigParser::parseSize(const std::string& str) {
    return (std::atoi(str.c_str()));
}

void        ConfigParser::parseRoot(const std::string& line, Location& server) {
    std::istringstream   iss(line);
    std::string         directive;
    std::string         value;

    std::cout << "Parsing line: " << line << std::endl;

    iss >> directive >> value;
    std::cout << "Directive: " << directive << std::endl;
    std::cout << "Value: " << value << std::endl;

    if (directive == "root") {
        server.root = value;
    } else {
        throw std::runtime_error("Unknown directive: " + directive);
    }
}


bool    ConfigParser::isLocation(const std::string& line) {
    if (line.find("location") == 0 || line.find("root") == 0
        || line.find("index") == 0 || line.find("methods") == 0
        || line.find("upload_store") == 0 || line.find("cgi_path") == 0
        || line.find("cgi_extension") == 0 || line.find("autoindex") == 0
        || line.find("redirect") == 0) {
        return true;
    }
    return false;
}

static bool isOnlyNumber(std::string &str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (!std::isdigit(str[i]) && str[i] != ';')
            return (false);
    }
    return (!str.empty());
}

bool    ConfigParser::isPort(std::string &token) { return (isOnlyNumber(token)); }

std::vector<int>    ConfigParser::parsePorts(const std::string &line)
{
    std::vector<int>    ports;
    std::stringstream   ss(line);
    std::string token;

    while (ss >> token)
    {
        if (isPort(token))
            ports.push_back(atoi(token.c_str()));
    }
    return (ports);
}

bool    isPartofLocation(std::string &line)
{
    if (line.find("root ") == 0 || line.find("index ") == 0 || line.find("methods ") == 0 || line.find("upload_dir ") == 0)
        return (true);
    return (false);
}
ServerConfig    ConfigParser::parseServer(std::ifstream& file, std::string& line)
{
    ServerConfig    server;
    trim(line);
    if (line != "server {")
        throw std::runtime_error("Unexpected end of file");
    //std::cout << "Parsing server..." << std::endl;
    while (std::getline(file, line))
    {
        trim(line);
       // std::cout << line << '\n';
        if (line == "}")
            break;
        if (line.empty() || line[0] == '#')
            continue;
        if (line.find("listen ") == 0) 
            server.port = parsePorts(line); 
        else if (line.find("server_name ") == 0)
            server.server_name = trailCheck(line.substr(12));
        else if (line.find("error_page ") == 0)
        {
            std::string errorLine = trailCheck(line.substr(11));
            int errorCode = std::atoi(errorLine.c_str());
            size_t  slash = errorLine.find_first_of('/');

            std::string errorPath = errorLine.substr(slash);
            server.error_page[errorCode] = errorPath;
        }
        else if (line.find("client_max_body_size ") == 0)
            server.client_max_body_size = parseSize(line.substr(21));
        else if (isPartofLocation(line))
            parseLocation(file);
        else if (line.find("location ") == 0)
        {
            std::string locationPath = line.substr(9);
            trim(locationPath);
            if (!locationPath.empty() && locationPath[locationPath.size() - 1] == '{')
            {
                locationPath = locationPath.substr(0, locationPath.size() - 1);
                trim(locationPath);
            }
            Location location = parseLocation(file);
            server.locations[locationPath] = location;
            server.locations[locationPath].url = locationPath;
        }
        else
            throw std::runtime_error("Unknown directive (parse server): " + line);
    }
//    std::cout << "Server parsed!" << std::endl;
//    std::cout << "Port: ";
//    for (size_t i = 0; i < server.port.size(); i++)
//        std::cout << server.port[i] << " ";
//    std::cout << "\nServer Name: " << server.server_name << std::endl;
//    std::cout << "  Error Pages:\n";
//    for (std::map<int, std::string>::iterator it = server.error_page.begin(); it != server.error_page.end(); ++it)
//        std::cout << "    " << it->first << " " << it->second << '\n';
//    std::cout << "Max Body Size: " << server.client_max_body_size << std::endl;
    return server;
}

Location    ConfigParser::parseLocation(std::ifstream& file) {
    Location    location;
    location.autoindex = false;
    std::string line;
//    std::cout << "Parsing location..." << std::endl;
//    std::cout << "parsing location lines\n";
    while (std::getline(file, line)) {
        trim(line);
        if (line == "}")
            break;
        if (line.empty() || line[0] == '#')
            continue;
        else if (line.find("root ") == 0) {
            location.root = trailCheck(line.substr(5));
            //parseRoot(line, location);
        }
        else if (line.find("index ") == 0)
            location.index = trailCheck(line.substr(6));
        else if (line.find("methods ") == 0)
            location.methods = split(line.substr(8), ' ');
        else if (line.find("upload_store ") == 0)
        {
            location.upload_dir = trailCheck(line.substr(13));
            size_t  trailDot = location.upload_dir.find('.');
            if (trailDot != std::string::npos)
                location.upload_dir = location.upload_dir.substr(trailDot + 1, location.upload_dir.size());
        }
        else if (line.find("cgi_path ") == 0)
        {
            location.cgi_path = split(line.substr(9), ' ');
            size_t  cgiTrailPos = location.cgi_path[location.cgi_path.size() - 1].find(';');
            if (cgiTrailPos != std::string::npos)
                location.cgi_path[location.cgi_path.size() - 1] = location.cgi_path[location.cgi_path.size() - 1].substr(0, cgiTrailPos);
        }
        else if (line.find("cgi_extension ") == 0)
        {
            location.cgi_extension = split(line.substr(14), ' ');
            size_t  cgiTrailPos = location.cgi_extension[location.cgi_extension.size() - 1].find(';');
            if (cgiTrailPos != std::string::npos)
                location.cgi_extension[location.cgi_extension.size() - 1] = location.cgi_extension[location.cgi_extension.size() - 1].substr(0, cgiTrailPos);
        }
        else if (line.find("autoindex ") == 0)
        {
            std::string autoIndexStr = trailCheck(line.substr(10));
            location.autoindex = autoIndexStr == "on" ? true : false;
        }
        else if (line.find("redirect ") == 0)
        {
            int code = getRedirectCode(line);
            std::string redirect = getRedirectUrl(line);
            location.redirect.first = code; 
            location.redirect.second = redirect; 
        }
        else if (!isLocation(line)) {
            break;
        }
    }
//    std::cout << "Location parsed!" << std::endl;
//    std::cout << "Location: " << location.url << std::endl;
//    std::cout << "Root: " << location.root << std::endl;
//    std::cout << "Index: " << location.index << std::endl;
//    std::cout << "Upload Dir: " << location.upload_dir << std::endl;
//    std::cout << "autoindex: " << location.autoindex << std::endl;
//    std::cout << "redirect: " << location.redirect.first << " " << location.redirect.second << '\n';
    return location;
}

ssize_t readRequest(int fd, char *buffer, size_t bufferSize)
{
    ssize_t n = recv(fd, buffer, bufferSize, 0);
    if (n < 0)
    {
        if (DEBUG)
            std::cerr << "ERROR reading from client request on server: \n";
    }
    return (n);
}

void    ConfigParser::handleClientRequests(std::vector<pollfd>& poll_fds, size_t& i, const std::vector<ServerConfig> &servers) {

    char buffer[4096];
    int fd = poll_fds[i].fd;
    std::string fullRequest;
    BufferRequest   bufferRequest;
    clock_t start = getTime();

    while (!bufferRequest.requestOver())
    {
        clock_t currTime = getTime();
        double  elapsed = currTime - start;
        if (elapsed > 5.0)
            break;
        std::memset(buffer, 0, sizeof(buffer));
        ssize_t n = readRequest(fd, buffer, sizeof(buffer));
        if (n < 0)
        {
            bufferRequest.getBuffer().clear();
            break;
        }
        bufferRequest.appendBuffer(buffer, n);
    }
    if (bufferRequest.getBuffer().empty())
    {
        _pendingFds.push_back(fd);
        return ;
    }
    Request requestHandler(bufferRequest.getBuffer(), fd, servers);
    _pendingFds.push_back(fd);
}

void    ConfigParser::setupListeningSocket(const ServerConfig &servers, std::vector<pollfd> *poll_fds, std::vector<ListeningSocket*> *sockets)
{
    for (size_t j = 0; j < servers.port.size(); j++)
    {
        try
        {
            //std::cout << "Setting up server on port " << servers.port[j] << std::endl;
            ListeningSocket *server = new ListeningSocket(AF_INET, SOCK_STREAM, 0, servers.port[j], INADDR_ANY, 10);
            int sock_fd = server->get_sock();
            fcntl(sock_fd, F_SETFL, O_NONBLOCK);
            pollfd  pfd;
            pfd.fd = sock_fd;
            pfd.events = POLLIN | POLLOUT;
            pfd.revents = 0;
            poll_fds->push_back(pfd);
            sockets->push_back(server);
            std::cout << "Server listening on port " << servers.port[j] << std::endl;
        }
        catch(const std::exception& e)
        {
            throw   ServerException("Failed to set up server on port: " + toString(servers.port[j]) + '\n');
        }
    }
}

void    ConfigParser::handleIncomingConnections(const std::vector<ServerConfig>& servers) {
    std::vector<ListeningSocket*>   sockets;
    std::vector<pollfd>             poll_fds;

    Log::console(MESSAGE ,"SETTIN UP SERVERS...\n");

    for (size_t i = 0; i < servers.size(); ++i)
    {
        try
        {
            setupListeningSocket(servers[i], &poll_fds, &sockets);
        }
        catch(const std::exception& e)
        {
            for (size_t i = 0; i < sockets.size(); ++i)
                delete sockets[i];
            std::cerr << e.what() << '\n';
            Log::console(ERROR, "Failed to setup Listening Sockets");
            throw   ServerException("Failed to setup Listening Sockets \n");
        }
        
    }

    std::cout << "Entering main event loop..." << std::endl;
    while (!endFlag)
    {

        int activity;
        do
        {
            activity = poll(&poll_fds[0], poll_fds.size(), 350); 
        } while (activity < 0 && errno == EINTR);
        if (activity < 0 && !endFlag)
        {
            Log::console(ERROR, "Poll failed");
            continue;
        }
        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            if (poll_fds[i].revents & POLLIN)
            {
                //check if the event is for the listening socket
                if (isListeningSocket(poll_fds[i].fd, sockets))
                {           
                    struct sockaddr_in  clientAddress;
                    socklen_t           clientAddressLength = sizeof(clientAddress);
                    if (DEBUG)
                        std::cout << "Incoming connection detected on socket..." << poll_fds[i].fd << std::endl;        
                    int clientSocket = accept(poll_fds[i].fd, (struct sockaddr*)&clientAddress, &clientAddressLength);
                    if (clientSocket < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            continue;
                        Log::console(ERROR, "Failed to accept incoming connection ");
                        continue;
                    }
                if (DEBUG)
                    std::cout << "New client connected! FD:" << clientSocket << std::endl;
                fcntl(clientSocket, F_SETFL, O_NONBLOCK);
                pollfd  client_fd;
                client_fd.fd = clientSocket;
                client_fd.events = POLLIN | POLLOUT;
                client_fd.revents = 0;
                poll_fds.push_back(client_fd);
                }
                else if (poll_fds[i].revents & POLLOUT)
                    handleClientRequests(poll_fds, i, servers);
            }
        }
        cleanupPendingFds(poll_fds);
    }
    std::cout << RED << "Stoping servers" << RESET << '\n';
    for (size_t i = 0; i < sockets.size(); ++i)
        delete sockets[i];
}

std::string ConfigParser::trailCheck(const std::string  &str)
{
    size_t  trail = str.find(';');

    if (trail != std::string::npos)
        return (str.substr(0, trail));
    return (str);
}

int ConfigParser::getRedirectCode(const std::string &line)
{
    std::string code;
    std::string::const_iterator   it = line.begin();

    while (!std::isdigit(*it))
        ++it;
    while(std::isdigit(*it))
    {
        code += *it;
        ++it;
    }
    return (std::atoi(code.c_str()));
}

std::string ConfigParser::getRedirectUrl(const std::string &line)
{
    std::vector<std::string>    tokens = split(line, ' ');
    std::string url = tokens.back();

    url = trailCheck(url);
    return (url);
}

void    ConfigParser::cleanupPendingFds(std::vector<pollfd> &clientFds)
{
    std::vector<int>::iterator   itFd = _pendingFds.begin();

    for (;itFd != _pendingFds.end();)
    {
        std::vector<pollfd>::iterator itClient = std::find_if(clientFds.begin(), clientFds.end(), MatchFd(*itFd));

        if (itClient != clientFds.end())
        {
            close(itClient->fd);
            clientFds.erase(itClient);
        }
        itFd = _pendingFds.erase(itFd);
    }
}