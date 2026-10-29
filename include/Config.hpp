#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <map>
#include <algorithm>
#include <ctime>
#include "MainHeader.hpp"
#include "Log.hpp"

struct  Location {
    //can i initialize here, all empty = "", except methods
    std::string url;
    std::string root;
    std::string index;
    std::string upload_dir;
    std::pair<int, std::string>  redirect;
    std::vector<std::string>    cgi_path;
    std::vector<std::string>    cgi_extension;
    std::vector<std::string>    methods;
    bool    autoindex;
};

struct  ServerConfig {
    //initialize? 80; 1 << 20 (1MB); "localhost"; "error.html";
    std::vector<int> port;
    int client_max_body_size;
    std::string server_name;
    std::map<int, std::string> error_page;
    std::map<std::string, Location> locations;
};

#include "Request.hpp"
#include "Utils.hpp"

class   ConfigParser {
    private:
        std::string filename_;
        ServerConfig    parseServer(std::ifstream& file, std::string& line);
        Location    parseLocation(std::ifstream& file);

    public:
        ConfigParser(const std::string& filename);
        ~ConfigParser();
        void    cleanupPendingFds(std::vector<pollfd> &clientFds);
        void    handleIncomingConnections(const std::vector<ServerConfig>& servers);
        void    handleClientRequests(std::vector<pollfd>& poll_fds, size_t& i, const std::vector<ServerConfig> &servers);
        void    setupListeningSocket(const ServerConfig &servers, std::vector<pollfd> *poll_fds, std::vector<ListeningSocket*> *sockets);
        std::vector<ServerConfig>   parse();

        int getRedirectCode(const std::string &line);
        std::string getRedirectUrl(const std::string &line);
        bool    isLocation(const std::string& line);
        static void    trim(std::string& str);
        std::vector<std::string>    split(const std::string& str, char delimiter);
        int  parseSize(const std::string& str);
        //maybe instead of server use location, cleaner
        void    parseRoot(const std::string& line, Location& server);
		std::vector<int>	parsePorts(const std::string	&line);
		bool	isPort(std::string &token);
        std::string trailCheck(const std::string &str);
        std::vector<int>             _pendingFds;

        class   ServerException : public std::exception
        {
            public:
                ServerException(const std::string &error) : msg(error) {}
                ~ServerException() throw() {}
                const char  *what() const throw() { return(msg.c_str()); }

            private:
                std::string msg;
        };
};

//void  setupServerSockets(const std::vector<ServerConfig>& servers);

#endif