// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// CLASS SERVER - implementation: CORE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

/*      CONTENT:

        Class Server:
            * nested class TCP_ConnectionMap - implementation.
            * member functions in Server implemented:
                Server(const std::vector<ws::config_data>& config_server_blocks);
                ~Server();
                void run();
                void configure_listening_sockets();
                void listen(const int backlog) const;
                void handle_events(int& events);
                void accept_on_listening_socket(int id);
                void handle_connection(int id);
                void close_connection(int id);
                void log_pool_id_events(int id);
 */


#include "../include/Server.hpp"

namespace ws {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Server - nested class TCP_ConnectionMap, the container for the tcp connections.
// Manages accept calls and connection socket initialization
// (in TCP_Connection::TCP_IP6_ConectionSocket)
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// construction/destruction/copy

Server::TCP_ConnectionMap::TCP_ConnectionMap(const Server& server)
    : _server(server) { }

Server::TCP_ConnectionMap::~TCP_ConnectionMap() {}


// accessors

TCP_Connection& Server::TCP_ConnectionMap::operator[](int fd)
    { return(_map.find(fd)->second); }

const TCP_Connection& Server::TCP_ConnectionMap::operator[](int fd) const
    { return(_map.find(fd)->second); }


// core member functions

int Server::TCP_ConnectionMap::add_new(const TCP_IP6_ListeningSocket& l_socket) {
    TCP_Connection new_connection(
        l_socket, 
        _server._config_server_blocks[l_socket.server_id()],
        _server._tokens
    );
    if (new_connection.socket().fd() < 0)
        return (-1);
    _map.insert(std::make_pair(new_connection.socket().fd(), new_connection));
    // WS_events_debug("Established new connection with fd "
    //     << new_connection.socket().fd());
    return (new_connection.socket().fd());
}

void Server::TCP_ConnectionMap::remove(int fd) {
    _map.erase(_map.find(fd));
}

void Server::TCP_ConnectionMap::print() {
    // WS_events_debug("CONN:");
    // for (std::map<int, TCP_Connection>::const_iterator it = _map.begin(); it != _map.end(); it++) {
    //     WS_events_debug("fd: " << it->first << " sock_fd " << it->second.socket().fd());
    // }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Server - core
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// Constr/destr - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Server::Server(const std::vector<ws::config_data>& config_server_blocks)
    : _config_server_blocks(config_server_blocks), _connections(*this) { 
    configure_listening_sockets();
}

Server::~Server() {
    for (size_t id = 0; id < _fd_pool.size(); id++) {
            ::close(_fd_pool[id].fd);
    }
}

// Core - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Server::run() {
    #ifdef TCP_LOG_MAC_OS
    system("echo \"\n\" > stats");
    int i = 0;
    #endif
    signal(SIGINT, &Server::sigint);
    signal(SIGQUIT, &Server::sigquit);
    signal(SIGTERM, &Server::sigterm);
    signal(SIGPIPE, &Server::sigpipe);
    this->listen(BACKLOG);
    int events;
    for (;;) {
        // WS_events_debug("- - - - POLL - - - - ");
        events = _fd_pool.poll(TIMEOUT_MS);
        if (events < 0)
            throw_print_error(SystemError(), "poll() failed");
        #ifdef TCP_LOG_MAC_OS
        system("lsof -i -P -n | grep webserv >> stats; echo \"\n\" >> stats");
        if (++i > TCP_LOG_MAC_OS_MAX_ENTRIES)
            system("echo \"\n\" > stats");
        #endif
        // for (size_t id = _listening_sockets.size(); id < _fd_pool.size(); id++) {
        //     /* if ( */_connections[_fd_pool[id].fd].is_timedout();/* ) */
        //         // close_connection(id);
        // }
        
        if (!events)
            continue ;
        handle_events_incoming(events);
        handle_events_connections(events);
        _fd_pool.compress();
        WS_events_debug(std::endl);
        // if (events)
        //     throw_print_error(SystemError(), "Missed events - fatal"); // other error
        WS_events_debug("- - - - - - - - - - - ");
    }
}

void Server::configure_listening_sockets() {
    for (size_t s_id = 0; s_id < _config_server_blocks.size(); ++s_id) {
        for(size_t port_id = 0; port_id < _config_server_blocks[s_id].ports.size(); ++port_id) {
            try {
                TCP_IP6_ListeningSocket new_socket(
                    s_id,
                    _config_server_blocks[s_id].ports[port_id]
                );
                if (new_socket.configure() < 0)
                    std::cerr << "Failed to configure port " << new_socket.port()
                            << " - ignoring port." << std::endl;
                else {
                    _listening_sockets.push_back(new_socket);
                    _fd_pool.add_descriptor(new_socket.fd(), POLLRDNORM);
                }
            } catch (std::exception& e) {
                throw_print_error(SystemError(), "Fatal error - Failed to configure server");
            }
        }
    }
}

void Server::listen(const int backlog) const {
    // put these to socket class
    for (std::vector<TCP_IP6_ListeningSocket>::const_iterator it = _listening_sockets.begin();
        it != _listening_sockets.end(); it++) {
        if (::bind(it->fd(), (struct sockaddr *)(&(it->address())), sizeof(it->address())) < 0)
            throw_print_error(SystemError(), "Failed to bind socket");
        if (::listen(it->fd(), backlog) < 0)
            throw_print_error(SystemError(), "Server unable to listen for connections");
        // WS_events_debug(CYAN << "Server listening on port " << it->port() << NC);
    }
    #ifdef EVENTS_DEBUG
    for (std::vector<TCP_IP6_ListeningSocket>::const_iterator it = _listening_sockets.begin();
        it != _listening_sockets.end(); it++) {
        WS_events_debug("fd: " << it->fd() << " "
            << "port: " << it->port() << " "
            << "server_id: " << it->server_id() << " ");
    }
    #endif
}

void Server::handle_events_incoming(int& events) {
    for (size_t id = 0; id < _listening_sockets.size(); id++) {
        if (_fd_pool[id].revents != 0)
            { log_pool_id_events(id);
            //  WS_events_debug("    [ Ls ] ");
              }
        if (_fd_pool[id].revents & POLLRDNORM) {
            if (_fd_pool.size() < MAX_POLLFD_NB) {
                accept_on_listening_socket(id);
                _connections.print();
                if (--events <= 0)
                    break ;
            }
        }
    }
}

void Server::accept_on_listening_socket(int id) {
    // WS_events_debug("Accepting new connections on fd " << _fd_pool[id].fd);
    int fd = -1;
    while((fd = _connections.add_new(_listening_sockets[id])) > 0
     && errno != ECONNABORTED) { // Stevens, c 5.11
        _fd_pool.add_descriptor(fd, POLLRDNORM);
        _connections.print();
    }
    // WS_events_debug("Exiting accept loop");
}

void Server::handle_events_connections(int& events) {
    size_t current_size = _fd_pool.size();
    for (size_t id = _listening_sockets.size(); id < current_size; id++) {
        if (_fd_pool[id].revents != 0)
            { log_pool_id_events(id); WS_events_debug("    [ Cn ]"); }
        // if (_connections[_fd_pool[id].fd].is_timedout())
        //     handle_connection(id);
        if (_fd_pool[id].revents & (POLLRDNORM | POLLERR) || _fd_pool[id].revents & (POLLWRNORM | POLLERR)) { // == RST
            handle_connection(id);
            --events;
        }
    }
}

void Server::handle_connection(int id) {
    int cid = _fd_pool[id].fd; // connections are mapped by fd
    WS_events_debug(CYAN << _connections[cid].state_to_str() << NC);
    switch (_connections[cid].state()) {

        case TCP_Connection::ESTABLISHED:
            _connections[cid].rdwr();
            if (_connections[cid].state() == TCP_Connection::PARTIAL_RESP)
                _fd_pool[id].events = POLLWRNORM;
            break ;

        case TCP_Connection::PARTIAL_RESP:
            _connections[cid].write();
            if (_connections[cid].state() == TCP_Connection::ESTABLISHED)
                _fd_pool[id].events = POLLRDNORM;
            break ;

        case TCP_Connection::CLOSE_WAIT:
        case TCP_Connection::TIMED_OUT:
        case TCP_Connection::HTTP_ERROR:
            // half_close_connection(id); // siege problem! ("cn. reset by peer" error)
            close_connection(id);
            break;

        case TCP_Connection::RD_ERROR:
        case TCP_Connection::WR_ERROR:
            close_connection(id);
            break ;

        default:
            break;
    }
}

void Server::close_connection(int id) {
    // WS_events_debug(CYAN << "Closing connection on fd " << _fd_pool[id].fd << NC);
    _connections.remove(_fd_pool[id].fd);
    ::close(_fd_pool[id].fd);
    _fd_pool.mark_to_cancel(id);
}

// gracefully close -> leave client time to read remaining data on socket
void Server::half_close_connection(int id) {
    // WS_events_debug(CYAN << "Half closing connection on fd " << _fd_pool[id].fd << NC);
    _connections.remove(_fd_pool[id].fd);
    ::shutdown(_fd_pool[id].fd, SHUT_RDWR);
    _fd_pool.mark_to_cancel(id);
}

void Server::log_pool_id_events(int id) {
    // WS_events_debug_n("id: " << id << " "
    //     << "events: " << _fd_pool[id].events << " "
    //     << "revents: " << _fd_pool[id].revents << " "
    //     << "fd: " << _fd_pool[id].fd << " "
    //     << "descriptor: " << &_fd_pool[id] << " ");
    (void)id;
}

} // namespace ws
