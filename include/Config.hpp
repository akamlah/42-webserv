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
# include <vector>

namespace ws {

typedef struct c_route {
	std::string					folder;
	std::string					root;
	std::string					index;
	std::string					http_redirects;
	std::string					download;
	std::string					cgi;
	bool						isCgiOn;
	bool						directory_listing;
	std::vector<std::string>	http_methods;
} config_route;

typedef struct c_data {
	std::vector<int>			ports;
	int							limit_body;
	std::string 				server_name;
	std::string					error;
	std::string					host;

	std::vector<ws::config_route>	routs;

	std::string					root;
	std::string					index;
	std::string					http_redirects;
	std::string					download;
	std::string					cgi;
	bool						isCgiOn;
	bool						directory_listing;
	std::vector<std::string>	http_methods;

	std::string					location;
} config_data;

class Config {
	public:
		Config();
		Config(std::string const & argv);
		~Config();
	bool checkValid(std::string const & argv);
	bool checkValidLocation(std::string const &);
	void checkContent(std::string const & configDataString);
	std::string helpCheckContent(std::string const & , std::string const &, bool );
	std::string helpCheckLocation(std::string const & , std::string const &, bool );
	
	std::vector<int>  helpChecPorts(std::string const & , std::string const &);
	std::vector<std::string> helpCheckHTTPmethods(std::string const & , std::string const &);

	config_route helpGetrouts(std::string const & );
	std::vector<ws::config_route> locationGetRouts(std::string const & );

	bool helpGetDirecotry_listing(std::string const & configDataString, std::string const & checkThis);
	bool check_ports_repeat(config_data const &);
	std::vector<ws::config_data> const & getAllConfigData() const;
	config_data const & getNumberConfigData(int number) const;
	
	
	int getnumberOfServers() const {return (numberOfServers);};
	
	class ConfigFileError: public std::exception {
		private:
			std::string msg;
		public:
			ConfigFileError(std::string const & msg);
			~ConfigFileError() _NOEXCEPT {}
			virtual const char* what() const throw();
	};

	private:
		std::vector<ws::config_data>	configDataAll;
		int								numberOfServers;

}; // class Config

} //namespase ws


#endif
