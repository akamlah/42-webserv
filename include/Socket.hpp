/* ************************************************************************ */
/*                                                                          */
/*                              Class: Socket                               */
/*                                                                          */
/* ************************************************************************ */

#ifndef __SOCKET_HPP__
# define __SOCKET_HPP__

#include "utility.hpp"

#include <sys/socket.h> // bind & socket
#include <unistd.h> // close
#include <netinet/in.h> // addresses
#include <sys/ioctl.h> //ioctl

#include <iostream>

namespace ws {

class Socket {

    public:

        class SystemError: public ws::exception {
            public:
                virtual const char* what() const throw();
        };

        Socket();
        Socket(const int domain, const int type, const int protocol, const int port = -1);
        Socket(const Socket& other);
        Socket(int fd);

        bool operator<(const Socket& other) const;

        void bind(const struct sockaddr_in6 server_address) const;
        void connect(const struct sockaddr_in6 server_address) const;
        // + cpy assign ope
        virtual ~Socket();

        int fd;
        int port;

    // pvt:
        // address ?
        // int domain; ?
        // int type; ?
        // int protocol; ?

}; // CLASS Socket

} // NAMESPACE ws

#endif // __SOCKET_HPP__
