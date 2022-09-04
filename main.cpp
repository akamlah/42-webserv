#include "ListenSocket.hpp"

#define PORT 8080

int main()
{
	ListenSocket	one_sock(AF_INET, SOCK_STREAM, 0, PORT);

	return 0;
}