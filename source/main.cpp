#include <fstream>
#include <csignal>
#include "../include/MainHeader.hpp"
#include "../include/Log.hpp"
#include "../include/Config.hpp"

bool    configExists(std::string str)
{
    std::ifstream   file(str.c_str());
    struct stat fileInfo;

    stat(str.c_str(), &fileInfo);
    if (!file.is_open())
    {
        Log::console(ERROR, "Invalid configuration file.");
        return (false);
    }
    else if (S_ISDIR(fileInfo.st_mode))
    {
        Log::console(ERROR, "Configuration file can't be a directory.");
        return (false);
    }
    return (true);
}

int main(int argc, char **argv, char **env)
{
    std::signal(SIGINT, stopServer);
    std::signal(SIGPIPE, SIG_IGN);
    (void)env;
    if (argc != 2) {
        Log::console(ERROR, "❌ Invalid usage!\n"
            "Usage: ./webserv [configuration file]");
        return (1);
    }
    if (argc == 2 && configExists(argv[1])) {
        try {
            ConfigParser    parser(argv[1]);
    std::vector<ServerConfig>   servers = parser.parse();

//            for (size_t i = 0; i < servers.size(); ++i)
//            {
//                std::cout << "Server " << i + 1 << ":\n";
//                std::cout << "Port: ";
//                for (size_t p = 0; p < servers[i].port.size(); p++)
//                    std::cout << servers[i].port[p] << " ";
//                std::cout << "\n  Server Name: " << servers[i].server_name << "\n";
//                std::cout << "  Error Pages:\n";
//                for (std::map<int, std::string>::iterator it = servers[i].error_page.begin(); it != servers[i].error_page.end(); ++it)
//                    std::cout << "    " << it->first << " " << it->second << '\n';
//                std::cout << "  Max Body Size: " << servers[i].client_max_body_size << " bytes\n";
//                for (std::map<std::string, Location>::iterator it = servers[i].locations.begin(); it != servers[i].locations.end(); ++it) {
//                    std::cout << "  Location: " << it->first << "\n";
//                    std::cout << "    url:  " << it->second.url << '\n';
//                    std::cout << "    Root: " << it->second.root << "\n";
//                    std::cout << "    Index: " << it->second.index << "\n";
//                    std::cout << "    Methods: ";
//                    for (size_t j = 0; j < it->second.methods.size(); ++j) {
//                        std::cout << it->second.methods[j] << (j + 1 < it->second.methods.size() ? ", " : "");
//                    }
//                    std::cout << "\n";
//                    if (!it->second.upload_dir.empty()) {
//                        std::cout << "    Upload Dir: " << it->second.upload_dir << "\n";
//                    }
//                    if (it->second.autoindex)
//                        std::cout << "    autoindex: true\n";
//                }
//            }
            parser.handleIncomingConnections(servers);
        } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
        }
    }
        
    return 0;
}