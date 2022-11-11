/* ************************************************************************ */
/*                                                                          */
/*                              Class: Response                             */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Response.hpp"

namespace ws {
namespace http {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Exceptions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const char* Response::ResponseException::what() const throw() {
    return ("Response error");
}

const char* Response::Respond_with_directory_listing::what() const throw() {
    return ("Responding with directory listing because index was not found");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Construction/destruction
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Response::Response(const Request& request, const config_data& config, const Tokens& tokens) :
    _request(request), _config(config), _tokens(tokens), _status(request.status())
{
    // std::list<std::string> templng = _request.get_field_value("content-length");
    // if (templng.empty())
    // {
    // }
    build_response();
}

Response::~Response() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// PUBLIC Member fuctions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string& Response::string() { return (_response_str); }

bool Response::status_is_success() const {
    if (_status >= 400)
        return (false);
    return (true);
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// PUBLIC non-member fuctions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Response::append_slash(std::string& path) {
    if (!path.empty())
        if (path.rfind('/') != path.length() - 1)
            path = path + "/";
}

void Response::remove_leading_slash(std::string& path) {
    if (!path.empty())
        if (path[0] == '/')
            path.erase(0, 1);
}

static void remove_trailing_slash(std::string& path) {
    if (!path.empty())
        if (path[path.length() - 1] == '/')
            path.erase(path.length() - 1, path.length());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Main fuction for response generation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Response::redirection_check()
{
    if (getValid("GET"))
        throw_error_status(WS_405_METHOD_NOT_ALLOWED, "Method forbidden by config file");
    if (_request.header.method == "POST")
        throw_error_status(WS_501_NOT_IMPLEMENTED, "Redirection for POST cureently not supported, come back later!");
    else if (_request.header.method == "DELETE")
        throw_error_status(WS_501_NOT_IMPLEMENTED, "Redirection for DELETE cureently not supported, come back later!");
    _status = WS_301_MOVED_PERMANENTLY;
    _body.str(std::string());;
    _fields_stream.str(std::string());
    _response_str = std::string();
    add_field("Server", "ZHero serv/1.0");
    add_field("Location", _config.http_redirects);
    add_formatted_timestamp();
    response_to_string();
}

void Response::build_response() {
    if (_status != WS_200_OK) {
        respond_to_error();
        return ;
    }
    add_field("Server", "ZHero serv/1.0");
    add_formatted_timestamp();
    try {
        if (_config.limit_body != 0)
        {
            int bodysize = 0;
            bodysize = _request._body.str().length();
            if (bodysize > _config.limit_body)
                throw_error_status(WS_413_PAYLOAD_TOO_LARGE, "Config setting not alowing such a big request.");
        }
        identify_resource();
        if (_config.http_redirects != "non")
            redirection_check();
        else if (_request.header.method == "GET")
            method_get();
        else if (_request.header.method == "POST")
            method_post();
        else if (_request.header.method == "DELETE")
            method_delete();
        else
            throw_error_status(WS_501_NOT_IMPLEMENTED, "Sadly this HTTP method is not implemented.");
    }
    catch (Respond_with_directory_listing& e) {
        respond_with_directory_listing_html(); // will build dir listing response
    }
    catch (ResponseException& e) {
        respond_to_error(); // will build error response
    }
    // anything else like stringstream errors etc: MEANS STATUS STILL OK and has to be set to not okay
    catch (std::exception& e) {
        if (DEBUG) { std::cout << "unforeseen exception in response: " << e.what() << std::endl; }
        _status = WS_500_INTERNAL_SERVER_ERROR;
        respond_to_error(); // will build error response
    }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Method: Delete
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Response::method_delete()
{
    if (getValid("DELETE"))
        throw_error_status(WS_405_METHOD_NOT_ALLOWED, "Method forbidden by config file");
    if (std::remove(_resource.abs_path.c_str()) == 0) {
        add_field("Content-length", "0");
        respond_to_delete();
    }
    else {
        throw_error_status(WS_404_NOT_FOUND, "The file is not there!");
    }
}

void Response::respond_to_delete() {
    _body.str(std::string());
    _fields_stream.str(std::string());
    _response_str = std::string();
    add_field("Server", "ZHero serv/1.0");
    add_formatted_timestamp();
    _body << "The file was deleted!\r\n";
    response_to_string();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Method: exceptions (errors and directory listing)
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


void Response::default_error() {
    _body.str(std::string());;
    _fields_stream.str(std::string());
    _response_str = std::string();
    add_field("Server", "ZHero serv/1.0");
    add_field("Connection", "close");
    add_formatted_timestamp();
    _body << "<!DOCTYPE html>\n<html lang=\"en\">\n"
        << "<head><title>Error " << _status << "</title></head>\n"
        << "<body body style=\"background-color:black;"
        << "font-family: 'Courier New', Courier, monospace; color:rgb(209, 209, 209)\">"
        << "<h3> Zhero serv 1.0: Error</h3>\n"
        << "<h1>" << _tokens.status_phrases[_status] << "</h1>"
        << "<h3>" << _request.error_msg << "</h3>\n"
        << "<h3>" << error_msg << "</h3>\n"
        << "</body></html>";
}

std::string Response::custom_error_check_status() {
    std::string temp;
    std::string::size_type positofstatus = _config.error.find(std::to_string(_status));
    if (positofstatus == std::string::npos)
        return (temp);
    std::string::size_type positofend = _config.error.find(",", positofstatus + 4);
    if (positofend == std::string::npos)
        positofend = _config.error.length();
    temp = _config.error.substr(positofstatus + 4, positofend - (positofstatus + 4));
    return (temp);
}

void Response::custom_error() {
    std::string path = custom_error_check_status();
    if (path.empty()) {
        default_error();
        return ;
    }
    _body.str(std::string());
    _fields_stream.str(std::string());
    _response_str = std::string();
    add_field("Server", "ZHero serv/1.0");
    add_field("Connection", "close");
    add_formatted_timestamp();
    std::string abs_path = _config.root + path;
    if (check_error_path(abs_path)) {
        default_error();
        return ;
    }
    std::ifstream fin(abs_path, std::ios::in);
    _body << fin.rdbuf();
}

bool Response::check_error_path(std::string const & path) {
    int tmp_fd;
    if ((tmp_fd = open(path.c_str(), O_RDONLY)) < 0) {
        close(tmp_fd);
        return (true);
    }
    close(tmp_fd);
    return (false);
}

void Response::respond_to_error() {
    if (_config.error == "non")
        default_error();
    else
        custom_error();
    response_to_string();
}

void Response::respond_with_directory_listing_html() {
    DIR *dir;
    struct dirent *ent;
    int i = 0;
    
    _body.str(std::string());
    _fields_stream.str(std::string());
    _response_str = std::string();
    
    std::string tmp_ent_d_name;
    std::string tmp_path = _resource.path;
    remove_leading_slash(tmp_path);
    append_slash(tmp_path);
    std::string current_directory = _resource.root + tmp_path;
    
    add_field("Server", "ZHero serv/1.0");
    add_formatted_timestamp();
    add_field("Content-type", "text/html");
    _body << "<!DOCTYPE html>\n<html lang=\"en\">\n";
    _body << "<head><title>Index</title></head>\n";
    _body << "<body>";
    _body << "<h1>Index of " << tmp_path << "<br></h1>";
    _body << "<p>";
    if ((dir = opendir(&(*(current_directory.c_str())))) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            tmp_ent_d_name = ent->d_name;
            append_slash(tmp_ent_d_name);
            if (_resource.path == "/" && i == 1)
                _body << tmp_ent_d_name << "<br>";
            else
                _body << "<a href=\"" << tmp_ent_d_name << "\">" << tmp_ent_d_name << "</a><br>";
            i++;
        }
        closedir (dir);
    }
    else
        throw_error_status(WS_404_NOT_FOUND, strerror(errno));
    _body << "</p>";
    _body << "</body>\r\n";
    response_to_string();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// further TARGET PARSING
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// identifies target path and type and adds content-type field to header
void Response::identify_resource() {
    interpret_target();
    validate_target_abs_path(); // maybe only in get ?
    extract_resource_extension();
    identify_resource_type();
}

void Response::interpret_target() {
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
    }    _resource.root = _config.root;
    
    append_slash(_resource.root);
    _resource.file = (_resource.path == "/") ? _config.index : _resource.path;
    remove_leading_slash(_resource.file);
    _resource.abs_path = _resource.root + _resource.file;

    if (DEBUG) {
        std::cout << "RESOURCE:" << std::endl;
        std::cout << "root: " << _resource.root << std::endl;
        std::cout << "file: " << _resource.file << std::endl;
        std::cout << "path: " << _resource.path << std::endl;
        std::cout << "query: " << _resource.query << std::endl;
        std::cout << "abs path: " << _resource.abs_path << std::endl;
    }
}

static bool is_directory(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

void Response::validate_target_abs_path() {
    int tmp_fd;
    std::string temp_path;
    std::string index = _config.index;
    remove_leading_slash(index);
    if (is_directory(_resource.abs_path) && _request.header.method == "GET") {
        std::cout << YELLOW << "IS DIR! Responding with dir list" << NC << std::endl;
        throw Respond_with_directory_listing();
    }
    if (!(_config.location.compare("non")) )
        temp_path = _resource.abs_path;
    else
        temp_path = _config.location + "/" + _config.index;
    if ((tmp_fd = open(temp_path.c_str(), O_RDONLY)) < 0) {
        if (errno == ENOENT) {
            if (_resource.file == index && _config.directory_listing == true) {
                // config has index file but it was not found
                std::cout << YELLOW << "Responding with dir list" << NC << std::endl;
                throw Respond_with_directory_listing();
            }
            throw_error_status(WS_404_NOT_FOUND, strerror(errno));
        }
        else if (errno == EACCES)
            throw_error_status(WS_403_FORBIDDEN, strerror(errno));
        else if (_config.directory_listing == true && errno == ENOTDIR) {
            remove_trailing_slash(_resource.abs_path);
            remove_trailing_slash(_resource.file);
            validate_target_abs_path();
        }
        else
            throw_error_status(WS_500_INTERNAL_SERVER_ERROR, strerror(errno));
    }
    close(tmp_fd);
}

void Response::extract_resource_extension() {
    size_t pos = _resource.abs_path.rfind('.');
    if (pos != _resource.abs_path.npos)
        _resource.extension = _resource.abs_path.substr(pos + 1);
    else
    _resource.extension = std::string();
}

// only if the extesion is mapped in 'tokens' content-type field is set.
// if not found type is set to extension.
void Response::identify_resource_type() {
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
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Utilities
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string Response::generate_status_line() const {
    std::stringstream stream_status_line;
    stream_status_line << WS_HTTP_VERSION << SP << _tokens.status_phrases[_status];
    return (stream_status_line.str());
}

// field format example: "date: Mon, 26 Sep 2022 09:14:21 GMT"
void Response::add_formatted_timestamp() {
    std::stringstream s;
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    s << std::put_time(now, "%a, %d %b %Y %T %Z");
    add_field("Date", s.str());
}

// adds a string formatted as <'field name': 'value'CRLF> to the header stream buffer
void Response::add_field(const std::string& field_name, const std::string& value) {
    _fields_stream << field_name << ": " << value << CRLF;
}

void Response::response_to_string() {
    std::stringstream response;
    if (!_body.str().empty())
        add_field("Content-length", std::to_string(_body.str().size()));
    response << generate_status_line() << CRLF;
    response << _fields_stream.str() << CRLF;
    response << _body.str();
    _response_str = response.str();
}

void Response::upload_file() { // + error handeling & target check here !
    if (DEBUG)
    {
        std::cout << "BUFFERING BODY FROM TARGET: " << _resource.abs_path << std::endl;
        std::cout << "redirect: " << _config.http_redirects << std::endl;
    }
    try {

        if (!(_config.location.compare("non")) )
        {
            std::ifstream fin(_resource.abs_path, std::ios::in);
            _body << fin.rdbuf();
        }
        else
        {
            std::string deside = (_resource.path == "/") ? _config.index : _resource.path;
            std::string temp = _config.location + "/" + deside;
            std::ifstream fin(temp, std::ios::in);
            _body << fin.rdbuf();
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        throw_error_status(WS_500_INTERNAL_SERVER_ERROR, strerror(errno));
    }
}

bool Response::getValid(const std::string & nameof)
{
    std::vector<std::string> temp = _config.http_methods;
    std::vector<std::string>::iterator it = temp.begin();
    std::vector<std::string>::iterator eit = temp.end();
    while (it != eit)
    {
        if (!((*it).compare(nameof)))
            return (false);
        it++;
    }
    return (true);
}

} // NAMESPACE http
} // NAMESPACE ws
