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

Connection::Connection(const int fd): _fd(fd), _status(0), _is_persistent(true) { }

Connection::Connection(): _is_persistent(true) { }

Connection::Connection(const Connection& other) {
    _fd = other._fd;
    _poll_id = other._poll_id;
    _address = other._address;
    _fd = other._fd;
    _is_persistent = other._is_persistent;
}

Connection& Connection::operator=(const Connection& other) {
    _fd = other._fd;
    _poll_id = other._poll_id;
    _address = other._address;
    _fd = other._fd;
    _is_persistent = other._is_persistent;
    return (*this);
}

int Connection::fd() const { return (_fd); }
bool Connection::good() const { return (_fd < 0 ? false : true); }
bool Connection::is_persistent() const { return (_is_persistent);}
int Connection::status() const { return (_status); }

int Connection::get_id() const { return (_poll_id); }
void Connection::set_id(const int id) { _poll_id = id; }

void Connection::establish(const int fd) {
    socklen_t address_length = sizeof(_address);
    _fd = ::accept(fd, (struct sockaddr *)&_address, &address_length);
}

void Connection::handle() {
    try {
        http::Request request;
        try {
            request.parse(_fd);
            _status = request.status();
            _is_persistent = request.is_persistent();
        }
        catch (http::Request::EofReached& e) { // <- very hacky, might become a problem, we'll see
            std::cout << "EOF" << std::endl;
        }
        http::Response response(request, _fd);
        // response.send(_fd);
    }
    catch (ws::exception& e) {
        std::cout << RED << "unforeseen exception req-resp" << NC << std::endl;
    }
}

Connection::~Connection() { /* free data ?*/ }

} // NAMESPACE http
} // NAMESPACE ws
