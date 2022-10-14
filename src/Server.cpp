/* ************************************************************************ */
/*																			*/
/*								Class: Server								*/
/*																			*/
/* ************************************************************************ */

#include "../include/Server.hpp"
#include "../include/utility.hpp"

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
    if (DEBUG){
        for (std::map<int,int>::iterator it = _port_server.begin(); it != _port_server.end(); ++it)
        {
            std::cout << "fd: " << it->first << "\tport: " 
            << ntohs(_listening_ports.find(it->first)->second._address.sin6_port) 
            << "\tblock: " << it->second << std::endl;
        }
    }
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
        if (DEBUG)
            std::cout << "Waiting on poll()..." << std::endl;
        _poll.poll();
        if (DEBUG)
            std::cout << "polled" << std::endl;
        handle_events();
        _poll.compress();
    }
} 

void Server::handle_events()
{
    size_t    current_size = _poll.fds.size();

    if (DEBUG) {
        std::map<int, http::Connection>::iterator it;
        for (it = _connections.begin(); it != _connections.end(); it++){
            std::cout << "fd:" << it->first << " conn status " << it->second.status() << std::endl;
        }
    }

    for (size_t poll_index = 0; poll_index < current_size; ++poll_index)
    {
        if (_poll.fds[poll_index].elem.revents == 0)
            continue;

//  - - - - - - [ ! ] - - - - - - - -
        if (_poll.fds[poll_index].elem.revents != POLLIN)
        {
            if (DEBUG)
                std::cout << "  Error! revents = " << _poll.fds[poll_index].elem.revents << std::endl;
            close_connection(poll_index);
            continue;
        }
//  - - - - - - [ ^ ] - - - - - - - -

        if (poll_index < _listening_ports.size())
            accept_new_connections(poll_index);
        else
            handle_connection(poll_index);
    }
}

// --------------------------------------------------------------------------------------------------------

void Server::accept_new_connections(const int poll_index)
{
    int listening_fd = _poll.get_fd(poll_index);


    if (DEBUG)
        std::cout << "Listening socket " << listening_fd << " is readable." << std::endl;
    while (true)
    {
        http::Connection incoming(_tokens, _all_config[_port_server.find(listening_fd)->second]);
        incoming.establish(listening_fd);
        if (!incoming.is_good()) {
            if (errno != EWOULDBLOCK) {
                if (DEBUG) {
                    std::cerr << "accept() failed on listening port " << listening_fd << std::endl;
                    std::cerr << "Port " << ntohs(_listening_ports.find(listening_fd)->second._address.sin6_port) 
                    << " has been discarded from array" << std::endl;
                }
                close_connection(poll_index);
                _listening_ports.erase(listening_fd);
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
        if (!incoming.is_good())
            break;
    }
}

void Server::handle_connection(const int poll_index)
{
    // + timeout ?
    // + try catch ?
    _connections.find(_poll.get_fd(poll_index))->second.handle();
    if (!_connections.find(_poll.get_fd(poll_index))->second.is_persistent()) {
        close_connection(poll_index);
        if (DEBUG)
            std::cout << "--->>>> Connection closed" << std::endl;
    }
}

void Server::close_connection(const int poll_index)
{
    int fd = _poll.get_fd(poll_index);
    if (DEBUG)
        std::cout << "\n >>>>>>>>   CLOSE ! fd: " << fd << std::endl;
    _connections.erase(fd);
    close(fd);
    _poll.fds[poll_index].elem.fd = -1;
    _poll.compress_array = true;
}

} // NAMESPACE ws
