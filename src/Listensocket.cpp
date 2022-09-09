#include "../include/Listensocket.hpp"

namespace ws {

Listensocket::Listensocket(const int domain, const int type, const int protocol)
    : Socket(domain, type, protocol)
{
    int temp = 1;
    if (setsockopt(fd, SOL_SOCKET,  SO_REUSEADDR, (char *)&temp, sizeof(temp)) < 0)
        throw_print_error(SystemError(), "setsockopt() failed");
    if (ioctl(fd, FIONBIO, (char *)&temp) < 0)
        throw_print_error(SystemError(), "ioctl() failed");
}

void Listensocket::bind(const struct sockaddr_in6 server_address) const {
    if (::bind(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        throw_print_error(SystemError(), "Failed to bind socket");
    std::cout << CYAN << "Bound socket" << NC << std::endl;;;;
}

} // NAMESPACE ws
