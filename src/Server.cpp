/* ************************************************************************ */
/*																			*/
/*								Class: Server								*/
/*																			*/
/* ************************************************************************ */

#include "../include/Server.hpp"
#include "../include/utility.hpp"

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
        (iter->second._address).sin6_family = AF_INET6;
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
    _listening_sockets[server_socket]._address.sin6_family = AF_INET6;
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
    _poll.set_timeout(timeout);
    for(std::map<Socket, s_address>::const_iterator iter = _listening_sockets.cbegin(); iter != _listening_sockets.cend(); ++iter)
        _poll.add_to_poll(iter->first.fd, POLLIN, 0);
    do{
        if (DEBUG)
            std::cout << "Waiting on poll()..." << std::endl;
        _poll.poll();
        if (DEBUG)
            std::cout << "polled" << std::endl;
        handle_events();
        _poll.compress();

    } while (_number_of_listening_ports);
    _poll.close_all();
} 

void Server::handle_events()
{
    if (DEBUG)
        std::cout << "handle events" << std::endl;
    int    current_size = _poll.fds.size();

    if (DEBUG) {
        std::map<int, http::Connection>::iterator it;
        for (it = _connections.begin(); it != _connections.end(); it++){
            std::cout << "fd:" << it->first << " conn status " << it->second.status() << std::endl;
        }
    }

    for (int poll_index = 0; poll_index < current_size; ++poll_index)
    {
<<<<<<< HEAD
        if (DEBUG)
            std::cout << "iter on index " << poll_index << std::endl;
        if (_poll.fds[poll_index].elem.revents == 0)
=======
        if (_poll.fds[poll_index].elem.revents == 0) {
            if (DEBUG)
                std::cout << "revents = 0" << std::endl;
>>>>>>> 41dd9f992646fc1ecf64036157c64361190e588c
            continue;
        }
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
int server_accept(int fd) {
    int new_conn_fd;
    struct sockaddr_in6 client_address;
    socklen_t client_length = sizeof(client_address);
    new_conn_fd = ::accept(fd, (struct sockaddr *)&client_address, &client_length);
    return new_conn_fd;
}

void Server::accept_new_connections(const int poll_index)
{
    std::cout << "accept index: " << poll_index << std::endl;
    int fd = _poll.fds[poll_index].elem.fd;
    int incoming_fd;
    if (DEBUG)
        std::cout << "Listening socket " << _poll.fds[poll_index].elem.fd << " is readable." << std::endl;
    do {

        incoming_fd = server_accept(fd);
        http::Connection c(incoming_fd);
        // c.establish(fd);
        if (incoming_fd < 0) {
            if (errno != EWOULDBLOCK) {
                if (DEBUG) {
                    std::cerr << "accept() failed on listening port " << _poll.fds[poll_index].elem.fd << std::endl;
                    std::cerr << "Port has been discarded from array" << std::endl;
                }
                close_connection(poll_index);
                --_number_of_listening_ports;
                break;
            }
            else {
                if (DEBUG)
                    std::cout << "EWOULDLOCK" << std::endl;
                break;
                // return ;
            }
        }
        // _poll.add_to_poll(incoming_fd, POLLIN);
        
        _poll.add_to_poll(c.fd(), POLLIN);
        _connections.insert(std::make_pair(c.fd(), c));
        if (DEBUG) {
            std::cout << "NEW MAP:" << std::endl;
            std::map<int, http::Connection>::iterator it;
            for (it = _connections.begin(); it != _connections.end(); it++){
                std::cout << "fd:" << it->first << " conn status " << it->second.status() << std::endl;
            }
            std::cout << std::endl;
        }
    } while (incoming_fd != -1);
    // } while (c.good());
}

void Server::handle_connection(const int poll_index)
{
    // + timeout ?
    // + try catch ?
    _connections[_poll.fds[poll_index].elem.fd].handle();
    if (!_connections[_poll.fds[poll_index].elem.fd].is_persistent()) {
        close_connection(poll_index);
        if (DEBUG)
            std::cout << "  Connection closed" << std::endl;
    }
}

void Server::close_connection(const int poll_index)
{
    if (DEBUG)
        std::cout << "CLOSE ! fd: " << _poll.fds[poll_index].elem.fd << std::endl;
    _connections.erase(_poll.fds[poll_index].elem.fd);
    close(_poll.fds[poll_index].elem.fd);
    _poll.fds[poll_index].elem.fd = -1;
    _poll.compress_array = true;
}

} // NAMESPACE ws
