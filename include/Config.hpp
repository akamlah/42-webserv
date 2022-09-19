/* ************************************************************************ */
/*                                                                          */
/*                              Class: Config                               */
/*                                                                          */
/* ************************************************************************ */

#ifndef __CONFIG_HPP__
# define __CONFIG_HPP__

# include <iostream>
# include  "../include/utility.hpp"
# include <map>

namespace ws {

class Config {
	public:
		Config(char *argv);
		~Config();
	bool checkValid(char *argv);
	void checkContent(std::string const & configDataString);
	std::string helpCheckContent(std::string const & , std::string const &, bool );

	std::map<int, ws::config_data> const & getAllConfigData() const;
	config_data const & getNumberConfigData(int number) const;
	
	config_data & getConfigData();
	void setConfigData();
	
	class ConfigFileError: public std::exception {
		private:
			std::string msg;
		public:
			ConfigFileError(std::string const & msg);
			~ConfigFileError() _NOEXCEPT {}
			virtual const char* what() const throw();
	};

	private:
		Config(); // pdf says that it has to run with argv so we dont have defult?
	protected:
		config_data configData;
		std::map<int, ws::config_data>	configDataAll;
		std::vector<int>				_ports;
		int								numberOfServers;
		int	port;
		std::string	host;
		std::string	root;
		std::string	index;
		config_data	_data;
}; // class Config

} //namespase ws


#endif
