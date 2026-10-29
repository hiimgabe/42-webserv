#include "../include/SimpleSocket.hpp"

SimpleSocket::SimpleSocket(int domain, int serviceType, int protocol, int port, u_long ip) {
    //define address struct
    address.sin_family = domain;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(ip);
    //establish socket
    this->sock = socket(domain, serviceType, protocol);
    int opt = 1;
    if (setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw std::runtime_error("Failed to set socket opt.");
    }
    test_connection(sock);
    //bind function called on the server side code. 
    //connect in client side
    //establish connection
    //this->connection = connect_to_network(sock, address);
    //test_connection(connection);
}

SimpleSocket::~SimpleSocket() {}

//virtual function
//checks if the socket or the connection has been properly established
void    SimpleSocket::test_connection(int test) {
    if (test < 0) {
        throw std::runtime_error("Failed to connect");
    }
}

//getter functions
int SimpleSocket::get_sock(void) {
    return sock;
}

struct  sockaddr_in SimpleSocket::get_address() {
    return address;
}

int SimpleSocket::get_connection() {
    return connection;
}

//setter functions

void    SimpleSocket::set_connection(int con) {
    connection = con;
}