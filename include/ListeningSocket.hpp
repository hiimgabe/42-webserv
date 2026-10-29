#ifndef LISTENING_SOCKET_HPP
#define LISTENING_SOCKET_HPP

#include "BindingSocket.hpp"

//namespace GS {
class ListeningSocket : public BindingSocket
{
    private:
        int backlog;
        int listening;
    public:
        ListeningSocket(int domain, int serviceType, int protocol, int port, u_long ip, int backlog);
        ~ListeningSocket();
        void    start_listening();
};
//}
#endif