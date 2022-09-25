/* ************************************************************************ */
/*                                                                          */
/*                              Class: Server                               */
/*                                                                          */
/* ************************************************************************ */

#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include "Socket.hpp"

#include "utility.hpp"
#include "Poll.hpp"
#include "Connection.hpp"

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
        void run(int timeout_in_milliseconds = -1);
        void handle_events();
        void accept_new_connections(const int poll_index);
        void handle_connection(const int poll_index);
        void close_connection(const int poll_index);

        const Socket& socket() const;
        int port() const;

    private:
        std::map<Socket, s_address> _listening_sockets;
        Poll    _poll;
        int _number_of_listening_ports;
        std::map<int, http::Connection> _connections;
        // config_data myConfigdata;

}; // Class Server

} // NAMESPACE ws

#endif // __SERVER_HPP__
