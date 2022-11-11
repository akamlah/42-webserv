#ifndef WS_SOCKETS
# define WS_SOCKETS

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>

#include "utility.hpp"

namespace ws {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Debugging

// #ifndef DEBUG_SOCKETS
//  #define DEBUG_SOCKETS 1 // comment out to avoid events log
// #endif

#if DEBUG
 #ifndef WS_sockets_debug
  #ifdef DEBUG_SOCKETS
//  #define WS_sockets_debug(message) (std::cout << __FILE__ << ": " << message << std::endl)
//  #define WS_sockets_debug(message) (std::cout << __FILE__ << ":" << __LINE__ << " " << message << std::endl)
   #define WS_sockets_debug(message) (std::cout << message << std::endl)
  #else
   #define WS_sockets_debug(message) ((void) 0)
  #endif
 #endif
#else
 #define WS_sockets_debug(message) ((void) 0)
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SOCKET - Socket base
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class TCP_IP6_Socket_base {

    protected:

        int _fd;
        int _server_id;
        struct sockaddr_in6 _address;

    public:

        TCP_IP6_Socket_base(int fd = -1) : _fd(fd) {
            memset(&_address, 0, sizeof(_address));
            WS_sockets_debug("A sock: constr " << _fd);
        }

        TCP_IP6_Socket_base(const TCP_IP6_Socket_base& other) {
            memset(&_address, 0, sizeof(_address));
            _fd = other._fd;
            _server_id = other._server_id;
            memcpy(&_address, &other._address, sizeof(_address));
            WS_sockets_debug("A sock: cpy constr " << _fd);
        }
        
        virtual ~TCP_IP6_Socket_base() {}

        TCP_IP6_Socket_base& operator=(const TCP_IP6_Socket_base& other) {
            _fd = other._fd;
            _server_id = other._server_id;
            memcpy(&_address, &other._address, sizeof(_address));
            WS_sockets_debug("A sock: cpy assign ope " << _fd);
            return (*this);
        }

        virtual int configure() = 0;

        int fd() const { return(_fd); }
        int port() const { return(ntohs(_address.sin6_port)); }
        int server_id() const { return(_server_id); }

        struct sockaddr_in6& address() { return(_address); }
        const struct sockaddr_in6& address() const { return(_address); }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SOCKET - Listening Socket
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class TCP_IP6_ListeningSocket : public TCP_IP6_Socket_base {

    public:
        
        TCP_IP6_ListeningSocket(int server_id, int port) {
            _server_id = server_id;
            if ((_fd = ::socket(AF_INET6, SOCK_STREAM, 0)) < 0)
                throw_print_error(std::exception(), "Failed to create socket");
            memset(&_address, 0, sizeof(_address));
            memcpy(&_address.sin6_addr, &in6addr_any, sizeof(in6addr_any));
            _address.sin6_family = AF_INET6;
            _address.sin6_port = htons(port);
            WS_sockets_debug("List sock: constr " << _fd);
        }

        ~TCP_IP6_ListeningSocket() {}

        int configure() {
            int n = 1;
            if ((::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n))) < 0)
                return (-1);
            if ((::ioctl(_fd, FIONBIO, (char *)&n)) < 0)
                return (-1);
            return (0);
        }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SOCKET - Connection Socket
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class TCP_IP6_ConnectionSocket : public TCP_IP6_Socket_base {

    public:

        // user has to check if fd > 0 after instatiation !
        TCP_IP6_ConnectionSocket(const TCP_IP6_ListeningSocket& listening_so) {
            _server_id = listening_so.server_id();
            memset(&_address, 0, sizeof(_address));
            socklen_t address_length = sizeof(_address);
            _fd = ::accept(listening_so.fd(), (struct sockaddr *)(&_address), &address_length);
            WS_sockets_debug("Conn sock: constr " << _fd);
        }

        ~TCP_IP6_ConnectionSocket() {}

        int configure() {
            int n = 1;
            int error;
            error = setsockopt(_fd, SOL_SOCKET, SO_NOSIGPIPE, &n, sizeof(n));
            if (error)
                return(error);

            error = fcntl(_fd, F_SETFL, O_NONBLOCK);
            if (error)
                return(error);
            
            struct timeval tv;
            tv.tv_sec = 7;
            tv.tv_usec = 0;
            error = setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            if (error)
                return(error);
            error = setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
            if (error)
                return(error);

            return (error);
        }
};

} // namespace ws
#endif // WS_SOCKETS
