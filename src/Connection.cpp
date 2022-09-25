/* ************************************************************************ */
/*                                                                          */
/*                              Class: Connection                           */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Connection.hpp"

namespace ws {
namespace http {

// const char* Connection::what() const throw() {
//     return ("");
// }

Connection::Connection(const int fd, const http::Tokens& tokens): _fd(fd), _status(0), _is_persistent(true), _tokens(tokens) { }

Connection::Connection(const http::Tokens& tokens): _fd(-1), _status(0), _is_persistent(true), _tokens(tokens) { }

Connection::Connection(const Connection& other)
    : _fd(other._fd), _status(other._status), _address(other._address),
        _is_persistent(other._is_persistent), _request(other._request), _tokens(other._tokens)
{
    if (DEBUG)
        std::cout << "Connection cpy constr" << std::endl;
}

Connection& Connection::operator=(const Connection& other) {
    if (DEBUG)
        std::cout << "Connection assign ope =" << std::endl;
    _fd = other._fd;
    _status = other._status;
    _address = other._address;
    _is_persistent = other._is_persistent;
    _request = other._request;
    return (*this);
}

int Connection::fd() const { return (_fd); }
bool Connection::is_good() const { return (_fd < 0 ? false : true); }
bool Connection::is_persistent() const { return (_is_persistent);}
int Connection::status() const { return (_status); }

void Connection::establish(const int fd) {
    socklen_t address_length = sizeof(_address);
    _fd = ::accept(fd, (struct sockaddr *)&_address, &address_length);
    if (DEBUG && _fd > 0)
        std::cout << "<<<<<--------- established connection on fd " << _fd << " from fd: " << fd << std::endl;
}

void Connection::handle() {
    // start timer if first call
    // else check timer
    try {
        try {
            _request.parse(_fd);
            _status = _request.status();
            // system("leaks webserv | tail - 3");
        }
        catch (http::Request::EofReached& e) { // <- very hacky, might become a problem, we'll see
            std::cout << "EOF" << std::endl;
            _is_persistent = false;
        }
        Response response(_request, _tokens);
        _is_persistent = _request.is_persistent();
        response.send(_fd); //argument config file
    }
    catch (ws::exception& e) {
        std::cout << RED << "unforeseen exception req-resp" << NC << std::endl;
    }
}

Connection::~Connection() { /* free data ?*/ }

} // NAMESPACE http
} // NAMESPACE ws
