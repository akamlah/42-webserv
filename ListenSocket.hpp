#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>

class ListenSocket
{
private:
	int					_server_fd;
	int					_new_socket;
	long				_valread;
	int					_temp_int = 1;
	struct sockaddr_in	_address;
	int addrlen = sizeof(_address);
	char *default_greeting = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
public:
	ListenSocket(int domain, int type, int protocol, int port)
	{
		check_connection(domain, type, protocol);
		_address.sin_family = AF_INET;
		_address.sin_addr.s_addr = INADDR_ANY;
		_address.sin_port = htons( port );
		memset(_address.sin_zero, '\0', sizeof(_address.sin_zero));

		if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &_temp_int, sizeof(int)) < 0)
    		perror("setsockopt(SO_REUSEADDR) failed");
		if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
		{
			perror("Binding failed");
			exit(EXIT_FAILURE);
		}
		if (listen(_server_fd, 10) < 0)
		{
			perror("Server failed to listen");
			exit(EXIT_FAILURE);
		}
		while(1)
		{
			printf("\n+++++++ Waiting for new connection ++++++++\n\n");
			if ((_new_socket = accept(_server_fd, (struct sockaddr *)&_address, (socklen_t*)&addrlen))<0)
			{
				perror("Server could not accept new connections");
				exit(EXIT_FAILURE);
			}
			
			char buffer[30000] = {0};
			_valread = read( _new_socket , buffer, 30000);
			printf("%s\n",buffer );
			write(_new_socket , default_greeting , strlen(default_greeting));
			printf("------------------Hello message sent-------------------\n");
			close(_new_socket);
		}
	}
	~ListenSocket() { close(_server_fd); }

	int	check_connection(int domain, int type, int protocol)
	{
		_server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (_server_fd < 0)
			throw std::invalid_argument( "\n Socket creation error \n" );
	}
	
};
