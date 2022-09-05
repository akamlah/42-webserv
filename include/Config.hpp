/* ************************************************************************ */
/*                                                                          */
/*                              Class: Config                               */
/*                                                                          */
/* ************************************************************************ */

#ifndef __CONFIG_HPP__
# define __CONFIG_HPP__

# include <iostream>

namespace ws {

class Config {
	public:
		Config();
		Config(char *argv);
		~Config();
	bool checkValid(char *argv) const;
	bool checkContent(std::string const & configDataString) const;

	private:
	protected:
}; // class Config

} //namespase ws


#endif
