#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include "Socket.hpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

namespace ws {

class Listensocket : public Socket
{
public:
    Listensocket();
    Listensocket(const int domain, const int type, const int protocol);
    void bind(const struct sockaddr_in6 server_address) const;
};

} //Namespace ws

#endif
