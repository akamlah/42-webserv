#include "../include/Response.hpp"

namespace ws {
namespace http {

	void Response::method_get()
	{
		if (getValid("GET"))
			throw_error_status(WS_405_METHOD_NOT_ALLOWED, "Method forbidden by config file");
		if (_config.isCgiOn && _config.cgi.compare(".php") == 0 && (_resource.extension == "php" || _resource.extension == "html")) {
			respond_cgi_get();
		}
		else if (_config.isCgiOn && _config.cgi.compare(".pl") == 0 && (_resource.extension == "pl" || _resource.extension == "html")) {
			respond_cgi_get_perl();
		}
		else {
			add_field("Content-type", _resource.subtype.empty() ? _resource.type : (_resource.type + "/" + _resource.subtype));
			#if DEBUG
				std::cout << CYAN << "Content-type: " <<  (_resource.subtype.empty() ? _resource.type : (_resource.type + "/" + _resource.subtype));
				std::cout << NC << std::endl;
			#endif
			respond_get();
		}
	}

	void Response::respond_get() {
		add_field("Accept-Ranges", "bytes");
		upload_file();
		response_to_string();
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// GET CGI
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


	void Response::respond_cgi_get()
	{
		add_field("accept-ranges", "bytes");
		add_field("Cache-Control", "no-cache");
		int templength;
		std::stringstream response;
		Cgi test;
		std::string phpresp;
		phpresp +=  cgiRespCreator();
		std::string::size_type shitindex;
		if (phpresp.empty())
			throw_error_status(WS_500_INTERNAL_SERVER_ERROR , "We are terripbly sorry,Something went wrong! Most likly it's your fault!!!");
		shitindex = phpresp.find("\r\n\r\n");
		if (shitindex == std::string::npos)
			return ;
		_body << phpresp;
		std::string temp = phpresp.substr(shitindex + 4);
		templength = temp.length();
		add_field("Content-length", std::to_string(templength));
		response << generate_status_line() << CRLF;
		response << _fields_stream.str();
		response << _body.str();
		_response_str = response.str();
		return ;
	}

	std::string Response::cgiRespCreator()
	{
			char ** env;
			env = new char*[10];
			fill_up_env(env);
			Cgi test;
			std::string phpresp;
			phpresp += test.executeCgiNew(env);
			if (DEBUG && phpresp.empty())
				std::cout << "unfortunetly this has nothing inside!\n";
			delete [] env;
		return (phpresp);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Utilities
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void Response::fill_up_env(char **env)
	{
		int i = 0;
		std::string cokie;
		std::list<std::string> tempcokie = _request.get_field_value("cookie");
		if (!(tempcokie.empty()))
			cokie = *(tempcokie.begin());
		else if (DEBUG)
			std::cout << "\nSORRYYYYY there is no cokie in the request\n";
		env[i++] = &(*((new std::string(cokie)))->begin());
		env[i++] = &(*((new std::string("REQUEST_METHOD=" + _request.header.method)))->begin());
		env[i++] = &(*((new std::string("PATH_TRANSLATED=" + _resource.abs_path   ))->begin()));
		env[i++] = &(*((new std::string("REDIRECT_STATUS=200")))->begin());
		env[i++] = &(*((new std::string("QUERY_STRING=" + _resource.query)))->begin());
		env[i++] = &(*((new std::string("HTTP_COOKIE=" + cokie)))->begin());
		env[i++] = NULL;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// perl
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void Response::respond_cgi_get_perl()
	{
		std::stringstream response;
		Cgi test;
		std::string phpresp;
		phpresp +=  perl_cgiRespCreator();
		if (phpresp.empty())
			throw_error_status(WS_500_INTERNAL_SERVER_ERROR , "We are terripbly sorry,Something went wrong! Most likly it's your fault!!!");
		_body << phpresp;
		add_field("Content-length", std::to_string(phpresp.length()));
		add_field("Content-type", "text/html");
		response << generate_status_line() << CRLF;
		response << _fields_stream.str() << CRLF;
		response << _body.str();
		_response_str = response.str();
		return ;
	}

	std::string Response::perl_cgiRespCreator()
	{

		Cgi test;
		std::string perlresp;
		perlresp += test.executeCgi_perl(_resource.abs_path);
		if (DEBUG && perlresp.empty())
			std::cout << "PERL unfortunetly this has nothing inside!\n";
		return (perlresp);
	}

} // NAMESPACE http
} // NAMESPACE ws
