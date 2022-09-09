#include "../include/Socket.hpp"

namespace ws {

const char* Socket::SystemError::what() const throw() {
    return ("Socket system error");
}

Socket::Socket() {}

Socket::Socket(const int domain, const int type, const int protocol): fd(socket(domain, type, protocol)) {
    if (fd < 0)
        throw_print_error(SystemError(), "Failed to create socket");
}

Socket::Socket(const Socket& other) : fd(other.fd) 
{
    if (fd < 0)
        throw_print_error(SystemError(), "Failed to copy socket");
}

Socket::~Socket() { close(fd); } // [ ! ]

} // NAMESPACE ws
