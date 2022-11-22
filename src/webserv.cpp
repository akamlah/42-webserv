#include "../include/Server.hpp"
#include "../include/utility.hpp"
#include "../include/Config.hpp"

#define PORT 8001

int main(int argc, char **argv) {

	if (argc > 2)
	{
		std::cout << "Wrong number of argument!.. try: ./webserv [configuration file]\n";
		return (0);
	}
	try
	{
		std::string pagename;
		if (argc == 2)
			pagename = argv[1];
		else
			pagename = "./default/default.conf";
		
		ws::Config configData(pagename);

		#if DEBUG
			std::cout << configData.getNumberConfigData(0).cgi << std::endl;
			std::cout << std::boolalpha << "direcotry listing: " << configData.getNumberConfigData(0).directory_listing << std::endl;
			std::cout << configData.getNumberConfigData(0).download << std::endl;
			std::cout << configData.getNumberConfigData(0).error << std::endl;
			std::cout << configData.getNumberConfigData(0).host << std::endl;
			std::cout << configData.getNumberConfigData(0).http_methods[0] << std::endl;
			std::cout << configData.getNumberConfigData(0).http_redirects << std::endl;
			std::cout << configData.getNumberConfigData(0).index << std::endl;
			std::cout << std::boolalpha << configData.getNumberConfigData(0).isCgiOn << std::endl;
			std::cout << configData.getNumberConfigData(0).limit_body << std::endl;
			std::cout << configData.getNumberConfigData(0).ports[0] << std::endl;
		#endif
		
		ws::Server server(configData.getAllConfigData());
        server.run();
	}
	catch (std::invalid_argument const& ex)
    {
        std::cerr << "Error in config, field not a number:\n" << ex.what() << '\n';
    }
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (-1);
	}
	std::cout << "Returning" << std::endl;
	return (0);
}
