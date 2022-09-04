/* ************************************************************************ */
/*                                                                          */
/*                              Class: Request                              */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Request.hpp"

namespace ws {

// bad request exceptions

Request::Request(const Socket& new_connection) {
    // try parse,
    // catch bad request

    // temporary
    char buffer[1024];
    bzero(buffer,256);
    size_t bytes_read;
    if ((bytes_read = read(new_connection.fd, buffer, 1023)) < 0) {
        std::cout << "BAD READ" << std::endl;
    std::cout << CYAN << "Message recieved: ---------\n\n" << NC << buffer;
    std::cout << CYAN << "---------------------------\n" << NC << std::endl;
    }
}

Request::~Request() {}

} // NAMESPACE ws
