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
Server::Server(Listensocket& server_socket, int port): _socket(server_socket), _port(port) {
    memset(&_address, 0, sizeof(_address));
    _address.sin6_family = AF_INET6; // as option ?
    memcpy(&_address.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    _address.sin6_port = htons(_port);
    // _address.sin6_addr.s6_addr = htonl(INADDR_ANY); // as option ?
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
    std::cout << CYAN << "Server listening" << NC << std::endl;
    std::cout << CYAN << "Waiting for connections...\n" << NC << std::endl;
}

int Server::accept() const {
    int new_conn_fd;
    struct sockaddr_in6 client_address;
    socklen_t client_length = sizeof(client_address);
    new_conn_fd = ::accept(_socket.fd, (struct sockaddr *)&client_address, &client_length);
    std::cout << " on port " << _port << NC << std::endl;
    return new_conn_fd;
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

void    Server::run(int timeout)
{
    int new_fd, close_conn;
    int ret, len;
    int compress_array = false;
    int end_server = false;
    int current_size = 0;

    _poll.set_timeout(timeout);
    _poll.add_to_poll(_socket.fd, POLLIN, 0);
    do{
        std::cout << "Waiting on poll()..." << std::endl;
        _poll.poll();
        current_size = _poll._fds.size();
        for (int i = 0; i < current_size; ++i)
        {
            if (_poll._fds[i].elem.revents == 0)
                continue;
            if (_poll._fds[i].elem.revents != POLLIN)
            {
                std::cout << "  Error! revents = " << _poll._fds[i].elem.revents << std::endl;
                close(_poll._fds[i].elem.fd);
                _poll._fds[i].elem.fd = -1;
                compress_array = true;
                continue;
            }
            if (_poll._fds[i].elem.fd == _socket.fd)
            {
                std::cout << "Listening socket is readable" << std::endl;
                do
                {
                    new_fd = accept();
                    if (new_fd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            std::cerr << "accept() failed" << std::endl;
                            end_server = true;
                        }
                        break;
                    }
                    std::cout << "  New incoming connection - " << new_fd << std::endl;
                    _poll.add_to_poll(new_fd, POLLIN);
                } while (new_fd != -1);
                    std::cout << new_fd << "Passed here" << std::endl;
            }
            else
            {
                std::cout << "Descriptor " << _poll._fds[i].elem.fd << " is readable" << std::endl;
                close_conn = false;
                do
                {
                    ret = recv(_poll._fds[i].elem.fd, _buffer, BUFFER_SIZE, 0);
                    if (ret < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            std::cerr << "  recv() failed" << std::endl;
                            close_conn = true;
                        }
                        break;
                    }
                    if (ret == 0)
                    {
                        std::cout << "  Connection closed" << std::endl;
                        close_conn = true;
                        break;
                    }
                    len = ret;
                    std::cout << len << " bytes received" << std::endl;

                    ret = send(_poll._fds[i].elem.fd, _buffer, len, 0);
                    if (ret < 0)
                    {
                        std::cerr << "  send() failed" << std::endl;
                        close_conn = true;
                        break;
                    }
                } while (true);
                if (close_conn)
                {
                    close(_poll._fds[i].elem.fd);
                    _poll._fds[i].elem.fd = -1;
                    compress_array = true;
                }
            }
        }
        if (compress_array)
        {
            compress_array = false;
            _poll.compress_array();
        }
    } while (end_server == false);
    _poll.close_all();
} 

// accessors
const Listensocket& Server::socket() const { return (_socket); }
int Server::port() const { return (_port); }

} // NAMESPACE ws

