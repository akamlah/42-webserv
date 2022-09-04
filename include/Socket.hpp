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

#include <iostream>

namespace ws {

class Socket {

    public:

        class SystemError: public ws::exception {
            public:
                virtual const char* what() const throw();
        };

        Socket();
        Socket(const int domain, const int type, const int protocol);
        // + cpy constr
        // + cpy assign ope
        ~Socket();

        void bind(const struct sockaddr_in server_address) const;

        int fd;

    // pvt:
        // address ?
        // int domain; ?
        // int type; ?
        // int protocol; ?

}; // CLASS Socket

} // NAMESPACE ws

#endif // __SOCKET_HPP__
