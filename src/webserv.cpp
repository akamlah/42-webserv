#include "../include/Server.hpp"
#include "../include/Socket.hpp"
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
		configData.setConfigData();

		// std::cout << "port in 0: " << configData.getAllConfigData()[1].http_methods[0] << std::endl;
		// std::cout << "port in 1: " << configData.getAllConfigData()[1].http_methods[1] << std::endl;
		// std::cout << "port in 1: " << configData.getAllConfigData()[1].port << std::endl;
		// std::cout << "port in 2: " << configData.getAllConfigData()[2].port << std::endl;
		std::cout << "number of server: " << configData.getnumberOfServers() <<  std::endl;


// while going over the vector of config files... ---> run each of them 
		ws::Server server(configData.getConfigData());


		server.listen(BACKLOG);



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
