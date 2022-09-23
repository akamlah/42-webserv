#include "../include/Socket.hpp"

namespace ws {

const char* Socket::SystemError::what() const throw() {
    return ("Socket system error");
}

Socket::Socket() : fd(-1), port(-1) { }

Socket::Socket(const int domain, const int type, const int protocol, const int port)
    : fd(socket(domain, type, protocol)), port(port) {
    if (fd < 0)
        throw_print_error(SystemError(), "Failed to create socket");
}

Socket::Socket(const Socket& other) 
    : fd(other.fd), port(other.port) 
{
    if (fd < 0)
        throw_print_error(SystemError(), "Failed to copy socket");
}

Socket::Socket(int new_fd) : fd(new_fd), port(-1)
{
    if (fd < 0)
        throw_print_error(SystemError(), "Failed to copy socket");

}

bool Socket::operator<(const Socket& other) const
{
    return (fd < other.fd);
}

void Socket::bind(const struct sockaddr_in6 server_address) const {
    if (::bind(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        throw_print_error(SystemError(), "Failed to bind socket");
}

void Socket::connect(const struct sockaddr_in6 server_address) const {
    if (::connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        throw_print_error(SystemError(), "Failed to connect socket");
    std::cout << CYAN << "Connected socket" << NC << std::endl;;;;
}

Socket::~Socket() { std::cout << "Socket destroyed" << std::endl;
} // [ ! ]

} // NAMESPACE ws
