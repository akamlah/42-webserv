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

	Config::Config() {
		std::ifstream confFile;
		confFile.open("./default/default.conf", std::ios::in);
		if (confFile.fail())
			throw ConfigFileError("ERROR: Couldn't open config file");
		std::stringstream buffer;
		buffer << confFile.rdbuf();
		std::string fullConfile;
		fullConfile = buffer.str();
		numberOfServers = 1;
		std::string::size_type closingsingLoc = 0;
		std::string::size_type startsignLoc = 0;
		checkContent(fullConfile.substr(startsignLoc, closingsingLoc - startsignLoc));
	}

	Config::Config(std::string const & argv)
	{
		if (argv.empty())
			throw ConfigFileError("The Config file is empty\n");
		else if (checkValid(argv))
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

	bool Config::checkValid(std::string const & argv)
	{
		if (argv.empty())
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
		if (buffer.fail())
			return (false);
		//cut
		std::string fullConfile;
		fullConfile = buffer.str();
		if (fullConfile.empty())
			return (false);
		numberOfServers = 0;
		std::string::size_type closingsingLoc = 0;
		std::string::size_type startsignLoc = 0;
		std::string::size_type test = fullConfile.length();
		do
		{
			closingsingLoc++;
			closingsingLoc = fullConfile.find('}', closingsingLoc);
			if (closingsingLoc == std::string::npos)
				return (false);
			checkContent(fullConfile.substr(startsignLoc, closingsingLoc - startsignLoc));
			startsignLoc = closingsingLoc + 1;
			numberOfServers++;
		}
		while (test > closingsingLoc + 9 && fullConfile[closingsingLoc + 9] == '{');
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
			while (!(std::isprint(configDataString[i])) || configDataString[i] == ' ')
				i++;
			if (configDataString[i] == ';')
				break;
			if (isNumber && std::isdigit(configDataString[i]))
				temp += configDataString[i];
			else if (std::isprint(configDataString[i]))
				temp += configDataString[i];
			else
				throw ConfigFileError("ERROR: " + checkThis + " wrong format in config file!");
		}
		if (temp.empty())
			throw ConfigFileError("ERROR: Missing " + checkThis);
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
			while (!(std::isprint(configDataString[i])) || configDataString[i] == ' ')
				i++;
			if (configDataString[i] == ';')
				break;
			temp += configDataString[i];
		}
		if (temp == "on")
			return (true);
		else if (temp == "off")
			return (false);
		else
			throw ConfigFileError("ERROR: " + checkThis + " wrong format in config file!");
		
	}

	bool Config::check_ports_repeat(config_data const & actual)
	{
		std::vector<ws::config_data>::iterator it = configDataAll.begin();
		std::vector<ws::config_data>::iterator itEnd = configDataAll.end();
		while (it !=itEnd)
		{
			std::vector<int>::const_iterator vit2 = actual.ports.begin();
			std::vector<int>::const_iterator vitend2 = actual.ports.end();
			while (vit2 != vitend2)
			{
				std::vector<int>::iterator vit = (*it).ports.begin();
				std::vector<int>::iterator vitend = (*it).ports.end();
				while (vit != vitend)
				{
					if ((*vit) == (*vit2))
						return (true);
					vit++;
				}
				vit2++;
			}
			it++;
		}
		return (false);
	}

	void Config::checkContent(std::string const & configDataString)
	{
		config_data temp;

		temp.ports = helpChecPorts(configDataString, "port:"); // new multiple ports.

		temp.limit_body = std::stoi(helpCheckContent(configDataString, "limit_body:", true));
		temp.server_name = helpCheckContent(configDataString, "server_name:", false);
		temp.error = helpCheckContent(configDataString, "error:", false);
		temp.host = helpCheckContent(configDataString, "host:", false);
		temp.root = helpCheckContent(configDataString, "root:", false);
		temp.index = helpCheckContent(configDataString, "index:", false);
		temp.http_redirects = helpCheckContent(configDataString, "http_redirects:", false);
		temp.download = helpCheckContent(configDataString, "download:", false);
		temp.http_methods = helpCheckHTTPmethods(configDataString, "http_methods:");
		temp.cgi = helpCheckContent(configDataString, "cgi:", false);
		temp.location = helpCheckContent(configDataString, "location:", false);
		if (temp.cgi.find("non") == std::string::npos)
			temp.isCgiOn = true;
		else
			temp.isCgiOn = false;
		temp.directory_listing = helpGetDirecotry_listing(configDataString, "directory_listing:");
		if (temp.directory_listing == true && temp.location != "non")
			throw ConfigFileError("ERROR: Conflict in settings! You can't set your directory visible if you hide your location!");
		if (check_ports_repeat(temp))
			throw ConfigFileError("ERROR: ports duplicate in server");
		configDataAll.push_back(temp);
	}

	std::vector<int> Config::helpChecPorts(std::string const &  configDataString, std::string const & checkThis )
	{
		std::vector<int> vectorTemp;
		int port = 0;
		std::string::size_type portPlace = configDataString.find(checkThis);
		if (portPlace == std::string::npos)
			throw ConfigFileError("ERROR: " + checkThis + " Missing from config file!");
		std::string temp;
		portPlace += checkThis.length();
		while (!(std::isprint(configDataString[portPlace])) || configDataString[portPlace] == ' ')
			portPlace++;
		for (std::string::size_type i = portPlace; configDataString[i] != ';'; i++)
		{
			while (!(std::isprint(configDataString[i])) || configDataString[i] == ' ')
				i++;
			if (configDataString[i] == ';')
				break;
			if (configDataString[i] == ',')
			{
				if (temp.empty())
					throw ConfigFileError("ERROR: Missing port.");
				port = std::stoi(temp);
				if (port < 0 || port > 65535)
					throw ConfigFileError("ERROR: invalid port.");
				vectorTemp.push_back(port);
				temp.clear();
			}
			else if (std::isdigit(configDataString[i]))
				temp += configDataString[i];
			else
			{
				std::cout << configDataString[i];
				throw ConfigFileError("ERROR: " + checkThis + " wrong format in config file!");
			}
		}
		if (temp.empty())
			throw ConfigFileError("ERROR: Missing port.");
		port = std::stoi(temp);
		if (port < 0 || port > 65535)
			throw ConfigFileError("ERROR: invalid port.");
		vectorTemp.push_back(port);
		return (vectorTemp);
	}

	std::vector<std::string> Config::helpCheckHTTPmethods(std::string const &  configDataString, std::string const & checkThis )
	{
		std::string methods("GET POST PUT HEAD DELETE PATCH OPTIONS CONNECT TRACE");
		std::vector<std::string> vectorTemp;
		std::string::size_type portPlace = configDataString.find(checkThis);
		if (portPlace == std::string::npos)
			throw ConfigFileError("ERROR: " + checkThis + " Missing from config file!");
		std::string temp;
		portPlace += checkThis.length();
		while (!(std::isprint(configDataString[portPlace])) || configDataString[portPlace] == ' ')
			portPlace++;
		for (std::string::size_type i = portPlace; configDataString[i] != ';'; i++)
		{
			while (!(std::isprint(configDataString[i])) || configDataString[i] == ' ')
				i++;
			if (configDataString[i] == ';')
				break;
			if (configDataString[i] == ',')
			{
				if (temp.empty())
					throw ConfigFileError("ERROR: Missing " + checkThis);
				if (methods.find(temp) == std::string::npos)
					throw ConfigFileError("ERROR: Wrong " + checkThis + " " + temp);
				vectorTemp.push_back(temp);
				temp.clear();
			}
			else if (std::isprint(configDataString[i]))
				temp += configDataString[i];
			else
				throw ConfigFileError("ERROR: " + checkThis + " wrong format in config file!");
		}
		if (temp.empty())
			throw ConfigFileError("ERROR: Missing " + checkThis);
		if (methods.find(temp) == std::string::npos)
			throw ConfigFileError("ERROR: Wrong " + checkThis + " " + temp);
		vectorTemp.push_back(temp);
		return (vectorTemp);
	}
	
	std::vector<ws::config_data> const & Config::getAllConfigData() const { return (configDataAll); }
	config_data const & Config::getNumberConfigData(int number) const { return (configDataAll[number]);}

} // namspace ws
