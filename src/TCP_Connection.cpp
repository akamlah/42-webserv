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

TCP_Connection::TCP_Connection(const TCP_IP6_ListeningSocket& l_socket, const ws::config_data& conf,
	const http::Tokens& tokens) : _socket(l_socket), _conf(conf), _tokens(tokens),
	_state(ESTABLISHED),
	_brecv(0),
	_btosend(0), _bsent(0)
{
	memset(_buffer, 0, BUFFER_SIZE);
	WS_connection_debug("Opened Connection on fd " << _socket.fd());
	_socket.configure();
	refresh_timer();
	_request.reset();
}

TCP_Connection& TCP_Connection::operator=(const TCP_Connection& other) {
	refresh_timer();
	_state = other._state;
	_request.reset();
	return (*this);
}

TCP_Connection::TCP_Connection(const TCP_Connection& other) : _socket(other._socket),
_conf(other._conf), _tokens(other._tokens), _state(other._state)
{
	WS_connection_debug("Connection: cpy constr " << _socket.fd());
	_request.reset();
	refresh_timer();
}

TCP_Connection::~TCP_Connection() {}


// Access - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const TCP_IP6_ConnectionSocket& TCP_Connection::socket() const
	{ return(_socket); }

TCP_Connection::cn_state& TCP_Connection::state()
	{ return(_state); }


// Core - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TCP_Connection::refresh_timer() {
	struct timeval	tvx;
	gettimeofday(&tvx, NULL);
	_time_last = ((tvx.tv_usec / 1000) + (tvx.tv_sec * 1000));
}

bool TCP_Connection::is_timedout() {
	struct timeval	tvx;
	gettimeofday(&tvx, NULL);
	long t_now = ((tvx.tv_usec / 1000) + (tvx.tv_sec * 1000));
	long diff = t_now - _time_last;
	if (diff > 60000) { // 60 sec
		_state = TIMED_OUT;
		return (true);
	}
	return (false);
}

void TCP_Connection::prepare_read_buffer() {
	_brecv = 0;
	memset(&_buffer, 0, BUFFER_SIZE);
}

static void remove_trailing_slash(std::string& path) {
	if (!path.empty())
		if (path[path.length() - 1] == '/')
			path.erase(path.length() - 1, path.length());
}


void TCP_Connection::prepare_response() {
	_bsent = 0;
	_socket.configure();
	config_data tempconf;
	if (_conf.location != "[") {
		std::vector<ws::config_route>::const_iterator it = _conf.routs.begin();
		std::vector<ws::config_route>::const_iterator iendt = _conf.routs.end();
		std::string::size_type secondloc = _request.header.target.find("/", 1);
		if (secondloc == std::string::npos)
			secondloc = _request.header.target.length() - 1;
		std::string tempercPath = _request.header.target.substr(1, secondloc);
		remove_trailing_slash(tempercPath);
		while (tempercPath != (*it).folder && it != iendt) 
			it++;
		if (it == iendt) {
			std::__1::list<std::__1::string> fieldtemp = _request.get_field_value("referer");
			std::__1::list<std::__1::string> fieldhost = _request.get_field_value("host");
			std::string temphost = *(fieldhost.begin());
			std::string tempRef = *(fieldtemp.begin());
			std::string::size_type whereTheFront = tempRef.find(temphost);

			if (whereTheFront != std::string::npos)
			{
				it = _conf.routs.begin();
				std::string::size_type startcutsign = whereTheFront + temphost.length() + 1;
				std::string temRefutEnd;
				if (startcutsign < tempRef.length())
					temRefutEnd = tempRef.substr(startcutsign, tempRef.length() - startcutsign);
				remove_trailing_slash(temRefutEnd);
				while (it != iendt && temRefutEnd != (*it).folder)
					it++;
			}
		} 

		if (it != iendt) {
			tempconf.host = _conf.host;
			tempconf.ports = _conf.ports;
			tempconf.error = _conf.error;
			tempconf.limit_body = _conf.limit_body;
			tempconf.server_name = _conf.server_name;
			tempconf.routs = _conf.routs;

			tempconf.cgi = (*it).cgi;
			tempconf.root = (*it).root;
			tempconf.http_methods = (*it).http_methods;
			tempconf.http_redirects = (*it).http_redirects;
			tempconf.index = (*it).index ;
			tempconf.directory_listing = (*it).directory_listing ;
			tempconf.download = (*it).download ;
			tempconf.isCgiOn = (*it).isCgiOn ;

			try {
				http::Response response(_request, tempconf, _tokens);
				_response_str = response.string();
				_request.reset();
				_btosend = _response_str.length();
				if (!response.status_is_success())
					_state = HTTP_ERROR;
			}
			catch (std::exception& e) {
				_state = RD_ERROR;
			}
		}
		else
		{
			try {
				http::Response response(_request, tempconf, _tokens);
				_response_str = response.string();
				_request.reset();
				_btosend = _response_str.length();
				if (!response.status_is_success())
					_state = HTTP_ERROR;
			}
			catch (std::exception& e) {
				_state = RD_ERROR;
			}

		}
	}
	else
	{
			try {
				http::Response response(_request, tempconf, _tokens);
				_response_str = response.string();
				_request.reset();
				_btosend = _response_str.length();
				if (!response.status_is_success())
					_state = HTTP_ERROR;
			}
			catch (std::exception& e) {
				_state = RD_ERROR;
			}
	}
}

void TCP_Connection::read() {
	WS_connection_debug("Reading on fd: " << _socket.fd());
	int n = ::recv(_socket.fd(), _buffer, BUFFER_SIZE, 0);
	refresh_timer();
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
		_brecv += n;
		_state = ESTABLISHED;
		refresh_timer();
	}
}

void TCP_Connection::rdwr() {
	prepare_read_buffer();
	read();
	if (_state == ESTABLISHED) {
		_request.reset();
		_request.parse(_buffer, _brecv);
		prepare_response();
		if (_state != RD_ERROR)
			write();
	}
}

void TCP_Connection::write() {
	WS_connection_debug("Responding on fd: " << _socket.fd());
	int n = ::send(_socket.fd(), _response_str.c_str() + _bsent, _btosend - _bsent, 0);
	refresh_timer();
	if (n <= 0) {
		WS_connection_debug("Bytes sent now " << n);
		WS_connection_debug("Error writing: " << strerror(errno));
		_state = WR_ERROR;
		return ;
	}
	else {
		_bsent += n;
		WS_connection_debug("bytes sent now " << n << " for a total of " << _bsent << " of " << _btosend);
		if (_bsent == _btosend) {
			WS_connection_debug("DONE");
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
