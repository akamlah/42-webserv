/* ************************************************************************ */
/*                                                                          */
/*                              Class: Response                             */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Response.hpp"

namespace ws {
namespace http {

const char* Response::ResponseException::what() const throw() {
    return ("Response error");
}

const char* Response::throw_status(int status, const char* msg) const {
    #if DEBUG
    if (msg)
        std::cout << RED << msg << ": " << NC;
    std::cout << RED << "Error: " << StatusPhrase()[status] << NC << std::endl;
    #endif
    (void)status;
    (void)msg;
    throw Response::ResponseException();
}
bool Response::is_persistent() const { return (_is_persistent);}

// void Response::runSendCig( const std::string & path )
// {
// 	Cgi test;
// 	std::string phpresp;
// 	// phpresp = "HTTP/1.1 200 OK\nContent-Type: text/event-stream\nCache-Control: no-cache\n";
// 	// phpresp +=  test.executeCgi("./example_sites/phptestsite/send_sse.php");
// 	phpresp +=  test.executeCgi(path);
// 	std::stringstream buffer;

// 	buffer << _status_line << fields_stream.str() << CHAR_CR << CHAR_LF;
// 	std::string _response_str = buffer.str();

// 	buffer << phpresp << CHAR_CR << CHAR_LF; // write php data in

// 	_response_str = buffer.str();
// 	if (send(client_socket.fd, _response_str.c_str(), strlen(_response_str.c_str()), 0) < 0)
// 	throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
// 	std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
// }

// void Response::sendPicResp( const std::string & path )
// {
// 	// std::ifstream fin(path, std::ios::in | std::ios::binary);
// 	std::ifstream fin(path, std::ios::in);
// 	std::ostringstream oss;

// 	oss << fin.rdbuf();

// 	std::stringstream ss;
// 	ss << oss.str().length();
// 	std::string str = ss.str();
// 	fields_stream << CHAR_LF << "accept-ranges: bytes\ncontent-length: " << str << CHAR_CR << CHAR_LF;

// 	std::ostringstream buffer;

// 	buffer << _status_line << fields_stream.str() << CHAR_CR << CHAR_LF;

// 	std::string _response_str = buffer.str();  // write before the file staff

// 	buffer << oss.str() << CHAR_CR << CHAR_LF;
// 	_response_str = buffer.str();

// 	if (send(client_socket.fd,  &(*(_response_str.begin())) , _response_str.length(), 0) < 0)
// 	throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
// 	std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
// }

// void Response::createFieldStream()
// {
// 	if ((request.fields.find("sec-fetch-dest"))->second == "image") {
// 		// std::cout << "is image" << std::endl;
// 			// std::cout << "FUCK------111\n";
// 		if (_file.find(".ico") != std::string::npos)
// 			// fields_stream << "Content-Type: image/x-icon\nCache-Control: no-cache" << CHAR_CR << CHAR_LF;
// 			fields_stream << "Content-Type: image/x-icon"/*  << CHAR_CR << CHAR_LF */;
// 		else if (_file.find(".png") != std::string::npos)
// 			// fields_stream << "Content-Type: image/x-icon\nCache-Control: no-cache" << CHAR_CR << CHAR_LF;
// 			fields_stream << "Content-Type: image/png"/*  << CHAR_CR << CHAR_LF */;
// 		else
// 		{
// 			// fields_stream << "Content-Type: image/png\nCache-Control: no-cache" << CHAR_CR << CHAR_LF;
// 			fields_stream << "Content-Type: image/jpeg"/* << CHAR_CR << CHAR_LF */;
// 			// fields_stream << "Content-Type: image/jpeg" << CHAR_CR << CHAR_LF;
// 			// std::cout << "FUCK------\n";
// 		}
// 			// fields_stream << "Content-Type: image/x-icon" << CHAR_CR << CHAR_LF;
// 		// fields_stream << "Transfer-Encoding: chunked" << CHAR_CR << CHAR_LF;
// 		// fields_stream << "Connection: keep-alive" << CHAR_CR << CHAR_LF;
// 	}
// 	// cgi php part
// 	// if (std::string::size_type locate = _file.find(".php") != std::string::npos && locate + 4 == _file.length())
// 	if (_file.find(".php") != std::string::npos)
// 	{
// 		fields_stream << "Content-Type: text/event-stream\nCache-Control: no-cache" << CHAR_CR << CHAR_LF;
// 	}
// }

Response::Response(const Request& request, const int fd): _request(request), _status(request.status()),
    _is_persistent(request._is_persistent) {

    
    #if DEBUG
    std::cout << RED << "rsp: PARSED REQUEST STATUS: " << request.status() << NC << std::endl;
    std::cout << CYAN << "rsp: PARSED HEADER:\n" \
    << "\tMethod: " << request.header.method << "\n" \
    << "\tTarget: " << request.header.target << "\n" \
    << "\tVersion: " << request.header.version << NC << std::endl;
    // std::cout << CYAN << "rsp: PARSED FIELDS:\n";
    // for (std::map<std::string, std::string>::iterator it = request.fields.begin(); \
        // it != request.fields.end(); it++) {
    //     std::cout << it->first << "|" << it->second << std::endl;
    // }
    // std::cout << NC << std::endl;
    #endif

    __set_target_path();
    __set_content_type_field();

    #if DEBUG
    std::cout << "RESPONSE PATH = " << _path << std::endl;
    #endif

    if (request._is_persistent == true
        // && (request.field_is_value("connection", "keep-alive")
        && (((request.fields.find("connection"))->second == "keep-alive")
        // || request.field_is_value("connection", "chunked"))) {
        || ((request.fields.find("connection"))->second == "chunked"))) {
            _is_persistent = true;

            if ((request.fields.find("connection"))->second == "keep-alive")
                std::cout << "KEEP ALIVE" << std::endl;
            else
                std::cout << "not keep alive" << std::endl;
        }

    // FILEDS

    // 1. Server: ZHero-serv/1.0

    // 2. Content-type:
    if ((request.fields.find("sec-fetch-dest"))->second == "image") {
        // std::cout << "is image" << std::endl;
        _fields_stream << "Content-Type: image/jpeg" << CRLF;
        // fields_stream << "Transfer-Encoding: chunked" << CRLF;
        // fields_stream << "Connection: keep-alive" << CRLF;
    }

    // chunked request:
    // Transfer-Encoding: chunked

    // https://en.wikipedia.org/wiki/HTTP_persistent_connection -> keep-alive and timeouts

// -------- header composition ^^ ---------------

    try {
        std::ifstream page_file(_path);
        // if (!page_file.is_open()) ...
        std::stringstream buffer;

        buffer << __generate_status_line() << _fields_stream.str() << CRLF;
        std::string _response_str = buffer.str();
        buffer << page_file.rdbuf() << CRLF;
        _response_str = buffer.str();
        std::cout << "RESPONSE:\n" << _response_str << std::endl;

        if (::send(fd, _response_str.c_str(), strlen(_response_str.c_str()), 0) < 0)
            throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
        std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        throw_status(WS_500_INTERNAL_SERVER_ERROR);
    }
}

void Response::send(const int fd) {

    if (::send(fd, _response_str.c_str(), strlen(_response_str.c_str()), 0) < 0)
        throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data"); 
    std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
}


// - - - - - - - path/ utils - - - - - - - -

void Response::__add_field(const std::string& field_name, const std::string& value) {
    _fields_stream << field_name << ": " << value << CRLF;
}


static std::string __extension(const std::string& target) {
    size_t pos = target.rfind('.');
    if (pos != target.npos)
        return (target.substr(pos + 1));
    return ("");
}

static bool __is_image_extension(const std::string extension) {
    std::string poss_extensions[9] = \
        {"png", "jpg",  "jpeg",  "jfif", "pjpeg", "pjp", "svg", "avif", "webp"};
    int i = 0;
    while (i < 9) {
        if (extension == poss_extensions[i])
            return (true);
        i++;
    }
    return (false);
}

// application/javascript
// application/json
// application/ld+json
// application/msword (.doc)
// application/pdf
// application/sql
// application/vnd.api+json
// application/vnd.ms-excel (.xls)
// application/vnd.ms-powerpoint (.ppt)
// application/vnd.oasis.opendocument.text (.odt)
// application/vnd.openxmlformats-officedocument.presentationml.presentation (.pptx)
// application/vnd.openxmlformats-officedocument.spreadsheetml.sheet (.xlsx)
// application/vnd.openxmlformats-officedocument.wordprocessingml.document (.docx)
// application/x-www-form-urlencoded
// application/xml
// application/zip
// application/zstd (.zst)

void Response::__set_content_type_field() {
    std::string extension = __extension(_path);
    std::string subtype;

    std::cout << "" << (_request.fields.find("accept")->second) << std::endl;

    if (extension == "html") {
        _type = "text";
        subtype = "/html";
    }
    // if (__is_php())
    if (__is_image_extension(extension) || _request.field_is_value("sec-fetch-dest", "image"))
        // || (_request.fields.find("accept")->second).find("image") != _request.fields.find("accept")->second.npos)
    {
        _type = "image";
        subtype = "/" + extension;
    }
    // if (__is_text())
    // if (__is_text())
    std::cout << extension << std::endl;
    // set feild accordingly

    __add_field("Content-type", _type + subtype);
}

void Response::__set_target_path() {
    std::string root;
    std::string file;

    if (status() == WS_200_OK) {
        root = "./example_sites/example2";
        // root = config.root
        // if (!root) -> defaoult root

        if (_request.header.target == "/") {
            file = "/index.html";
            // file = config.index (try them all out)
        }
        else
            file = _request.header.target;
    }

    else { // assuming any other thing besides 200 ok is wrong for now (rdr?)
        root = "./default_pages/errors";
        // root = config.error_root
        // if (!root) -> defaoult root
        
        std::stringstream stream_file;
        stream_file << "/error_" << status() << ".html";
        file = stream_file.str();
    }
    // sets member attribute to full path to use in system calls
    _path = root + file;
}

std::string Response::__generate_status_line() const {
    std::stringstream stream_status_line;
    stream_status_line << WS_HTTP_VERSION << SP << StatusPhrase()[status()] << CRLF;
    return (stream_status_line.str());
}

Response::~Response() {}

} // NAMESPACE http
} // NAMESPACE ws
