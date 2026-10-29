#include "../include/ConnectionSocket.hpp"
//this would be the client socket, so that we have client and server

//Constructor
ConnectionSocket::ConnectionSocket(int domain, int serviceType, int protocol, int port, u_long ip) : SimpleSocket(domain, serviceType, protocol, port, ip) {
        set_connection(connect_to_network(get_sock(), get_address()));
        test_connection(get_connection());
}

ConnectionSocket::~ConnectionSocket() {}

int ConnectionSocket::connect_to_network(int sock, struct sockaddr_in address) {
    return connect(sock, (struct sockaddr *)&address, sizeof(address));
}

/* The connect() function is used by a client to establish a connection to a server
It specifies the servers IP address and port number that the client wants to connect to.
the function initiates the connection request from the client to the server
-connect() to establish a connection to a specific server
-sets remote endpoint for the socket */