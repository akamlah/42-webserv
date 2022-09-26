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


Response::Response(const Request& request, const config_data& config, const Tokens& tokens):
    _request(request), _config(config), _tokens(tokens), _status(request.status()),
    _is_persistent(request._is_persistent) {

    __build_response();
}

// Response::Response(const Tokens& tokens): _tokens(tokens) {}

Response::~Response() {}

bool Response::is_persistent() const { return (_is_persistent); }

void Response::send(const int fd) { // more error handeling here too [ + ]
    if (DEBUG)
        std::cout << "SENDING RESPONSE:\n" << _response_str;
    if (::send(fd, _response_str.c_str(), _response_str.length(), 0) < 0)
        error_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
    std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
}

int Response::error_status(int status, const char* msg) {
    if (DEBUG) {
        if (msg)
            std::cout << RED << msg << ": " << NC;
        std::cout << RED << "Error: " << _tokens.status_phrases[status] << NC << std::endl;
    }
    // throw Response::ResponseException();
    _status = status;
    // __respond_to_error(); ?
    return (status);
}

// - - - - - - - - - - - PRIVATE - - - - - - - - - - - - - 

static bool is_success_code(const int n) {
    if (n >= 200 && n < 300)
        return (true);
    return (false);
}

void Response::__build_response() {
    __add_field("Server", "ZHero serv/1.0");
    __add_formatted_timestamp();
    __add_field("accept-ranges", "bytes");
    if (_status == WS_200_OK) {
        __identify_resource();
        if (_request.header.method == "GET")
            __respond_get();
        if (_request.header.method == "POST")
            __respond_post();
    }
    else
        __respond_to_error();
}

// for now exact same a get, but later specialize this [ + ]
void Response::__respond_to_error() {
    // __fetch_error_page_path(); ?
    __handle_type(); // TODO: map function pointers ? have a decision tree system.
    __decide_persistency();
    __response_to_string();
    // send error page
    // write this s.t. if an error occurs during this process one can
    // re-call this function and respond with new status coda
}

void Response::__respond_get() {
    // chunked request: ?
    // Transfer-Encoding: chunked ...
    __handle_type(); // TODO: map function pointers ? have a decision tree system.
    __decide_persistency();
    __response_to_string();
    // status 200
}


// PARSED HEADER:
// 	Method: POST
// 	Target: /?action=save
// 	Version: HTTP/1.1
// PARSED FIELDS:

// accept|text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
// accept-encoding|gzip, deflate, br
// accept-language|en-gb,en-us;q=0.9,en;q=0.8
// cache-control|max-age=0
// connection|keep-alive
// content-length|12
// content-type|application/x-www-form-urlencoded
// host|localhost:9999
// origin|http://localhost:9999
// referer|http://localhost:9999/
// sec-ch-ua|"google chrome";v="105", "not)a;brand";v="8", "chromium";v="105"
// sec-ch-ua-mobile|?0
// sec-ch-ua-platform|"macos"
// sec-fetch-dest|document
// sec-fetch-mode|navigate
// sec-fetch-site|same-origin
// sec-fetch-user|?1
// upgrade-insecure-requests|1
// user-agent|mozilla/5.0 (macintosh; intel mac os x 10_15_7) applewebkit/537.36 (khtml, like gecko) chrome/105.0.0.0 safari/537.36

// The data that you send in a POST request must adhere to specific formatting requirements.
// You can send only the following content types in a POST request to Media Server:

// application/x-www-form-urlencoded
// multipart/form-data

// https://httpwg.org/specs/rfc9110.html#POST
void Response::__respond_post() {
    // read content-type field
    // read content-length field
    // send 201 created
    // create location header with resource

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
    if (is_success_code(_status)) {
        root = _config.root;
        if (_request.header.target == "/")
            file = "/" + _config.index; // is it always "/" or are there other formats? [ + ]
        else
            file = _request.header.target;
    }
    else { // assuming any other thing besides 200 ok is wrong for now (rdr?)
        root = "./default_pages/errors";
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
    if (DEBUG)
        std::cout << "PATH: " << _resource.path << std::endl;
    // - - - -TARGET CHECK - - - - - 
    __validate_target();
}

void Response::__validate_target() {
    int tmp_fd;
    // check also for W in POST ?
    // [ + ] system to send error pages accordingly
    if ((tmp_fd = open(_resource.path.c_str(), O_RDONLY)) < 0) {
        if (errno == ENOENT)
            error_status(WS_404_NOT_FOUND, strerror(errno));
        else if (errno == EACCES)
            error_status(WS_403_FORBIDDEN, strerror(errno));
        else
            error_status(WS_500_INTERNAL_SERVER_ERROR, strerror(errno));
    }
    close(tmp_fd);
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
        _resource.type = _resource.extension; //    [ ? ]
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
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        error_status(WS_500_INTERNAL_SERVER_ERROR);
    }
}

// adds a string formatted as <'field name': 'value'CRLF> to the header stream buffer
void Response::__add_field(const std::string& field_name, const std::string& value) {
    _fields_stream << field_name << ": " << value << CRLF;
}

// field format example: "date: Mon, 26 Sep 2022 09:14:21 GMT"
void Response::__add_formatted_timestamp() {
    std::stringstream s;
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    s << std::put_time(now, "%a, %d %b %Y %T %Z");
    __add_field("Date", s.str());
}

void Response::__decide_persistency() {
    if (_request._is_persistent == true
        && (_request.field_is_value("connection", "keep-alive")
            || _request.field_is_value("connection", "chunked")))
        _is_persistent = true;
}

std::string Response::__generate_status_line() const {
    std::stringstream stream_status_line;
    stream_status_line << WS_HTTP_VERSION << SP << _tokens.status_phrases[_status];
    return (stream_status_line.str());
}

void Response::__response_to_string() {
    if (!_body.str().empty())
        __add_field("Content-length", std::to_string(_body.str().length()));
    std::stringstream response;
    response << __generate_status_line() << CRLF
        << _fields_stream.str() << CRLF
        << _body.str() << CRLF;
    _response_str = response.str();
}

} // NAMESPACE http
} // NAMESPACE ws
