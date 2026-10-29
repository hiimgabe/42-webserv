#ifndef CONNECTION_SOCKET_HPP
#define CONNECTION_SOCKET_HPP

#include "SimpleSocket.hpp"
#include <stdio.h> 

//namespace GS {
class ConnectionSocket : public SimpleSocket
{
    public:
    //Constructor
        ConnectionSocket(int domain, int serviceType, int protocol, int port, u_long ip);
        ~ConnectionSocket();
        //virtual funtion from parent socket
        int connect_to_network(int sock, struct sockaddr_in address);
};

//}

#endif
