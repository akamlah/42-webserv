/* ************************************************************************ */
/*                                                                          */
/*                              Class: Request                              */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Request.hpp"

namespace ws {

const char* Request::BadRead::what() const throw() {
    return ("Bad read!");
}
const char* Request::EofReached::what() const throw() {
    return ("EOF reached!");
}

Request::Request(const Socket& new_connection) {

    char buffer[1024];
    bzero(buffer,256);
    ssize_t bytes_read;
    bytes_read = recv(new_connection.fd, buffer, 1023, 0);

    if (bytes_read < 0)
        throw EofReached();
        // throw BadRead();
    if (bytes_read == 0)
        throw EofReached();
    
    std::cout << CYAN << "server received request from : " << new_connection.fd << NC << std::endl;
    std::cout << CYAN << "Message recieved: ---------\n\n" << NC << buffer << std::endl ;
    std::cout << CYAN << "---------------------------\n" << NC << std::endl;

    if (DEBUG)
    std::cout << CYAN << "Executing request from : " << new_connection.fd << NC << std::endl;
}

Request::~Request() {}

} // NAMESPACE ws
