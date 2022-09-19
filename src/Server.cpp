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
}

Server::~Server() {}

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

int Server::accept(int fd) const {
    int new_conn_fd;
    struct sockaddr_in6 client_address;
    socklen_t client_length = sizeof(client_address);
    new_conn_fd = ::accept(fd, (struct sockaddr *)&client_address, &client_length);
    return new_conn_fd;
}

bool Server::handle_connection(Socket& new_connection) const {
    // try{
        http::Request request(new_connection);
        request.parse();
        http::Response response(request);
        if (response.keep_alive)
            return (true);
    // }

    // #if DEBUG
    std::cout << "Going on" << std::endl;
    // #endif
    return (false);
}

void    Server::run(int timeout)
{
    int number_of_listening_ports = _listening_sockets.size();

    _poll.set_timeout(timeout);
    for(std::map<Socket, s_address>::const_iterator iter = _listening_sockets.cbegin(); iter != _listening_sockets.cend(); ++iter)
        _poll.add_to_poll(iter->first.fd, POLLIN, 0);
    do{
        if (DEBUG) 
            std::cout << "Waiting on poll()..." << std::endl;
        _poll.poll();
        handle_events(number_of_listening_ports);
        _poll.compress();

    } while (number_of_listening_ports);
    _poll.close_all();
} 

void Server::handle_events(int& number_of_listening_ports)
{
    int    current_size = _poll.fds.size();

    for (int i = 0; i < current_size; ++i)
    {
        if (_poll.fds[i].elem.revents == 0)
            continue;
        if (_poll.fds[i].elem.revents != POLLIN)
        {
            if (DEBUG) 
                std::cout << "  Error! revents = " << _poll.fds[i].elem.revents << std::endl;
            close_connection(i);
            continue;
        }
        if (i < number_of_listening_ports)
            accept_new_connections(i, number_of_listening_ports);
        else
            handle_incoming(i);
    }
}

void Server::accept_new_connections(int& index, int& number_of_listening_ports)
{
    int    incoming_fd;

    if (DEBUG)
        std::cout << "Listening socket " << _poll.fds[index].elem.fd << "is readable." << std::endl;
    do
    {
        incoming_fd = accept(_poll.fds[index].elem.fd);
        if (incoming_fd < 0)
        {
            if (errno != EWOULDBLOCK)
            {
                if (DEBUG){
                    std::cerr << "accept() failed on listening port " << _poll.fds[index].elem.fd << std::endl;
                    std::cerr << "Port has been discarded from array" << std::endl;
                }
                close_connection(index);
                --number_of_listening_ports;
            }
            break;
        }
        _poll.add_to_poll(incoming_fd, POLLIN);
    } while (incoming_fd != -1);
}

void Server::handle_incoming(int& index)
{ 
    Socket  new_conn;
    bool keep_alive = true;

    if (DEBUG)
    std::cout << "Descriptor " << _poll.fds[index].elem.fd << " is readable" << std::endl;
    new_conn = Socket(_poll.fds[index].elem.fd);

	// struct timespec timeStart, timeEnd;
	// clock_gettime(CLOCK_MONOTONIC, &timeStart);
    // handle_connection(new_conn);

    while(keep_alive == true) {

        try {
            keep_alive = handle_connection(new_conn);
        }
        catch (http::Request::EofReached& e) {
            std::cout << "EOF" << std::endl;
            // close_connection(index);
            break ;
        }
        catch (ws::exception& e) {
            std::cout << RED << "unforeseen exception req-resp" << NC << std::endl;
            // close_connection(index);
            break ;
        }

	    // clock_gettime(CLOCK_MONOTONIC, &timeEnd);
        // double timePerformance = timeStart.tv_sec * 1e9;
        // timePerformance = (timeEnd.tv_sec - timeStart.tv_sec) * 1e9;
        // timePerformance = (timePerformance + (timeEnd.tv_nsec - timeStart.tv_nsec)) * 1e-9;
        // std::cout << timePerformance << std::endl;
        // if (timePerformance >= 60)
            // break ;
    }
    close_connection(index);
    std::cout << "  Connection closed" << std::endl;
}

void Server::close_connection(int index)
{
    close(_poll.fds[index].elem.fd);
    _poll.fds[index].elem.fd = -1;
    _poll.compress_array = true;
}

// Do we need these accessors?
// const Socket& Server::socket() const { return (_socket); }
// int Server::port() const { return (_port); }

} // NAMESPACE ws

