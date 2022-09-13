/* ************************************************************************ */
/*																			*/
/*								Class: CGI									*/
/*																			*/
/* ************************************************************************ */

#ifndef CGI_HPP
# define CGI_HPP

// basic testing beta version
namespace ws {

class Cgi {
public:
	Cgi();
	~Cgi();
	std::string executeCgi(std::string const & phpfile);

private:
}; // CLASS Cgi

} // NAMESPACE ws

#endif //CGI