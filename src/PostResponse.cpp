#include "../include/Response.hpp"

namespace ws {
namespace http {

	static bool is_directory(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

	void Response::method_post()
	{
		if (getValid("POST"))
			throw_error_status(WS_405_METHOD_NOT_ALLOWED, "Method forbidden by config file");
		if (_config.isCgiOn && _config.cgi.compare(".php") == 0 && (_resource.extension == "php" || _resource.extension == "html")) {
			if (_config.download != "non" && !(is_directory(_config.root + _config.download)))
				throw_error_status(WS_404_NOT_FOUND, "Download folder not found");
			respond_cgi_post();
		}
		else {
			add_field("Content-type", _resource.subtype.empty() ? _resource.type : (_resource.type + "/" + _resource.subtype));
			respond_get();
		}
	}

	void Response::respond_post() {
		upload_file();
		response_to_string();
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// POST CGI
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void Response::respond_cgi_post()
	{
		add_field("accept-ranges", "bytes");
		add_field("Cache-Control", "no-cache");
		int templength;
		std::stringstream response;
		Cgi test;
		std::string phpresp;
		phpresp +=  cgiRespCreator_post();
		std::string::size_type separationindex;
		if (phpresp.empty())
			return ;
		separationindex = phpresp.find("\r\n\r\n");
		if (separationindex == std::string::npos)
			return ;
		_body << phpresp;
		std::string temp = phpresp.substr(separationindex + 4);
		templength = temp.length();
		add_field("Content-length", std::to_string(templength));
		response << generate_status_line() << CRLF;
		response << _fields_stream.str();
		response << _body.str();
		_response_str = response.str();
		return ;
	}

	std::string Response::cgiRespCreator_post()
	{
		char ** env;
		env = new char*[14];
		int i = 0;
		env[i++] = &(*((new std::string(_request._body.str())))->begin());
		env[i++] = &(*((new std::string("CONTENT_LENGTH=" + contentLength_for_post()))->begin()));
		env[i++] = &(*((new std::string("REQUEST_METHOD=" + _request.header.method)))->begin());
		env[i++] = &(*((new std::string("PATH_TRANSLATED=" + _resource.abs_path ))->begin()));
		env[i++] = &(*((new std::string("PATH_INFO=" + _resource.abs_path ))->begin()));
		env[i++] = &(*((new std::string("SERVER_PROTOCOL=HTTP/1.1")))->begin());
		env[i++] = &(*((new std::string("GATEWAY_INTERFACE=CGI/1.1")))->begin());
		env[i++] = &(*((new std::string("REDIRECT_STATUS=200")))->begin());
		env[i++] = &(*((new std::string("CONTENT_TYPE=" +   contentType_for_post() ))->begin()));
		env[i++] = &(*((new std::string("QUERY_STRING=" + _resource.query)))->begin());
		env[i++] = NULL;
		Cgi test;
		std::string phpresp;
		phpresp += test.executeCgiNew(env);
		delete [] env;
		return (phpresp);
	}	


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Utilities
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string Response::contentLength_for_post()
	{
		std::list<std::string> konttype;
		std::list<std::string>::iterator it;
		konttype = _request.get_field_value("content-length");
		std::string tempLength;
		if ( !(konttype.empty()) )
		{
			it = konttype.begin();
			while (it != konttype.end())
			{
				tempLength += *it;
				#if DEBUG
					std::cout << "length:\n" << tempLength << std::endl;
				#endif
				it++;
			}
		}
		return (tempLength);
	}

	std::string Response::contentType_for_post()
	{
		std::list<std::string> konttype = _request.get_field_value("content-type");
		std::list<std::string>::iterator it;
		std::string tmp;
		if ( !(konttype.empty()) )
		{
			it = konttype.begin();
			while (it != konttype.end())
			{
				tmp += *it;
				#if DEBUG
					std::cout << "type:\n" << tmp << std::endl;
				#endif
				it++;
			}
		}
		return (tmp);
	}




} // NAMESPACE http
} // NAMESPACE ws
