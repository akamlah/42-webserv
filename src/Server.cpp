/* ************************************************************************ */
/*																			*/
/*								Class: Server								*/
/*																			*/
/* ************************************************************************ */

#include "../include/Server.hpp"
#include "../include/utility.hpp"
#include "../include/CGI.hpp"

bool trythis = true;

namespace ws {

const char* Server::SystemError::what() const throw() {
    return ("Server system error");
}

// --------------------------------------------------------------------------------------------------------

Server::Server(config_data& configData) {
    int temp = 1;

    for (std::vector<int>::const_iterator iter = configData.ports.cbegin(); iter < configData.ports.cend(); ++iter)
        _listening_sockets.insert(std::make_pair(Socket(AF_INET6, SOCK_STREAM, 0, *iter), s_address()));

    for(std::map<Socket, s_address>::iterator iter = _listening_sockets.begin(); iter != _listening_sockets.end(); ++iter)
    {
        if (setsockopt(iter->first.fd, SOL_SOCKET,  SO_REUSEADDR, (char *)&temp, sizeof(temp)) < 0)
        {
            std::cout << "Failed to listen on: " << iter->first.fd << std::endl;
            std::cout << "Port ignored" << std::endl;
            continue;
        }
        if (ioctl(iter->first.fd, FIONBIO, (char *)&temp) < 0)
        {
            std::cout << "ioctl() failed on: " << iter->first.fd << std::endl;
            std::cout << "Port ignored" << std::endl;
            continue;
        }
        (iter->second._address).sin6_family = AF_INET6; // as option ?
        (iter->second._address).sin6_port = htons(iter->first.port);
    }
    _number_of_listening_ports = _listening_sockets.size();
}

Server::Server(Socket& server_socket, int port) {
    int temp = 1;

    server_socket.port = port;
    _listening_sockets.insert(std::make_pair(server_socket, s_address()));
    if (setsockopt(_listening_sockets.begin()->first.fd, SOL_SOCKET,  SO_REUSEADDR, (char *)&temp, sizeof(temp)) < 0)
        throw_print_error(SystemError(), "setsockopt() failed");
    if (ioctl(_listening_sockets.begin()->first.fd, FIONBIO, (char *)&temp) < 0)
        throw_print_error(SystemError(), "ioctl() failed");
    _listening_sockets[server_socket]._address.sin6_family = AF_INET6; // as option ?
    _listening_sockets[server_socket]._address.sin6_port = htons(port);
    _number_of_listening_ports = _listening_sockets.size();
}

Server::~Server() {}

// --------------------------------------------------------------------------------------------------------

void Server::listen(const int backlog) const {
    
    for(std::map<Socket, s_address>::const_iterator iter = _listening_sockets.begin(); iter != _listening_sockets.end(); ++iter)
    {
        try {
            iter->first.bind(iter->second._address);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            throw_print_error(SystemError(), "Bad server socket address");
        }
        if (::listen(iter->first.fd, backlog) < 0)
            throw_print_error(SystemError(), "Server unable to listen for connections");
        std::cout << CYAN << "Server listening on port " << iter->first.port << NC << std::endl;
    }
}

void    Server::run(int timeout)
{
    // number_of_listening_ports = _listening_sockets.size(); <- is in constructors now
    _poll.set_timeout(timeout);
    for(std::map<Socket, s_address>::const_iterator iter = _listening_sockets.cbegin(); iter != _listening_sockets.cend(); ++iter)
        _poll.add_to_poll(iter->first.fd, POLLIN, 0);
    do{
        if (DEBUG) 
            std::cout << "Waiting on poll()..." << std::endl;
        _poll.poll();
        handle_events();
        _poll.compress();

    } while (_number_of_listening_ports);
    _poll.close_all();
} 

void Server::handle_events()
{
    int    current_size = _poll.fds.size();

    for (int poll_index = 0; poll_index < current_size; ++poll_index)
    {
        std::cout << "iter on index " << poll_index << std::endl;
        if (_poll.fds[poll_index].elem.revents == 0)
            continue;
        if (_poll.fds[poll_index].elem.revents != POLLIN)
        {
            if (DEBUG)
                std::cout << "  Error! revents = " << _poll.fds[poll_index].elem.revents << std::endl;
            close_connection(poll_index);
            continue;
        }
        if (poll_index < _number_of_listening_ports)
            accept_new_connections(poll_index);
        else
            handle_connection(poll_index);
    }
}

// --------------------------------------------------------------------------------------------------------

// void map_connection(const Connection& c) {

// }

void Server::accept_new_connections(const int poll_index)
{
    int fd = _poll.fds[poll_index].elem.fd;
    if (DEBUG)
        std::cout << "Listening socket " << _poll.fds[poll_index].elem.fd << "is readable." << std::endl;
    do {
        http::Connection c;
        c.establish(fd);
        if (!c.good()) {
            if (errno != EWOULDBLOCK) {
                if (DEBUG) {
                    std::cerr << "accept() failed on listening port " << _poll.fds[poll_index].elem.fd << std::endl;
                    std::cerr << "Port has been discarded from array" << std::endl;
                }
                close_connection(poll_index);
                --_number_of_listening_ports;
            }
            break;
        }
        _poll.add_to_poll(c.fd(), POLLIN);
        _connections[poll_index + 1] = c;
    } while (true);
}

void Server::handle_connection(const int poll_index)
{
    // + timeout ?
    // + try catch ?
    _connections[poll_index].handle();
    if (!_connections[poll_index].is_persistent()) {
        close_connection(poll_index);
        std::cout << "  Connection closed" << std::endl;
    }
}

void Server::close_connection(const int poll_index)
{
    close(_poll.fds[poll_index].elem.fd);
    _poll.fds[poll_index].elem.fd = -1;
    _poll.compress_array = true;
    _connections.erase(poll_index);
}

// Do we need these accessors?
// const Socket& Server::socket() const { return (_socket); }
// int Server::port() const { return (_port); }

} // NAMESPACE ws
