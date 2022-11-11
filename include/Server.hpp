#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include <string.h>
#include <iostream>

#include <unistd.h>
#include <csignal>
#include <vector>
#include <map>

#include "utility.hpp"
#include "http_tokens.hpp"
#include "sockets.hpp"
#include "Config.hpp"
#include "TCP_Connection.hpp"
#include "PollDescriptors.hpp"

namespace ws {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Debugging

// if set, system(lsof -i -P -n | grep webserv) gets redirected to file "./stats"
// for every poll loop :

// #define TCP_LOG_MAC_OS 1 
#ifndef TCP_LOG_MAC_OS_MAX_ENTRIES
# define TCP_LOG_MAC_OS_MAX_ENTRIES 100
#endif


// #ifndef DEBUG_EVENTS
//  #define DEBUG_EVENTS 1 // comment out to avoid events log
// #endif

#ifdef DEBUG
 #ifndef WS_events_debug
  #ifdef DEBUG_EVENTS
//  #define WS_events_debug(message) (std::cout << __FILE__ << ": " << message << std::endl)
//  #define WS_events_debug(message) (std::cout << __FILE__ << ":" << __LINE__ << " " << message << std::endl)
   #define WS_events_debug(message) (std::cout << message << std::endl)
   #define WS_events_debug_n(message) (std::cout << message)
  #else
   #define WS_events_debug(message) ((void) 0)
   #define WS_events_debug_n(message) ((void) 0)
  #endif
 #endif
#else
 #define WS_events_debug(message) ((void) 0)
 #define WS_events_debug_n(message) ((void) 0)
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CLASS Server
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Server core constants

#define TIMEOUT_MS 1000
#define BACKLOG 0
#define MAX_POLLFD_NB 1024 // hard limit macos & linux

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Server {

    // Member classes - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    public:

        class SystemError: public ws::exception {
            virtual const char* what() const throw() { return ("Server system error"); }
        };

    private:

        class TCP_ConnectionMap {
            private:
                std::map<int, TCP_Connection> _map; // mapped by fd
                const Server& _server;

            public:
                TCP_ConnectionMap(const Server& server);
                ~TCP_ConnectionMap();
                TCP_Connection& operator[](int fd);
                const TCP_Connection& operator[](int fd) const;

                int add_new(const TCP_IP6_ListeningSocket& l_socket);
                void remove(int fd);
                void print();
        };

        friend class TCP_ConnectionMap;

    // Member attributes - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    private:

        const std::vector<ws::config_data>&     _config_server_blocks;
        std::vector<TCP_IP6_ListeningSocket>    _listening_sockets;
        PollDescriptors                         _fd_pool;
        TCP_ConnectionMap                       _connections;
        http::Tokens                            _tokens;

    // Member types - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        typedef void (*_sig_func)(int);

    public:

    // Constr/destr - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        Server(const std::vector<ws::config_data>& config_server_blocks);
        ~Server();

    private:

    // Signal processing (Server_signals.cpp) - - - - - - - - - - - - - - - - - -

        static _sig_func signal(int signo, _sig_func func);
        static void sigint(int signo);
        static void sigquit(int signo);
        static void sigterm(int signo);
        static void sigpipe(int signo);

    public:

    // Core - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        void run();

    private:

        void configure_listening_sockets();
        void listen(const int backlog) const;
        void handle_events_incoming(int& events);
        void accept_on_listening_socket(int id);
        void handle_events_connections(int& events);
        void handle_connection(int id);
        void close_connection(int id);
        void half_close_connection(int id);
        void log_pool_id_events(int id);

}; // Class Server

} // NAMESPACE ws
#endif
