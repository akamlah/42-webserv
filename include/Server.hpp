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
#include "Poll.hpp"

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
#include <map>

#define BUFFER_SIZE 5000

namespace ws {

class Server {

    public:

        class SystemError: public ws::exception {
                virtual const char* what() const throw();
        };

        Server(config_data& configData); // change this to constructor with a "config" object later
        Server(Socket& server_socket, int port); // change this to constructor with a "config" object later
        ~Server();

        void listen(const int backlog) const;
        int accept(int port) const;
        void handle_connection(Socket& new_connection) const;
        void respond(Socket& new_connection, Request request) const;
        void run(int timeout_in_milliseconds);
        void handle_events(int& number_of_listening_ports);
        void accept_new_connections(int& index, int& number_of_listening_sockets);
        void handle_incoming(int& index);
        void close_connection(int index);

        const Socket& socket() const;
        int port() const;

    private:
        std::map<Socket, s_address> _listening_sockets;
        Poll    _poll;

}; // Class Server

} // NAMESPACE ws

#endif // __SERVER_HPP__
