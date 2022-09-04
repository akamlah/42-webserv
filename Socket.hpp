#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>

using std::string;

class Socket
{
private:
	int		_sock;
	long	_valread;
	struct	sockaddr_in _serv_addr;
	string	buffer;
public:
	Socket(int domain, int type, int protocol, int port) : _sock(0), _valread(0)
	{
		_sock = check_connection(domain, type, protocol);
		memset(&_serv_addr, '0', sizeof(_serv_addr));
		_serv_addr.sin_family = domain;
    	_serv_addr.sin_port = htons(port);
	}
	~Socket();

	int	check_connection(int domain, int type, int protocol)
	{
		if (_sock < 0)
			throw std::invalid_argument( "\n Socket creation error \n" );
	}
	
};
