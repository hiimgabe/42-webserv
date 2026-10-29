#include "../include/BindingSocket.hpp"

//constructor
BindingSocket::BindingSocket(int domain, int serviceType, int protocol, int port, u_long ip) : SimpleSocket(domain, serviceType, protocol, port, ip) {
        set_connection(connect_to_network(get_sock(), get_address()));
        test_connection(get_connection());
    }

BindingSocket::~BindingSocket() {}

//definition of connetc_to network virtual funtion
int BindingSocket::connect_to_network(int sock, struct sockaddr_in address) {
    return bind(sock, (struct sockaddr *)&address, sizeof(address));
    //socket to be bind, address to bind to, size of address structure
}

/*here I use the bind() function because the server will associate its socket with a 
specific local IP address and port number. this establishes a unique identifier for 
the socket so that it can accept incoming connections and data.
-Servers use bind() to specify which port they will listen on for incoming connectiomns
-sets the local endpoint for the socket*/
