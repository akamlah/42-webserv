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
    _socket.configure();
    _timer = std::clock();
}

TCP_Connection& TCP_Connection::operator=(const TCP_Connection& other) {
    // WS_connection_debug("Connection: cpy ass ope " << _socket.fd());
    _timer = std::clock();
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

bool TCP_Connection::is_timedout() {
    std::clock_t t_now = std::clock();
    int elapsed_sec = ((double)(t_now - _timer) / CLOCKS_PER_SEC) * 1000;
    std::cout << YELLOW << " - " << elapsed_sec << " - " << NC << std::endl;
    if (elapsed_sec > 60) {
        _state = TIMED_OUT;
        return (true);
    }
    return (false);
}


void TCP_Connection::prepare_read_buffer() {
    _brecv = 0;
    memset(&_buffer, 0, BUFFER_SIZE);
    _request = http::Request(); // request.reset() method ?
}

void TCP_Connection::prepare_response() {
    _socket.configure();
    http::Response response(_request, _conf, _tokens);
    _response_str = response.string();
    _btosend = _response_str.length();
    if (!response.status_is_success())
        _state = HTTP_ERROR;
    // else if (response.()) HAS "CLOSE"
        // _state = CLOSE_WAIT;
}

// handle buffer size: read again if bytes read > socket buffer etc...
void TCP_Connection::read() {
    WS_connection_debug("Reading on fd: " << _socket.fd());
    int n = ::recv(_socket.fd(), _buffer, BUFFER_SIZE, 0);
    _timer = std::clock();
    WS_connection_debug("BYTES read: " << n);
    if (n < 0) {
        WS_connection_debug("Error reading: " << strerror(errno));
        _state = RD_ERROR;
    }
    else if (n == 0) {
        WS_connection_debug("Conn closed by client");
        _state = CLOSE_WAIT;
    }
    else {
        std::cout << "|" << _buffer << "|" << std::endl;
        _brecv += n;
        // if (_brecv == content length)
        _state = ESTABLISHED; // redundant
        // return
        _timer = std::clock();
    }
}

void TCP_Connection::rdwr() {
    prepare_read_buffer();
    read();
    if (_state == ESTABLISHED) {
        _request.parse(_buffer, _brecv);
        prepare_response();
        write();
    }
}

void TCP_Connection::write() {
    WS_connection_debug("Responding on fd: " << _socket.fd());
    int n = ::send(_socket.fd(), _response_str.c_str() + _bsent, _btosend - _bsent, 0);
    _timer = std::clock();
    if (n <= 0) {
        WS_connection_debug("Bytes sent now " << n);
        WS_connection_debug("Error writing: " << strerror(errno));
        _state = WR_ERROR;
        return ;
    }
    else {
        // [ ! ] make case for 0 to not get stuck in infinite loop 0 ? or maybe poll is enough
        _bsent += n;
        WS_connection_debug("bytes sent now " << n << " for a total of " << _bsent << " of " << _btosend);
        if (_bsent == _btosend) {
            WS_connection_debug("DONE");
            // std::cout << "|" << _response_str << "|" << std::endl;
            _bsent = 0;
            _btosend = 0;
            _response_str = std::string();
            if (_state != HTTP_ERROR)
                _state = ESTABLISHED;
            return ;
        }
        _state = PARTIAL_RESP;
        WS_connection_debug("NEXT ROUND");
        return ;
    }
}

std::string TCP_Connection::state_to_str() const {
    switch (_state) {
        case TCP_Connection::ESTABLISHED: return("ESTABLISHED");
        case TCP_Connection::PARTIAL_RESP: return("PARTIAL_RESP");
        case TCP_Connection::CLOSE_WAIT: return("CLOSE WAIT");
        case TCP_Connection::TIMED_OUT: return("TIMEOUT");
        case TCP_Connection::HTTP_ERROR: return("HTTP_ERROR");
        case TCP_Connection::RD_ERROR: return("RD_ERROR");
        case TCP_Connection::WR_ERROR: return("WR_ERROR");
        default: return (std::string());
    }
}

} // namespace ws
