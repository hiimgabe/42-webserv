#ifndef BINDING_SOCKET_HPP
#define BINDING_SOCKET_HPP

#include "SimpleSocket.hpp"

//namespace GS {
class BindingSocket : public SimpleSocket {
public:
    //Constructor
    BindingSocket(int domain, int serviceType, int protocol, int port, u_long ip);
    virtual ~BindingSocket();
    //virtual function from parent Socket
    int connect_to_network(int sock, struct sockaddr_in address);
};
//}

#endif