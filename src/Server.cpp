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

Server::~Server() {
    _poll.close_all();
}

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
} 

void Server::handle_events()
{
    int    current_size = _poll.fds.size();

    if (DEBUG) {
        std::map<int, http::Connection>::iterator it;
        for (it = _connections.begin(); it != _connections.end(); it++){
            std::cout << "fd:" << it->first << " conn status " << it->second.status() << std::endl;
        }
    }

    for (int poll_index = 0; poll_index < current_size; ++poll_index)
    {
        if (_poll.fds[poll_index].elem.revents == 0) {
            if (DEBUG)
                std::cout << "revents = 0" << std::endl;
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

void Server::accept_new_connections(const int poll_index)
{
    http::Connection incoming;
    int listening_fd = _poll.get_fd(poll_index);

    if (DEBUG)
        std::cout << "Listening socket " << listening_fd << " is readable." << std::endl;
    do {
        incoming.establish(listening_fd);
        if (!incoming.is_good()) {
            if (errno != EWOULDBLOCK) {
                if (DEBUG) {
                    std::cerr << "accept() failed on listening port " << listening_fd << std::endl;
                    std::cerr << "Port has been discarded from array" << std::endl;
                }
                close_connection(poll_index);
                --_number_of_listening_ports;
                break;
            }
            else {
                if (DEBUG)
                    std::cout << "EWOULDBLOCK" << std::endl;
                break;
            }
        }
        _poll.add_to_poll(incoming.fd(), POLLIN);
        _connections.insert(std::make_pair(incoming.fd(), incoming));
        if (DEBUG) {
            std::cout << "NEW MAP:" << std::endl;
            std::map<int, http::Connection>::iterator it;
            for (it = _connections.begin(); it != _connections.end(); it++){
                std::cout << "fd:" << it->first << " conn status " << it->second.status() << std::endl;
            }
            std::cout << std::endl;
        }
    } while (incoming.is_good());
}

void Server::handle_connection(const int poll_index)
{
    // + timeout ?
    // + try catch ?
    _connections[_poll.get_fd(poll_index)].handle();
    if (!_connections[_poll.get_fd(poll_index)].is_persistent()) {
        close_connection(poll_index);
        if (DEBUG)
            std::cout << "--->>>> Connection closed" << std::endl;
    }
}

void Server::close_connection(const int poll_index)
{
    int fd = _poll.get_fd(poll_index);
    if (DEBUG)
        std::cout << "CLOSE ! fd: " << fd << std::endl;
    _connections.erase(fd);
    close(fd);
    _poll.fds[poll_index].elem.fd = -1;
    _poll.compress_array = true;
}

} // NAMESPACE ws
