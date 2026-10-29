#ifndef SIMPLE_SOCKET_HPP
#define SIMPLE_SOCKET_HPP

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <poll.h>

// socketFunction, create socket itself
// bind it to the network
// listen
// the type for web server is SOCK_STREAM
//namespace GS {
class SimpleSocket {
private:
    struct sockaddr_in  address;
    int sock;
    int connection;
public:
    //constructor
    SimpleSocket(int domain, int serviceType, int protocol, int port, u_long ip);
    virtual ~SimpleSocket();
    //function to connect to network. virtual because different types of sockets need to do something different here
    virtual int connect_to_network(int sock, struct sockaddr_in address) = 0;
    //test connections and sockets
    void    test_connection(int test);
    int     get_sock(void);
    struct  sockaddr_in get_address();
    int     get_connection();
    void    set_connection(int con);        
};
//}
#endif