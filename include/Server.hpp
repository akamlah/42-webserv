/* ************************************************************************ */
/*                                                                          */
/*                              Class: Server                               */
/*                                                                          */
/* ************************************************************************ */

#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include "Socket.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "utility.hpp"

#include <arpa/inet.h> // htons etc
#include <string.h> // bezero
#include <iostream>
#include <sys/event.h> // kqueue

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/ioctl.h>

#include "Poll.hpp"

#define DEBUG 0

#define BUFFER_SIZE 5000

namespace ws {

class Server {

    public:

        class SystemError: public ws::exception {
                virtual const char* what() const throw();
        };

        Server(Socket& server_socket, int port); // change this to constructor with a "config" object later
        ~Server();

        void listen(const int backlog) const;
        int accept() const;
        void handle_connection(Socket& new_connection) const;
        void respond(Socket& new_connection, Request request) const;
        void run(int timeout_in_milliseconds);

        const Socket& socket() const;
        int port() const;

    private:

        const Socket _socket;
        const int _port;
        struct sockaddr_in6 _address;
        Poll    _poll;
        char    _buffer[BUFFER_SIZE];

}; // Class Server

} // NAMESPACE ws

#endif // __SERVER_HPP__
