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

Socket::Socket(int new_fd) : fd(new_fd)
{
    if (fd < 0)
        throw_print_error(SystemError(), "Failed to copy socket");

}

void Socket::bind(const struct sockaddr_in6 server_address) const {
    if (::bind(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        throw_print_error(SystemError(), "Failed to bind socket");
    std::cout << CYAN << "Bound socket" << NC << std::endl;;;;
}

void Socket::connect(const struct sockaddr_in6 server_address) const {
    if (::connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        throw_print_error(SystemError(), "Failed to connect socket");
    std::cout << CYAN << "Connected socket" << NC << std::endl;;;;
}

Socket::~Socket() { } // [ ! ]

} // NAMESPACE ws
