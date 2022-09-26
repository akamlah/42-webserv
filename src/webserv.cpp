#include "../include/Server.hpp"
#include "../include/utility.hpp"
#include "../include/Config.hpp"

#define PORT 8001
#define BACKLOG 100


int main(int argc, char **argv) {

	if (argc != 2)
	{
		std::cout << "Wrong number of argument!.. try: ./webserv [configuration file]\n";
		return (0);
	}
	try
	{
		ws::Config configData(argv[1]);
		ws::Server server(configData.getAllConfigData());

		server.listen(BACKLOG);
        server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (-1);
	}
	#if DEBUG
	system("leaks webserv | tail - 3");
	#endif
	return (0);
}
