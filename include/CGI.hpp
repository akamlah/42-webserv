/* ************************************************************************ */
/*																			*/
/*								Class: CGI									*/
/*																			*/
/* ************************************************************************ */

#ifndef CGI_HPP
# define CGI_HPP
# include <iostream>


// basic testing beta version
namespace ws {

class Cgi {
public:
	Cgi();
	~Cgi();
	void readHTML();
	std::string & executeCgi(std::string const & phpfile); // have to be in the correct location.
	std::string & createTempPHP(std::string const & phpCode);
	std::string & findPHPtag(std::string const & htmlfile);

private:
}; // CLASS Cgi

} // NAMESPACE ws

#endif //CGI