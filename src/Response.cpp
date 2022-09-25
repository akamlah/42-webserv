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

Response::Response(const Request& request): _request(request), _status(request.status()),
    _is_persistent(request._is_persistent) {
    
    // todo: distinguish methids! for now it is just GET
    
    __add_field("Server", "ZHero serv/1.0");
    // chunked request: ?
    // Transfer-Encoding: chunked ...
    __set_target_path();
    __set_content_type_field();
    __handle_type(); // TODO: map function pointers ? have a decision tree system.
    __decide_persistency();
    __generate_response();
}

Response::~Response() {}

bool Response::is_persistent() const { return (_is_persistent); }

void Response::send(const int fd) { // more error handeling here too
    // &(*(_response_str.begin()))
    if (DEBUG)
        std::cout << "SENDING RESPONSE:\n" << _response_str;

    if (::send(fd, _response_str.c_str(), _response_str.length(), 0) < 0)
        throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");

    std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
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

// - - - - - - - - - - - PRIVATE - - - - - - - - - - - - - 

// utilities

void Response::__add_field(const std::string& field_name, const std::string& value) {
    _fields_stream << field_name << ": " << value << CRLF;
}

static std::string __extension(const std::string& target) {
    size_t pos = target.rfind('.');
    if (pos != target.npos)
        return (target.substr(pos + 1));
    return ("");
}

void Response::__set_type(const std::string& type, const std::string& subtype) {
    _type = type;
    _subtype = subtype;
}

// main funcitonalities

std::string Response::__generate_status_line() const {
    std::stringstream stream_status_line;
    stream_status_line << WS_HTTP_VERSION << SP << StatusPhrase()[status()];
    return (stream_status_line.str());
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
        if (_request.header.target == "/") {
            stream_file << "/error_" << status() << ".html";
            file = stream_file.str();
        }
        else
            file = _request.header.target;
    }
    // sets member attribute to full path to use in system calls
    _path = root + file;
}

void Response::__set_content_type_field() {
    _extension = __extension(_path);
    if (_extension == "html")
        __set_type("text", "html");
    else if (_extension == "css")
        __set_type("text", "css");
    else if (_extension == "php")
        __set_type("text", "event-stream");
    else if (_extension == "jpg" || _extension == "jpeg" || _extension == "jfif" || _extension == "pjpeg" || _extension == "pjp")
        __set_type("image", "jpeg");
    else if (_extension == "png" || _extension == "avif" || _extension == "webp")
        __set_type("image", _extension);
    else if (_extension == "svg")
        __set_type("image", "svg+xml");
    else if (_extension == "ico")
        __set_type("image", "x-icon");
    else
        __set_type("application", _extension); // +

    __add_field("Content-type", _type + "/" + _subtype);
}

void Response::__handle_type() {
    // temporarily handles every type, later have a decision tree
    if (_extension == "php") {
        __add_field("Cache-Control", "no-cache");
        // CALL CGI HERE - - - -- - - - - 
    }
    // are there other cases ?
    __buffer_target_body();
}

void Response::__buffer_target_body() { // + error handeling & target check here !
    if (DEBUG)
        std::cout << "BUFFERING BODY FROM TARGET: " << _path << std::endl;
    try {
        std::ifstream fin(_path, std::ios::in);
        // if (!page_file.is_open()) ...
        _body << fin.rdbuf();
        if (!_body.str().empty())
            __add_field("Content-length", std::to_string(_body.str().length()));
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        throw_status(WS_500_INTERNAL_SERVER_ERROR);
    }
}

void Response::__decide_persistency() {
    if (_request._is_persistent == true && (_request.field_is_value("connection", "keep-alive")
        || _request.field_is_value("connection", "chunked"))) {
        _is_persistent = true;
        if ((_request.fields.find("connection"))->second == "keep-alive")
            std::cout << "KEEP ALIVE" << std::endl;
        else
            std::cout << "not keep alive" << std::endl;
        }
}

void Response::__generate_response() {
    std::stringstream response;
    response << __generate_status_line() << CRLF
        << _fields_stream.str() << CRLF
        << _body.str() << CRLF;
    _response_str = response.str();
}

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

} // NAMESPACE http
} // NAMESPACE ws
