/* ************************************************************************ */
/*																			*/
/*								Class: Server								*/
/*																			*/
/* ************************************************************************ */

#include "../include/Server.hpp"
#include "../include/utility.hpp"

#if DEBUG
#include <stdio.h>
#endif

// bool trythis = true;

namespace ws {

const char* Server::SystemError::what() const throw() {
    return ("Server system error");
}

// --------------------------------------------------------------------------------------------------------

Server::Server(const std::vector<ws::config_data>& all_config) : _all_config(all_config)
{
    int temp = 1;
    int fd = -1;

    s_address address;
    size_t number_of_servers = all_config.size();
    for (size_t i = 0; i < number_of_servers; ++i)
    {
        size_t number_of_ports = all_config[i].ports.size();
        for(size_t j = 0; j < number_of_ports; ++j)
        {
            fd = ::socket(AF_INET6, SOCK_STREAM, 0);
            if (fd < 0)
                throw_print_error(SystemError(), "Failed to create socket");
            if (setsockopt(fd, SOL_SOCKET,  SO_REUSEADDR, (char *)&temp, sizeof(temp)) < 0)
            {
                std::cout << "Failed to listen on: " << fd << std::endl;
                std::cout << "Port ignored" << std::endl;
                continue;
            }
            if (ioctl(fd, FIONBIO, (char *)&temp) < 0)
            {
                std::cout << "ioctl() failed on: " << fd << std::endl;
                std::cout << "Port ignored" << std::endl;
                continue;
            }
            _listening_ports.insert(std::make_pair(fd, address));
            (_listening_ports.find(fd)->second._address).sin6_family = AF_INET6;
            (_listening_ports.find(fd)->second._address).sin6_port = htons(all_config[i].ports[j]);
            _port_server.insert(std::make_pair(fd, i));
        }
    }
        #if DEBUG
        for (std::map<int,int>::iterator it = _port_server.begin(); it != _port_server.end(); ++it) {
            std::cout << "fd: " << it->first << "\tport: " 
            << ntohs(_listening_ports.find(it->first)->second._address.sin6_port) 
            << "\tblock: " << it->second << std::endl;
        }
        #endif
}

Server::~Server() {
    _poll.close_all();
}

// --------------------------------------------------------------------------------------------------------

void Server::listen(const int backlog) const {
    
    for(std::map<int, s_address>::const_iterator iter = _listening_ports.begin(); iter != _listening_ports.end(); ++iter)
    {
        if (::bind(iter->first, (struct sockaddr *) &(iter->second._address), sizeof(struct sockaddr_in6)) < 0)
            throw_print_error(SystemError(), "Failed to bind socket");
        if (::listen(iter->first, backlog) < 0)
            throw_print_error(SystemError(), "Server unable to listen for connections");
        std::cout << CYAN << "Server listening on port " << ntohs(iter->second._address.sin6_port) << NC << std::endl;
    }
}

void    Server::run(int timeout)
{
    listen(BACKLOG);
    _poll.set_timeout(timeout);
    for(std::map<int, s_address>::const_iterator iter = _listening_ports.cbegin(); iter != _listening_ports.cend(); ++iter)
        _poll.add_to_poll(iter->first, POLLIN, 0);

    while (!_listening_ports.empty()) {
            #if DEBUG
            std::cout << "Waiting on poll()..." << std::endl;
            #endif
        _poll.poll();
            #if DEBUG
            std::cout << "polled" << std::endl;
            #endif
        handle_events();
        _poll.compress();
    }
} 

void Server::handle_events() {
    size_t    current_size = _poll.fds.size();
        #if DEBUG
        std::map<int, http::Connection>::iterator it;
        for (it = _connections.begin(); it != _connections.end(); it++)
            std::cout << "fd:" << it->first << " conn status " << it->second.status() << std::endl;
        #endif
    for (size_t poll_index = 0; poll_index < current_size; ++poll_index) {
            #if DEBUG
            printf("  fd=%d; events: %s%s%s\n", _poll.get_fd(poll_index),
            (_poll.fds[poll_index].elem.revents & POLLIN)  ? "POLLIN "  : "",
            (_poll.fds[poll_index].elem.revents & POLLHUP) ? "POLLHUP " : "",
            (_poll.fds[poll_index].elem.revents & POLLERR) ? "POLLERR " : "");
            #endif
        if (_poll.fds[poll_index].elem.revents != 0) {
            if (poll_index < _listening_ports.size())
                accept_new_connections(poll_index);
            else
                handle_connection(poll_index);
        }
    }
}

// --------------------------------------------------------------------------------------------------------

void Server::accept_new_connections(const int poll_index) {
    int listening_fd = _poll.get_fd(poll_index);
        #if DEBUG
        std::cout << "Listening socket " << listening_fd << " is readable." << std::endl;
        #endif
    http::Connection incoming(_tokens, _all_config[_port_server.find(listening_fd)->second]);
    incoming.establish(listening_fd);
    if (!incoming.is_good()) {
            #if DEBUG
            std::cerr << "accept() failed on fd " << listening_fd
            << "; Port " << ntohs(_listening_ports.find(listening_fd)->second._address.sin6_port) 
            << "; errno = " << errno << std::endl;
            #endif
        return ;
        // break;
    }
    _poll.add_to_poll(incoming.fd(), POLLIN);
    _connections.insert(std::make_pair(incoming.fd(), incoming));
}

void Server::handle_connection(const int poll_index) {
    // + timeout ?
    // + try catch ?
        #if DEBUG
        std::cout << " Polled fd = " << _poll.get_fd(poll_index) << std::endl;
        printf("  fd=%d; revents: %s%s%s\n", _poll.get_fd(poll_index),
        (_poll.fds[poll_index].elem.revents & POLLIN)  ? "POLLIN "  : "",
        (_poll.fds[poll_index].elem.revents & POLLHUP) ? "POLLHUP " : "",
        (_poll.fds[poll_index].elem.revents & POLLERR) ? "POLLERR " : "");
        #endif
    if (((_poll.fds[poll_index].elem.revents & POLLIN) && !(_poll.fds[poll_index].elem.revents & POLLERR)) 
        && _connections.find(_poll.get_fd(poll_index))->second.is_persistent()) {
        _connections.find(_poll.get_fd(poll_index))->second.handle(_poll, poll_index);
    }
    if (_poll.fds[poll_index].elem.revents & POLLHUP) {
        close_connection(poll_index);
        return ;
    }
    else
        close_connection(poll_index);
}

void Server::close_connection(const int poll_index) {
    int fd = _poll.get_fd(poll_index);
        // #if DEBUG
        std::cout << YELLOW << "Close fd: " << fd << NC << std::endl;
        // #endif
    _connections.erase(fd);
    close(fd);
    _poll.fds[poll_index].elem.fd = -1;
    _poll.compress_array = true;
    _poll.compress();

}

} // NAMESPACE ws
