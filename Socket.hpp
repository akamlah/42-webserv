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
	char	*hello = "Hello from client";
	char	_buffer[1024] = {0};
public:
	Socket(int domain, int type, int protocol, int port) : _sock(0), _valread(0)
	{
		check_connection(domain, type, protocol);
		memset(&_serv_addr, '0', sizeof(_serv_addr));
		_serv_addr.sin_family = domain;
    	_serv_addr.sin_port = htons(port);

		if(inet_pton(AF_INET, "127.0.0.1", &_serv_addr.sin_addr) <= 0)
			throw std::invalid_argument( "\nInvalid address/ Address not supported \n" );
		if (connect(_sock, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) < 0)
			throw std::invalid_argument( "\nConnection Failed \n" );
		send(_sock , hello , strlen(hello) , 0 );
    	printf("Hello message sent\n");
    	_valread = read( _sock , _buffer, 1024);
    	printf("%s\n",_buffer );
	}
	~Socket() {}

	void	check_connection(int domain, int type, int protocol)
	{
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock < 0)
			throw std::invalid_argument( "\n Socket creation error \n" );
	}
	
};
