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

Response::Response(const Request& request, const Tokens& tokens): _request(request), _status(request.status()),
    _is_persistent(request._is_persistent), _tokens(tokens) {
    // todo: distinguish methids! for now it is just GET
    __add_field("Server", "ZHero serv/1.0");
    // chunked request: ?
    // Transfer-Encoding: chunked ...
    __identify_resource();
    __handle_type(); // TODO: map function pointers ? have a decision tree system.
    __decide_persistency();
    __generate_response();
}

Response::Response(const Tokens& tokens): _tokens(tokens) {}

Response::~Response() {}

bool Response::is_persistent() const { return (_is_persistent); }

void Response::send(const int fd) { // more error handeling here too [ + ]
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

// adds a string formatted as <'field name': 'value'> to the header stream buffer
void Response::__add_field(const std::string& field_name, const std::string& value) {
    _fields_stream << field_name << ": " << value << CRLF;
}

// sets type and subtype of a resource in one function call for more readibility.
void Response::__set_type(const std::string& type, const std::string& subtype) {
    _resource.type = type;
    _resource.subtype = subtype;
}

// main funcitonalities

std::string Response::__generate_status_line() const {
    std::stringstream stream_status_line;
    stream_status_line << WS_HTTP_VERSION << SP << StatusPhrase()[status()];
    return (stream_status_line.str());
}

// identifies target path and type and adds content-type field to header
void Response::__identify_resource() {
    __identify_resource_path();
    __extract_resource_extension();
    __identify_resource_type();
}

void Response::__identify_resource_path() {
    std::string root;
    std::string file;
    if (status() == WS_200_OK) {
        root = "./example_sites/phptestsite";
        // root = "./example_sites/example2";

        // __find_site_root();    [ + ]

        // root = "./example_sites/example2";
        // root = config.root
        // if (!root) -> defaoult root
        // #include <fcntl.h>
        // #include <unistd.h>
        if (_request.header.target == "/") {
            file = "/index.html";
            // file = "/index.php"; //     [ + ]

            // if (open((root + file).c_str(), O_RDWR) < 0) .... some way to check if one exists !
            //     std::cout << "no such file" << std::endl;
            // file = config.index (try them all out)
        }
        else
            file = _request.header.target;
    }
    else { // assuming any other thing besides 200 ok is wrong for now (rdr?)

        // __find_error_root();    [ + ]

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
    _resource.path = root + file;
    std::cout << "PATH: " << _resource.path << std::endl;
}

void Response::__extract_resource_extension() {
    size_t pos = _resource.path.rfind('.');
    if (pos != _resource.path.npos)
        _resource.extension = _resource.path.substr(pos + 1);
    else
    _resource.extension = "";
}

// only if the extesion is mapped in 'tokens' content-type field is set.
// if not found type is set to extension.
void Response::__identify_resource_type() {
    std::map<std::string, std::string>::const_iterator it = _tokens.extensions.typemap.find(_resource.extension);
    if (it == _tokens.extensions.typemap.end()) {
        _resource.type = _resource.extension;
        return ;
    }
    size_t separator_pos = it->second.find('/');
    _resource.type = it->second.substr(0, separator_pos);
    if (separator_pos < it->second.npos)
        _resource.subtype = it->second.substr(separator_pos + 1);
    __add_field("Content-type", _resource.subtype.empty() ? _resource.type
        : (_resource.type + "/" + _resource.subtype));
}

void Response::__handle_type() {
    // temporarily handles every type, later have a decision tree
    if (_resource.extension == "php") { // and cgi in general -> ADD THE OTHER CASES    [ + ]
        __add_field("Cache-Control", "no-cache");
        // CALL CGI HERE - - - -- - - - - 
        	Cgi test;
            std::string phpresp;
            phpresp +=  test.executeCgi(_resource.path);
            _body << phpresp;
            return ;
    }
    else
    // are there other cases ?
    __buffer_target_body();
}

void Response::__buffer_target_body() { // + error handeling & target check here !
    if (DEBUG)
        std::cout << "BUFFERING BODY FROM TARGET: " << _resource.path << std::endl;
    try {
        std::ifstream fin(_resource.path, std::ios::in);
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

} // NAMESPACE http
} // NAMESPACE ws
