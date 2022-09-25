#include "../include/Config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

namespace ws {

	Config::ConfigFileError::ConfigFileError(std::string const & msg) :msg(msg) {}

	const char* Config::ConfigFileError::what() const throw() 
	{
		return (msg.c_str());
	}
	Config::Config(): port(4200), root("html"), index("inex.html") {}

	Config::Config(char *argv)
	{
		if (Config::checkValid(argv))
		{
			if (DEBUG)
				std::cout << "The config file is valid, You may procede!\n";
		}
		else
		{
			throw ConfigFileError("The Config file is invalid!\n");
		}
	}

	Config::~Config() {}

	bool Config::checkValid(char *argv)
	{
		if (!argv)
			return (false);

		std::string temp = argv;
		if (temp.size() <= 0)
			return (false);
		std::string::size_type ret;
		ret = temp.find(".conf");
		if (ret == std::string::npos || ret != temp.length() - 5)
			return (false);
		std::ifstream confFile;
		confFile.open(argv, std::ios::in);
		if (confFile.fail())
			return (false);
		char ch;
		confFile >> ch;
		if (ch != '{')
			return (false);
		std::stringstream buffer;
		buffer << confFile.rdbuf();

		//cut
		std::string fullConfile;
		fullConfile = buffer.str();
		numberOfServers = 0;
		std::string::size_type closingsingLoc = -1;
		std::string::size_type startsignLoc = 0;
		do
		{
			// std::cout << "I'm here\n";
			closingsingLoc++;
			closingsingLoc = fullConfile.find('}', closingsingLoc);

			// std::cout << "post }; " << closingsingLoc << std::endl;
			// std::cout << "caracter after " << fullConfile[closingsingLoc + 1] << std::endl;
			
			if (closingsingLoc == std::string::npos )
				return (false);
			checkContent(fullConfile.substr(startsignLoc, closingsingLoc - startsignLoc));
			startsignLoc = closingsingLoc + 1;
			numberOfServers++;
		}
		while (fullConfile[closingsingLoc + 9] == '{');

		// checkContent(buffer.str());

		return (true);
	}

	std::string Config::helpCheckContent(std::string const & configDataString, std::string const & checkThis, bool isNumber)
	{
		std::string::size_type portPlace = configDataString.find(checkThis);
		if (portPlace == std::string::npos)
			throw ConfigFileError("ERROR: " + checkThis + " Missing from config file!");
		std::string temp;
		portPlace += checkThis.length();
		while (!(std::isprint(configDataString[portPlace])) || configDataString[portPlace] == ' ')
			portPlace++;
		for (std::string::size_type i = portPlace; configDataString[i] != ';'; i++)
		{
			if (isNumber && std::isdigit(configDataString[i]))
				temp += configDataString[i];
			else if (std::isprint(configDataString[i]))
				temp += configDataString[i];
			else
				throw ConfigFileError("ERROR: " + checkThis + " wrong format in config file!");
		}
		return (temp);
	}

	bool Config::helpGetDirecotry_listing(std::string const &  configDataString, std::string const & checkThis )
	{
		std::string::size_type portPlace = configDataString.find(checkThis);
		if (portPlace == std::string::npos)
			throw ConfigFileError("ERROR: " + checkThis + " Missing from config file!");
		std::string temp;
		portPlace += checkThis.length();
		while (!(std::isprint(configDataString[portPlace])) || configDataString[portPlace] == ' ')
			portPlace++;
		for (std::string::size_type i = portPlace; configDataString[i] != ';'; i++)
		{
				temp += configDataString[i];
		}
		if (temp.compare("on"))
			return (true);
		else if (temp.compare("off"))
			return (false);
		else
			throw ConfigFileError("ERROR: " + checkThis + " wrong format in config file!");
		
	}

	void Config::checkContent(std::string const & configDataString)
	{
		// std::string temp = helpCheckContent(configDataString, "port:", true);
		// std::cout << temp << std::endl;
		config_data temp;
		temp.port = std::stoi(helpCheckContent(configDataString, "port:", true));
		temp.limit_body = std::stoi(helpCheckContent(configDataString, "limit_body:", true));
		temp.server_name = helpCheckContent(configDataString, "server_name:", false);
		temp.error = helpCheckContent(configDataString, "error:", false);
		temp.host = helpCheckContent(configDataString, "host:", false);
		temp.root = helpCheckContent(configDataString, "root:", false);
		temp.index = helpCheckContent(configDataString, "index:", false);
		temp.http_redirects = helpCheckContent(configDataString, "http_redirects:", false);
		temp.download = helpCheckContent(configDataString, "download:", false);
		temp.cgi = helpCheckContent(configDataString, "cgi:", false);
		if (temp.cgi.find("non") == std::string::npos)
			temp.isCgiOn = true;
		else
			temp.isCgiOn = false;
		temp.directory_listing = helpGetDirecotry_listing(configDataString, "directory_listing:");
		std::cout << "inside: " << temp.port << std::endl;
		configDataAll.push_back(temp);
	}
	
	void Config::setConfigData() {

		_data.ports.push_back(54000);
		_data.ports.push_back(18000);
		_data.ports.push_back(8001);
/////

	}

	config_data& Config::getConfigData() { return (_data); }

	std::vector<ws::config_data> const & Config::getAllConfigData() const { return (configDataAll); }

} // namspace ws
