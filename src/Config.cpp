#include "../include/Config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

ws::Config::Config() {}
ws::Config::Config(char *argv) {
	if (ws::Config::checkValid(argv))
		std::cout << "The config file is valid, You may procide!\n";
	else
	{

		std::cout << "The Config file is invalid!\n";
		exit(EXIT_FAILURE);
	}
}

ws::Config::~Config() {}

bool ws::Config::checkValid(char *argv) const
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
	// std::string fileContent(buffer.str());
	checkContent(buffer.str());

	return (true);
}

bool ws::Config::checkContent(std::string const & configDataString) const 
{

}
