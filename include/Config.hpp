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
	
	std::vector<int>  helpChecPorts(std::string const & , std::string const &);
	std::vector<std::string> helpCheckHTTPmethods(std::string const & , std::string const &);
	bool helpGetDirecotry_listing(std::string const & configDataString, std::string const & checkThis);

	std::vector<ws::config_data> const & getAllConfigData() const;
	// std::map<int, ws::config_data> const & getAllConfigData() const;
	config_data const & getNumberConfigData(int number) const;
	
	config_data & getConfigData();

	int getnumberOfServers() const {return (numberOfServers);};

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
		config_data configData; // delted
		// std::map<int, ws::config_data &>	configDataAll;

		// new structure
		std::vector<ws::config_data>	configDataAll;
		int								numberOfServers;

		// end new structure
		std::vector<int>				_ports;
		int	port;
		std::string	host;
		std::string	root;
		std::string	index;
		config_data	_data;
}; // class Config

} //namespase ws


#endif
