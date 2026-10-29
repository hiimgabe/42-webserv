#include "../include/ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int serviceType, int protocol, int port, u_long ip, int backlog) : BindingSocket(domain, serviceType, protocol, port, ip), backlog(backlog)
{
    start_listening();
    //testing unnecessary
    test_connection(get_connection());
}

ListeningSocket::~ListeningSocket() {}

void    ListeningSocket::start_listening() {
    int listen_status = listen(get_sock(), backlog);
    test_connection(listen_status);
}