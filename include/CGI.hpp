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
	void readHTML(std::string const & htmlFilePath);
	void runRaw_perl(std::string const &);
	std::string executeCgiNew(char **env);
	std::string executeCgi_perl(std::string const &); // have to be in the correct location.
	std::string createTemp_perl(std::string const &);
	std::string const find_perl_tag(std::string const & htmlfile);

private:
}; // CLASS Cgi

} // NAMESPACE ws

#endif //CGI
