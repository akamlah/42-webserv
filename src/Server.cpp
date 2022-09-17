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
        // size_t bytes_read;
        FILE *html_data = fopen("./example_sites/example1/index.html", "r");
        // FILE *html_data = fopen("./example_sites/someJoke/index.html", "r");
        if (!html_data)
            throw_print_error(SystemError());
        char response_data[1024];
        if (fgets(response_data, 1024, html_data) == NULL)
            throw_print_error(SystemError());
        char http_header[2048] = "HTTP/1.1 200 OK\r\n\n";
        strcat(http_header, response_data);
        // if ((bytes_read = read(new_connection.fd, buffer, 1023)) < 0)
        //     throw_print_error(SystemError());
        std::cout << CYAN << "Message recieved: ---------\n\n" << NC << buffer;
        std::cout << CYAN << "---------------------------\n" << NC << std::endl;

                // html response test ---------------------------
                        std::ifstream confFile;
                        if (confFile.fail())
                            throw_print_error(SystemError());
                        int sending_status;
                            // confFile.open("./example_sites/someJoke/index.html", std::ios::in);
                            // confFile.open("./example_sites/phptestsite/index.html", std::ios::in); // request came for php file
                        if (trythis)
                        {
                            confFile.open("./example_sites/phptestsite/index.html", std::ios::in); // request came for php file
                            // confFile.open("./example_sites/someJoke/index.html", std::ios::in);
                            std::stringstream buffer2;
                            buffer2 << confFile.rdbuf();
                            std::string temp = "HTTP/1.1 200 OK\r\n\n" + buffer2.str();
                            sending_status = send(new_connection.fd, temp.c_str(), temp.size(), 0);
                            trythis = false;
                        }
                        else
                        {
                            Cgi test;
                            std::string phpresp;
                            phpresp = "HTTP/1.1 200 OK \nCONTENT_TYPE = text/event-stream\r\n\n";
                            phpresp +=  test.executeCgi("./example_sites/phptestsite/send_sse.php");
                            sending_status = send(new_connection.fd, phpresp.c_str(), phpresp.size(), 0);
                        }
                        
                        //     confFile.open("./example_sites/someJoke/server.js", std::ios::in);
                        //     confFile.open("./example_sites/someJoke/server.js", std::ios::in);
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
    bool    close_conn = false;

    if (DEBUG)
    std::cout << "Descriptor " << _poll.fds[index].elem.fd << " is readable" << std::endl;
    do
    {
        new_conn = Socket(_poll.fds[index].elem.fd);
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
                std::cerr << "  Connection closed" << std::endl;
            close_conn = true;
            break;
        }
    } while (true);
    if (close_conn)
        close_connection(index);
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

