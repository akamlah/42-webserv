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

Response::~Response() {}

bool Response::is_persistent() const { return (_is_persistent); }

void Response::send(const int fd) { // more error handeling here too [ + ]
    if (DEBUG)
        std::cout << "SENDING RESPONSE:\n" << _response_str;
    if (::send(fd, _response_str.c_str(), _response_str.length(), 0) < 0)
        throw_error_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
    std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
}

int Response::throw_error_status(int status, const char* msg) {
    error_msg = msg;
    if (DEBUG) {
        if (msg)
            std::cout << RED << msg << ": " << NC;
        std::cout << RED << "Error: " << _tokens.status_phrases[status] << NC << std::endl;
    }
    _status = status;
    throw Response::ResponseException();
    return (status);
}

void Response::append_slash(std::string& path) {
    if (!path.empty())
        if (path.rfind('/') != path.npos)
            path = path + "/";
}

void Response::remove_leading_slash(std::string& path) {
    if (!path.empty())
        if (path[0] == '/')
            path.erase(0, 1);
}

// - - - - - - - - - - - PRIVATE - - - - - - - - - - - - - 

// static bool is_success_code(const int n) {
//     if (n >= 200 && n < 300)
//         return (true);
//     return (false);
// }

std::string Response::__generate_status_line() const {
    std::stringstream stream_status_line;
    stream_status_line << WS_HTTP_VERSION << SP << _tokens.status_phrases[_status];
    return (stream_status_line.str());
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

// main blocks - - - - - - - - -

void Response::__response_to_string() {
    std::stringstream response;
    if (!_body.str().empty())
        __add_field("Content-length", std::to_string(_body.str().length()));
    response << __generate_status_line() << CRLF;
    response << _fields_stream.str() << CRLF;
    response << _body.str() << CRLF;
    _response_str = response.str();
}

void Response::__build_response() {
    if (_status != WS_200_OK) {
        __respond_to_error();
        return ;
    }
    __add_field("Server", "ZHero serv/1.0");
    __add_formatted_timestamp();
    try {
        __identify_resource();
        // map function pointers to avoid if else statements [ ? ]
        if (_request.header.method == "GET")
            __respond_get();
        if (_request.header.method == "POST")
            __respond_post();
    }
    catch (ResponseException& e) {
        __respond_to_error(); // will build error response
    }
    // anything else like stringstream errors etc: MEANS STATUS STILL OK and has to be set to not okay
    catch (std::exception& e) {
        if (DEBUG) { std::cout << "unforeseen exception in response: " << e.what() << std::endl; }
        _status = WS_500_INTERNAL_SERVER_ERROR;
        __respond_to_error(); // will build error response
    }
}

// - - - - - - METHODS - - - - - - - -

// implement custom error pages fetching
//     root = "./default_pages/errors";
// assuming any other thing besides 200 ok is wrong for now (rdr?)
void Response::__respond_to_error() {
    _body.str(std::string());;
    _fields_stream.str(std::string());
    _response_str = std::string();
    __add_field("Server", "ZHero serv/1.0");
    __add_formatted_timestamp();
    __decide_persistency();
    _body << "<!DOCTYPE html>\n<html lang=\"en\">\n"
        << "<head><title>Error " << _status << "</title></head>\n"
        << "<body body style=\"background-color:black;"
        << "font-family: 'Courier New', Courier, monospace; color:rgb(209, 209, 209)\">"
        << "<h3> Zhero serv 1.0: Error</h3>\n"
        << "<h1>" << _tokens.status_phrases[_status] << "</h1>"
        << "<h3>" << _request.error_msg << "</h3>\n"
        << "<h3>" << error_msg << "</h3>\n"
        << "</body>\r\n";
    __response_to_string();
}

void Response::__respond_get() {
    __add_field("accept-ranges", "bytes");
    // chunked request: ?
    // Transfer-Encoding: chunked ...
    __handle_type(); // TODO: map function pointers ? have a decision tree system.
    __decide_persistency();
    __response_to_string();
    // status 200
}

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
    
    // for now:
    __handle_type(); // TODO: map function pointers ? have a decision tree system.
    __decide_persistency();
    __response_to_string();

}

// - - - - - Subfunctions - - - - 

// identifies target path and type and adds content-type field to header
void Response::__identify_resource() {
    __interpret_target();
    __validate_target_abs_path(); // maybe only in get ?
    __extract_resource_extension();
    __identify_resource_type();
}

// separate uri components, decoding done in request parser
// -> root always ends in '/' and file never starts with '/'
void Response::__interpret_target() {
    std::string uri = _request.header.target;
    try {
        size_t uri_end = uri.npos;
        size_t query_pos = uri.find('?');
        _resource.path = uri.substr(0, query_pos);
        if (query_pos != uri_end)
            _resource.query = uri.substr(query_pos + 1);
    }
    catch (std::exception& e) {
        throw_error_status(WS_500_INTERNAL_SERVER_ERROR, "Uri could not be parsed, format error");
    }
    if (DEBUG) {
        std::cout << "Separated URI components:" << std::endl;
        std::cout << "path: " << _resource.path << std::endl;
        std::cout << "query: " << _resource.query << std::endl;
    }
    _resource.root = _config.root; // always ?
    _resource.file = (_resource.path == "/") ? _config.index : _resource.path;
    append_slash(_resource.root);
    remove_leading_slash(_resource.file);
    _resource.abs_path = _resource.root + _resource.file;
    if (DEBUG)
        std::cout << "PATH: " << _resource.abs_path << std::endl;
}

void Response::__validate_target_abs_path() {
    int tmp_fd;
    // check also for W in POST ?
    // [ + ] system to send error pages accordingly
    if ((tmp_fd = open(_resource.abs_path.c_str(), O_RDONLY)) < 0) {
        if (errno == ENOENT)
            throw_error_status(WS_404_NOT_FOUND, strerror(errno));
        else if (errno == EACCES)
            throw_error_status(WS_403_FORBIDDEN, strerror(errno));
        else
            throw_error_status(WS_500_INTERNAL_SERVER_ERROR, strerror(errno));
    }
    close(tmp_fd);
}

void Response::__extract_resource_extension() {
    size_t pos = _resource.abs_path.rfind('.');
    if (pos != _resource.abs_path.npos)
        _resource.extension = _resource.abs_path.substr(pos + 1);
    else
    _resource.extension = std::string();
}

// only if the extesion is mapped in 'tokens' content-type field is set.
// if not found type is set to extension.
void Response::__identify_resource_type() {
    std::map<std::string, std::string>::const_iterator it \
        = _tokens.extensions.typemap.find(_resource.extension);
    if (it == _tokens.extensions.typemap.end()) {
        _resource.type = _resource.extension; //    [ ? ]
        return ;
    }
    size_t separator_pos = it->second.find('/');
    _resource.type = it->second.substr(0, separator_pos);
    if (separator_pos != it->second.npos)
        _resource.subtype = it->second.substr(separator_pos + 1);
    __add_field("Content-type", _resource.subtype.empty() ? _resource.type
        : (_resource.type + "/" + _resource.subtype));
}

// temporarily handles every type, later have a decision tree or similar
void Response::__handle_type() {
    // if format = x form url encoded [ + ] - else ignore query ?
    if (!_resource.query.empty()) {
        CgiEnv_FormUrlencoded cgi_env(_resource.query);
        if (DEBUG)  {
            std::cout << YELLOW << "ENV for cgi:" << NC << std::endl;
            for (int i = 0; cgi_env.env[i] != NULL; i++) {
                std::cout << YELLOW << i << "    " << cgi_env.env[i] << NC << std::endl;
            }
        }
    }
    if (_resource.extension == "php") { // and cgi in general -> ADD THE OTHER CASES    [ + ]
        __add_field("Cache-Control", "no-cache");
        // CALL CGI HERE - - - -- - - - - 
        	Cgi test;
            std::string phpresp;
            phpresp +=  test.executeCgi(_resource.abs_path);
            _body << phpresp;
            return ;
    }
    // if (_resource.extension == "html" && _config.isCgiOn) {
    //     	Cgi test;
    //         // std::string phpresp;
    //         // phpresp +=  test.executeCgi(_resource.abs_path);
    //         test.readHTML(_resource.abs_path);
    //         _resource.abs_path = "./response.html"; //tmp
    //         // _body << phpresp;
    //         return ;
    // }
    else
    // are there other cases ?
    __upload_file();
}

void Response::__upload_file() { // + error handeling & target check here !
    if (DEBUG)
        std::cout << "BUFFERING BODY FROM TARGET: " << _resource.abs_path << std::endl;
    try {
        std::ifstream fin(_resource.abs_path, std::ios::in);
        // if (!page_file.is_open()) ...
        _body << fin.rdbuf();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        throw_error_status(WS_500_INTERNAL_SERVER_ERROR, strerror(errno));
    }
}

// __________________________________________________________________________________________________________
// 
// CGI ENV builder
// __________________________________________________________________________________________________________

// - - - - - -  constr/destr - - - - - - - - 

// delimiter = '&', placeholders %HH, split, replace and copy to char** ready for execve() call in cgi
CgiEnv_FormUrlencoded::CgiEnv_FormUrlencoded(const std::string& query_str) {
    _nb_tokens = 0;
    std::string token;
    size_t i = 0;
    size_t delimiter_pos = 0;
    size_t prev_delimiter_pos = 0;
    try {
        while ((delimiter_pos = query_str.find('&', prev_delimiter_pos)) != std::string::npos)
            { prev_delimiter_pos = delimiter_pos + 1; i++; }
        env = new char*[i + 2];
        i = 0;
        delimiter_pos = 0;
        prev_delimiter_pos = 0;
        while (i < query_str.size()) {
            delimiter_pos = query_str.find('&', prev_delimiter_pos);
            token = query_str.substr(prev_delimiter_pos, delimiter_pos - prev_delimiter_pos);
            env[i] = new char[token.size() + 2];
            _nb_tokens++;
            strlcpy(env[i], token.c_str(), token.size() + 1);
            if (delimiter_pos == std::string::npos)
                break ;
            prev_delimiter_pos = delimiter_pos + 1;
            i++;
        }
        env[_nb_tokens] = NULL;
    }
    catch (std::exception& e) {
        __delete_env();
        throw e; // will result in 500 in Request::__interpret_target catch block
    }
}

CgiEnv_FormUrlencoded::CgiEnv_FormUrlencoded(const CgiEnv_FormUrlencoded& other) {
    __copy_env(other);
}

CgiEnv_FormUrlencoded& CgiEnv_FormUrlencoded::operator=(const CgiEnv_FormUrlencoded& other) {
    __delete_env();
    __copy_env(other);
    return (*this);
}

CgiEnv_FormUrlencoded::~CgiEnv_FormUrlencoded() {
    __delete_env();
}

// - - - - - -  subfunctions - - - - - - - - 

void CgiEnv_FormUrlencoded::__copy_env(const CgiEnv_FormUrlencoded& other) {
    _nb_tokens = other._nb_tokens;
    env = new char*[_nb_tokens + 1];
    for (int i = 0; i < _nb_tokens; i++) {
        env[i] = new char[strlen(other.env[i]) + 2];
        strlcpy(env[i], other.env[i], strlen(other.env[i]) + 1);
    }
    env[_nb_tokens] = NULL;
}

void CgiEnv_FormUrlencoded::__delete_env() {
    int i = 0;
    while (i < _nb_tokens) {
        delete env[i];
        i++;
    }
    delete[] env;
}

} // NAMESPACE http
} // NAMESPACE ws
