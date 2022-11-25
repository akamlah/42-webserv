#ifndef WS_TCP_CONNECTION
#define WS_TCP_CONNECTION

#include <iostream>
#include <sstream>
# include <sys/time.h>

#include "utility.hpp"
#include "http_tokens.hpp"
#include "sockets.hpp"
#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"

namespace ws {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Debugging

// Activate/deactivate debugging for this file if DEBUG flag is set
// #ifndef DEBUG_CONNECTIONS
//  #define DEBUG_CONNECTIONS 1 // 0
// #endif

#if DEBUG
//  #ifndef WS_connection_debug
  #ifdef DEBUG_CONNECTIONS
//  #define WS_connection_debug(message) (std::cout << __FILE__ << ": " << message << std::endl)
//  #define WS_connection_debug(message) (std::cout << __FILE__ << ":" << __LINE__ << " " << message << std::endl)
   #define WS_connection_debug(message) (std::cout << message << std::endl)
   #define WS_connection_debug_n(message) (std::cout << message)
  #else
   #define WS_connection_debug(message) ((void) 0)
   #define WS_connection_debug_n(message) ((void) 0)
  #endif
//  #endif
#else
 #define WS_connection_debug(message) ((void) 0)
 #define WS_connection_debug_n(message) ((void) 0)
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TCP CONNECTION class
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class TCP_ConnectionSocket;

class TCP_Connection {

    public:

        enum cn_state {
            ESTABLISHED = 0 ,
            PARTIAL_RESP ,
            CLOSE_WAIT ,
            TIMED_OUT ,
            HTTP_ERROR ,
            RD_ERROR ,
            WR_ERROR
        };

    private:

        // CONNECT
        TCP_IP6_ConnectionSocket _socket;
        const config_data& _conf;
        const http::Tokens& _tokens;
        cn_state _state;

        // READ
        char _buffer[BUFFER_SIZE]; // correct size ? // [ ! ] refresh correctly
        size_t _brecv; // total bytes recieved so far on buffer
        http::Request _request;

        // WRITE
        size_t _btosend; // total bytes to send // [ ! ] refresh correctly
        size_t _bsent; // total bytes sent so far // [ ! ] refresh correctly
        // std::stringstream _s; // remove later and replace with _response. // [ ! ] refresh correctly
        std::string _response_str;

        // TIMEOUT
        long _time_last;

    public:

    // Constr/destr/cpy  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        TCP_Connection(const TCP_IP6_ListeningSocket& l_socket, const config_data& conf, const http::Tokens& tokens);
        TCP_Connection& operator=(const TCP_Connection& other);
        TCP_Connection(const TCP_Connection& other);
        ~TCP_Connection();
    
    // Access - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        const TCP_IP6_ConnectionSocket& socket() const ;
        cn_state& state();
        std::string state_to_str() const ;
    
    // Core - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        void rdwr();

        void read();
        void write();

        bool is_timedout();
        void refresh_timer();

    private:

        void prepare_read_buffer();
        void prepare_response();
        config_data createNewlocationConfig();

}; // class Connection

} // namespace ws
#endif // WS_TCP_CONNECTION
