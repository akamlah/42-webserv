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

namespace ws {

class Server {

    public:

        class SystemError: public ws::exception {
                virtual const char* what() const throw();
        };

        Server(Socket & server_socket, config_data configData); // change this to constructor with a "config" object later
        // + cpy constr
        // + cpy assign ope
        ~Server();

        void listen(const int backlog) const;
        void accept(Socket& new_connection) const;
        void handle_connection(Socket& new_connection) const;
        void respond(Socket& new_connection, Request request) const;

        const Socket& socket() const;
        int port() const;

    private:

        const Socket _socket;
        const int _port;
        struct sockaddr_in _address;

}; // Class Server

} // NAMESPACE ws

#endif // __SERVER_HPP__
