// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TCP CONNECTION
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "../include/TCP_Connection.hpp"

namespace ws {

/*
    Class TCP_Connection: member types and attributes

    enum cn_state { ESTABLISHED = 0, CLOSE_WAIT, TIMED_OUT };
    TCP_IP6_ConnectionSocket _socket;
    const ws::config_data& _conf;
    const http::Tokens& _tokens;
    cn_state _state;
 */

// Constr/destr/cpy  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//  MAKE CPY FUNC / INIT FUNC [ + ] [ ! ]

TCP_Connection::TCP_Connection(const TCP_IP6_ListeningSocket& l_socket, const ws::config_data& conf,
    const http::Tokens& tokens) : _socket(l_socket), _conf(conf), _tokens(tokens),
    _state(ESTABLISHED),
    _brecv(0),
 /*    _request(_tokens), */
    _btosend(0), _bsent(0)
{
    memset(_buffer, 0, BUFFER_SIZE);
    WS_connection_debug("Opened Connection on fd " << _socket.fd());
}

TCP_Connection& TCP_Connection::operator=(const TCP_Connection& other) {
    // WS_connection_debug("Connection: cpy ass ope " << _socket.fd());
    _state = other._state;
    return (*this);
}

TCP_Connection::TCP_Connection(const TCP_Connection& other) : _socket(other._socket),
 _conf(other._conf), _tokens(other._tokens), _state(other._state), _request(other._request)
{
    WS_connection_debug("Connection: cpy constr " << _socket.fd());
}

TCP_Connection::~TCP_Connection() {}


// Access - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const TCP_IP6_ConnectionSocket& TCP_Connection::socket() const
    { return(_socket); }

TCP_Connection::cn_state& TCP_Connection::state()
    { return(_state); }


// Core - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TCP_Connection::response_to_str() { // replace with class response later.

    http::Response response(_request, _conf, _tokens);
    _response_str = response.string();
    _btosend = _response_str.length() + 1;
    if (!response.status_is_success())
        _state = HTTP_ERROR;
}

// handle buffer size: read again if bytes read > socket buffer etc...
void TCP_Connection::rdwr() {

    WS_connection_debug("Reading on fd: " << _socket.fd());

    _brecv = 0;
    memset(&_buffer, 0, BUFFER_SIZE);
    _request = http::Request();

    int n = ::recv(_socket.fd(), _buffer, BUFFER_SIZE, 0);
    WS_connection_debug("BYTES read: " << n);
    if (n < 0) {
        
        #ifdef DEBUG_CONNECTIONS
        if (errno == ECONNRESET) { WS_connection_debug("Connection reset"); }
        else if (errno == EPIPE) { WS_connection_debug("Pipe error - RD"); }
        WS_connection_debug("READ ERROR" << strerror(errno));
        #endif
        
        _state = RD_ERROR;
    }
    else if (n == 0) {
        WS_connection_debug("Conn closed by client");
        _state = CLOSE_WAIT;
    }
    else {
        std::cout << "|" << _buffer << "|" << std::endl;
        _brecv += n;

        // _request.parse_header(_buffer, _brecv);
        _request.parse(_buffer, _brecv);

        _state = ESTABLISHED; // redundant, for safety


        // PREPARE WRITE :

        // Maybe do a check for writing func, set to pollout and check that before actually writing
        _socket.configure(); // [ ! ] HERE OR BEFORE RECV ??
        response_to_str();
        write();
    }
}

void TCP_Connection::write() {

    WS_connection_debug("Responding on fd: " << _socket.fd());
    // int n = ::send(_socket.fd(), &(*(_s.str().begin() + _bsent)), _btosend - _bsent, 0);
    int n = ::send(_socket.fd(), _response_str.c_str() + _bsent, _btosend - _bsent, 0);
    if (n < 0) {
        std::cout << "bytes sent now " << n << std::endl;
        
        #ifdef DEBUG_CONNECTIONS
        if (errno == EPIPE) { WS_connection_debug("Pipe error - RD"); }
        WS_connection_debug("READ ERROR" << strerror(errno));
        #endif
        
        _state = WR_ERROR;
        return ;
    }
    else {
        // [ ! ] make case for 0 to not get stuck in infinite loop 0 ? or maybe poll is enough
        _bsent += n;
        std::cout << "bytes sent now " << n << " for a total of " << _bsent << " of " << _btosend << std::endl;
        if (_bsent == _btosend) {
            std::cout << "DONE" << std::endl;
            // std::cout << "|" << _response_str << "|" << std::endl;
            _bsent = 0;
            _btosend = 0;
            _response_str = std::string();
            if (_state != HTTP_ERROR)
                _state = ESTABLISHED;
            return ;
        }
        _state = PARTIAL_RESPONSE;
        std::cout << "NEXT ROUND" << std::endl;
        return ;
    }
}


} // namespace ws
