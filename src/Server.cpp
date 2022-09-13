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

// [ ! ] server can listen to multiple ports on nginx -> have different sockets?
// probably best to have just one server constructor with CONFIG
// and a default constructor with a defalut config
// config gets paresd by Master Class ("engine" or "configuration" or something like that, that then passes an object or a string to 
// each server instance)
Server::Server(Socket& server_socket, config_data configData): _socket(server_socket), _port(configData.port) {
    int temp = 1;

    if (setsockopt(_socket.fd, SOL_SOCKET,  SO_REUSEADDR, (char *)&temp, sizeof(temp)) < 0)
        throw_print_error(SystemError(), "setsockopt() failed");
    if (ioctl(_socket.fd, FIONBIO, (char *)&temp) < 0)
        throw_print_error(SystemError(), "ioctl() failed");
    memset(&_address, 0, sizeof(_address));
    _address.sin6_family = AF_INET6; // as option ?
    memcpy(&_address.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    _address.sin6_port = htons(_port);

}
Server::Server(Socket& server_socket, int port): _socket(server_socket), _port(port) {
    int temp = 1;

    if (setsockopt(_socket.fd, SOL_SOCKET,  SO_REUSEADDR, (char *)&temp, sizeof(temp)) < 0)
        throw_print_error(SystemError(), "setsockopt() failed");
    if (ioctl(_socket.fd, FIONBIO, (char *)&temp) < 0)
        throw_print_error(SystemError(), "ioctl() failed");
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
    try{
        Request new_request(new_connection); // paring inside
        // 2 send response
        respond(new_connection, new_request);
    }
    // temporary
    catch (ws::exception& e) {
        // only catch specific exceptions
        // std::cout << "BAD REQUEST" << std::endl; /* bad request error */
        throw;
    }

// CODE TO TEST CONNECTION INDIPENDENTLY OF REQUEST/RESPONSE CLASSES --------------------------------
        char buffer[1024];
        bzero(buffer,256);
        size_t bytes_read;
        FILE *html_data = fopen("./example_sites/example1/index.html", "r");
        // FILE *html_data = fopen("./example_sites/someJoke/index.html", "r");
        if (!html_data)
            throw_print_error(SystemError());
        char response_data[1024];
        if (fgets(response_data, 1024, html_data) == NULL)
            throw_print_error(SystemError());
        char http_header[2048] = "HTTP/1.1 200 OK\r\n\n";
        strcat(http_header, response_data);
        if ((bytes_read = read(new_connection.fd, buffer, 1023)) < 0)
            throw_print_error(SystemError());
        std::cout << CYAN << "Message recieved: ---------\n\n" << NC << buffer;
        std::cout << CYAN << "---------------------------\n" << NC << std::endl;

                // html response test ---------------------------
                        std::ifstream confFile;
                        
                            confFile.open("./example_sites/someJoke/index.html", std::ios::in);
                        // if (trythis)
                        // {
                        //     confFile.open("./example_sites/someJoke/index.html", std::ios::in);
                        //     trythis = false;
                        // }
                        // else
                        //     confFile.open("./example_sites/someJoke/server.js", std::ios::in);
                        if (confFile.fail())
                            throw_print_error(SystemError());
                        std::stringstream buffer2;
                        buffer2 << confFile.rdbuf();
                        std::string temp = "HTTP/1.1 200 OK\r\n\n" + buffer2.str();
        int sending_status = send(new_connection.fd, temp.c_str(), temp.size(), 0);
                // end resos test -------------------------

        // how to stop the browser to be in constant loading phase?

        // int sending_status = send(new_connection.fd, http_header, sizeof(http_header), 0);
        if (sending_status < 0)
            throw_print_error(SystemError());
        std::cout << CYAN << "Server sent data" << NC << std::endl;
// --------------------------------------------------------------------------------------------------
}

void Server::respond(Socket& new_connection, Request request) const {
    // generate response according to request & send it:
    try {
        Response response(request);
        // send(response)
        if (::send(new_connection.fd, response.c_str(), sizeof(response.c_str()), 0) < 0) {
            throw_print_error(SystemError(), "Failed to send response");
        }
        std::cout << CYAN << "Server sent response" << NC << std::endl;
    }
    catch(std::exception& e) { throw_print_error(SystemError()); }
}

void    Server::run(int timeout)
{
    int incoming_fd;
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
                    try { handle_connection(new_conn); }
                    catch (ws::Request::BadRead& e)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            if (DEBUG)
                                std::cerr << "  recv() failed" << std::endl;
                            close_conn = true;
                        }
                        break;
                    }
                    catch (ws::Request::EofReached& e)
                    {
                        if (DEBUG)
                            std::cout << "  Connection closed" << std::endl;
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
const Socket& Server::socket() const { return (_socket); }
int Server::port() const { return (_port); }

} // NAMESPACE ws

