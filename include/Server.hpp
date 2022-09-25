/* ************************************************************************ */
/*                                                                          */
/*                              Class: Server                               */
/*                                                                          */
/* ************************************************************************ */

#ifndef __SERVER_HPP__
# define __SERVER_HPP__


#include "utility.hpp"
#include "Poll.hpp"
#include "Config.hpp"
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

        Server(const std::vector<ws::config_data>& configDataAll);
        ~Server();

        void listen(const int backlog) const;
        void run(int timeout_in_milliseconds = -1);
        void handle_events();
        void accept_new_connections(const int poll_index);
        void handle_connection(const int poll_index);
        void close_connection(const int poll_index);

        int port() const;

    private:

        std::map<int, s_address>            _listening_ports;
        Poll                                _poll;
        std::map<int, http::Connection>     _connections;
        std::map<int, int>                  _port_server;
        const std::vector<ws::config_data>& _all_config;
        http::Tokens                        _tokens;
        // init token maps here to do it just once and not every time they are called.




}; // Class Server

} // NAMESPACE ws

#endif // __SERVER_HPP__
