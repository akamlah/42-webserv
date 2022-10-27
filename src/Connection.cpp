/* ************************************************************************ */
/*                                                                          */
/*                              Class: Connection                           */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Connection.hpp"

namespace ws {
namespace http {

Connection::Connection(const int fd, const http::Tokens& tokens, const config_data& config)
    : _fd(fd), _status(0), _is_persistent(true), _tokens(tokens), _config(config) { }

Connection::Connection(const http::Tokens& tokens, const config_data& config)
    : _fd(-1), _status(0), _is_persistent(true), _tokens(tokens), _config(config) { }

Connection::Connection(const Connection& other)
    : _fd(other._fd), _status(other._status), _address(other._address),
        _is_persistent(other._is_persistent), _request(other._request), 
        _tokens(other._tokens), _config(other._config) { }

int Connection::fd() const { return (_fd); }
bool Connection::is_good() const { return (_fd < 0 ? false : true); }
bool Connection::is_persistent() const { return (_is_persistent);}
int Connection::status() const { return (_status); }

void Connection::establish(const int fd) {
    socklen_t address_length = sizeof(_address._address);
    _fd = ::accept(fd, (struct sockaddr *)&_address._address, &address_length);
    // #if DEBUG
    if (_fd > 0)
        std::cout << GREEN << "Successfully established connection on fd " << _fd << " from fd: " << fd << NC << std::endl;
    else
        std::cout << RED << "not valid fd: " << _fd << NC << std::endl;
    // #endif
}

void Connection::handle(const struct ws::Poll& poll, int poll_index) {
    if (!(poll.fds[poll_index].elem.revents & POLLERR)) {
    _request.parse(_fd);
    _status = _request.status();
    if (_request._waiting_for_chunks) {
        #if DEBUG
        std::cout << YELLOW << "WAITING for chnks -> return\n" << std::endl;
        #endif
        return ;
    }
        Response response(_request, _config, _tokens);
        _is_persistent = response.is_persistent();
        #if DEBUG
        std::cout << GREEN << "connection persistent ?" << _is_persistent << NC << std::endl;
        #endif
        response.send(_fd);
    }
    return ;
}

Connection::~Connection() { /* free data ?*/ }

} // NAMESPACE http
} // NAMESPACE ws
