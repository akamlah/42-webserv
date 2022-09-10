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
    int temp = 1;

    if (setsockopt(_socket.fd, SOL_SOCKET,  SO_REUSEADDR, (char *)&temp, sizeof(temp)) < 0)
        throw_print_error(SystemError(), "setsockopt() failed");
    if (ioctl(_socket.fd, FIONBIO, (char *)&temp) < 0)
        throw_print_error(SystemError(), "ioctl() failed");
    memset(_buffer, 0, BUFFER_SIZE);
    memset(&_address, 0, sizeof(_address));
    _address.sin6_family = AF_INET6; // as option ?
    memcpy(&_address.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    _address.sin6_port = htons(_port);
}

Server::~Server() {}

void Server::listen(const int backlog) const {
    try {
        _socket.bind(_address);
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        throw_print_error(SystemError(), "Bad server socket address");
    }
    if (::listen(_socket.fd, backlog) < 0)
        throw_print_error(SystemError(), "Server unable to listen for connections");
    std::cout << CYAN << "Server listening on port " << _port << NC << std::endl;
    std::cout << CYAN << "Waiting for connections..." << NC << std::endl;
}

int Server::accept() const {
    int new_conn_fd;
    struct sockaddr_in6 client_address;
    socklen_t client_length = sizeof(client_address);
    new_conn_fd = ::accept(_socket.fd, (struct sockaddr *)&client_address, &client_length);
    return new_conn_fd;
}

void Server::handle_connection(Socket& new_connection) const {
    // 1 parse request
    std::cout << CYAN << "server received data from : " << new_connection.fd << NC << std::endl;
    try{
        Request new_request(new_connection); // paring inside
        // 2 send response
        respond(new_connection, new_request);
    }
    // temporary
    catch (ws::exception& e) {
        std::cout << "BAD REQUEST" << std::endl; /* bad request error */
        throw;
    }
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

void    Server::run(int timeout)
{
    int incoming_fd;
    ssize_t ret;
    // ssize_t len;
    int current_size = 0;
    bool end_server = false, close_conn = false, compress_array = false;
    Socket new_conn;

    _poll.set_timeout(timeout);
    _poll.add_to_poll(_socket.fd, POLLIN, 0);
    do{
        if (DEBUG) 
            std::cout << "Waiting on poll()..." << std::endl;
        _poll.poll();
        current_size = _poll._fds.size();
        for (int i = 0; i < current_size; ++i)
        {
            if (_poll._fds[i].elem.revents == 0)
                continue;
            if (_poll._fds[i].elem.revents != POLLIN)
            {
                if (DEBUG) 
                    std::cout << "  Error! revents = " << _poll._fds[i].elem.revents << std::endl;
                close(_poll._fds[i].elem.fd);
                _poll._fds[i].elem.fd = -1;
                compress_array = true;
                continue;
            }
            if (_poll._fds[i].elem.fd == _socket.fd)
            {
                if (DEBUG)
                    std::cout << "Listening socket is readable" << std::endl;
                do
                {
                    incoming_fd = accept();
                    if (incoming_fd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            if (DEBUG)
                                std::cerr << "accept() failed" << std::endl;
                            end_server = true;
                        }
                        break;
                    }
                    _poll.add_to_poll(incoming_fd, POLLIN);
                } while (incoming_fd != -1);
            }
            else
            {

                if (DEBUG)
                std::cout << "Descriptor " << _poll._fds[i].elem.fd << " is readable" << std::endl;
                close_conn = false;
                do
                {
                    new_conn = Socket(_poll._fds[i].elem.fd);
                    ret = recv(_poll._fds[i].elem.fd, _buffer, BUFFER_SIZE - 1, 0);
                    if (ret < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            if (DEBUG)
                                std::cerr << "  recv() failed" << std::endl;
                            close_conn = true;
                        }
                        break;
                    }
                    if (ret == 0)
                    {
                        if (DEBUG)
                            std::cout << "  Connection closed" << std::endl;
                        close_conn = true;
                        break;
                    }
                    if (DEBUG) {   
                    std::cout << CYAN << "Message recieved: ---------\n\n" << NC << _buffer;
                    std::cout << CYAN << "---------------------------\n" << NC << std::endl;
                    }
                    try { handle_connection(new_conn); }
                    catch (ws::exception& e) 
                    { 
                        if (DEBUG)
                            std::cout << "  Connection closed" << std::endl;
                        if (errno != EWOULDBLOCK)
                            close_conn = true;
                        break;
                    }
                    // len = ret;
                    // if (DEBUG)
                    //     std::cout << len << " bytes received" << std::endl;

                    // ret = send(_poll._fds[i].elem.fd, _buffer, len, 0);
                    // if (ret < 0)
                    // {
                    //     if (DEBUG)
                    //         std::cerr << "  send() failed" << std::endl;
                    //     close_conn = true;
                    //     break;
                    // }
                    if (ret > 0) { memset(_buffer, 0, ret); }
                } while (true);
                if (close_conn)
                {
                    close(_poll._fds[i].elem.fd);
                    _poll._fds[i].elem.fd = -1;
                    compress_array = true;
                }
                if (ret > 0) { memset(_buffer, 0, ret); }
            }
        }
        if (compress_array)
        {
            compress_array = false;
            _poll.compress_array();
        }
    } while (end_server == false);
    memset(_buffer, 0, BUFFER_SIZE);
    _poll.close_all();
} 

// accessors
const Socket& Server::socket() const { return (_socket); }
int Server::port() const { return (_port); }

} // NAMESPACE ws

