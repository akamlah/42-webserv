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

Socket::~Socket() { close(fd); } // [ ! ]

void Socket::bind(const struct sockaddr_in server_address) const {
    if (::bind(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        throw_print_error(SystemError(), "Failed to bind socket");
    std::cout << CYAN << "Bound socket" << NC << std::endl;
}

} // NAMESPACE ws
