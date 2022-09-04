/* ************************************************************************ */
/*																			*/
/*								Class: Server								*/
/*																			*/
/* ************************************************************************ */

#include "../include/Server.hpp"

namespace ws {

const char* Server::SystemError::what() const throw() {
    return ("Server system error");
}

// [ ! ] server can listen to multiple ports on nginx -> have different sockets?
// probably best to have just one server constructor with CONFIG
// and a default constructor with a defalut config
// config gets paresd by Master Class ("engine" or "configuration" or something like that, that then passes an object or a string to 
// each server instance)
Server::Server(Socket& server_socket, int port): _socket(server_socket), _port(port) {
    bzero((char *)&_address, sizeof(_address));
    _address.sin_family = AF_INET; // as option ?
    _address.sin_port = htons(_port);
    _address.sin_addr.s_addr = htonl(INADDR_ANY); // as option ?
}

Server::~Server() {}

void Server::listen(const int backlog) const {
    try {
        // review structure... maybe better empty constructor and separate code more than so
        _socket.bind(_address);
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        throw_print_error(SystemError(), "Bad server socket address");
    }
    if (::listen(_socket.fd, backlog) < 0)
        throw_print_error(SystemError(), "Server unable to listen for connections");
    std::cout << CYAN << "Server listening" << NC << std::endl;
    std::cout << CYAN << "Waiting for connections...\n" << NC << std::endl;
}

void Server::accept(Socket& new_connection) const {
    struct sockaddr_in client_address;
    socklen_t client_length = sizeof(client_address);
    new_connection.fd = ::accept(_socket.fd, (struct sockaddr *)&client_address, &client_length);
    if (new_connection.fd < 0)
        throw_print_error(SystemError(), "Error accept");
    std::cout << CYAN << "Server: accepted connection from " << inet_ntoa(client_address.sin_addr);
    std::cout << " on port " << _port << NC << std::endl;
}

void Server::handle_connection(Socket& new_connection) const {
    // 1 parse request
    try{
        Request new_request(new_connection); // paring inside
        // 2 send response
        respond(new_connection, new_request);
    }
    // temporary
    catch (ws::exception& e) { std::cout << "BAD REQUEST" << std::endl; /* bad request error */}
}

void Server::respond(Socket& new_connection, Request request) const {
    // generate response according to request & send it:
    try {
        Response response(request);
        // send(response)
        if (::send(new_connection.fd, response.c_str(), sizeof(response.c_str()), 0) < 0) {
            throw_print_error(SystemError(), "Failed to send response");
        }
        std::cout << CYAN << "Server sent data" << NC << std::endl;
    }
    catch(std::exception& e) { throw_print_error(SystemError()); }
}

// accessors
const Socket& Server::socket() const { return (_socket); }
int Server::port() const { return (_port); }

} // NAMESPACE ws
