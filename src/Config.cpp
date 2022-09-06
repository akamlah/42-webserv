#include "../include/Config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

namespace ws {

	Config::ConfigFileError::ConfigFileError(std::string const & msg) :msg(msg) {}

	const char* Config::ConfigFileError::what() const throw() {
		return (msg.c_str());
	}
	Config::Config(): port(4200), root("html"), index("inex.html") {}
	Config::Config(char *argv) {
		if (Config::checkValid(argv))
			std::cout << "The config file is valid, You may procide!\n";
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
		checkContent(buffer.str());

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

	void Config::checkContent(std::string const & configDataString)
	{
		// std::string temp = helpCheckContent(configDataString, "port:", true);
		// std::cout << temp << std::endl;
		this->port = std::stoi(helpCheckContent(configDataString, "port:", true));
		this->root = helpCheckContent(configDataString, "root:", false);
		// std::cout << this->root << std::endl;
		this->index = helpCheckContent(configDataString, "index:", false);
		// this->index = helpCheckContent(configDataString, "index:", false);
		// std::cout << this->index << std::endl;

	}

	config_data const Config::getConfigData() const {
		config_data temp;
		temp.port = port;
		temp.root = root;
		temp.index = index;
		return (temp);
	}


} // namspace ws
