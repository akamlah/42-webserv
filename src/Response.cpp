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

Response::Response(const Request& request, const int fd): _request(request), _status(request.status()),
    _is_persistent(request._is_persistent) {

    __set_content_type();
    
    // #if DEBUG
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
    // #endif


    __set_target_path();

    #if DEBUG
    std::cout << "RESPONSE PATH = " << _path << std::endl;
    #endif

    if (request._is_persistent == true
        // && (request.field_is_value("connection", "keep-alive")
        && (((request.fields.find("connection"))->second == "keep-alive")
        // || request.field_is_value("connection", "chunked"))) {
        || ((request.fields.find("connection"))->second == "chunked"))) {
            keep_alive = true;

            if ((request.fields.find("connection"))->second == "keep-alive")
                std::cout << "KEEP ALIVE" << std::endl;
            else
                std::cout << "not keep alive" << std::endl;
        }

    // FILEDS

    // 1. Server: ZHero-serv/1.0

    // 2. Content-type:
    std::stringstream fields_stream;
    if ((request.fields.find("sec-fetch-dest"))->second == "image") {
        // std::cout << "is image" << std::endl;
        fields_stream << "Content-Type: image/jpeg" << CRLF;
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

        buffer << __generate_status_line() << fields_stream.str() << CRLF;
        std::string _response_str = buffer.str();
        // std::cout << "RESPONSE:\n" << _response_str << std::endl;
        buffer << page_file.rdbuf() << CRLF;
        _response_str = buffer.str();

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


// - - - - - - - path - - - - - - - -

static std::string __extension(const std::string& target) {
    size_t pos = target.rfind('.');
    if (pos != target.npos)
        return (target.substr(pos + 1));
    return ("no ext"); // <- temporary!!!!!!!!!!
}

void Response::__set_content_type() {
    std::cout << __extension(_request.header.target) << std::endl;
    // set feild accordingly
}

void Response::__set_target_path() {
    std::string root;
    std::string file;

    // TEST ONLY:
    // _status = WS_501_NOT_IMPLEMENTED;

    // [ ! ] TODO
    // if any response to a already failed system call fails agai, throw exception!

    // fetch path from config struct later, for now hardcode example
    // or use the DEFAULT CONFIG -> todo
    if (status() == WS_200_OK) {
        root = "./example_sites/example2";
        if (_request.header.target == "/")
            file = "/index.html";
        else
            file = _request.header.target;
    }

    else { // assuming any other thing besides 200 ok is wrong for now (rdr?)
        root = "./default_pages/errors";
        std::stringstream stream_file;
        stream_file << "/error_" << status() << ".html";
        file = stream_file.str();
    }
    _path = root + file;
}

std::string Response::__generate_status_line() const {
    std::stringstream stream_status_line;
    stream_status_line << WS_HTTP_VERSION << SP_int << StatusPhrase()[status()] << CRLF;
    return (stream_status_line.str());
}

Response::~Response() {}

const char *Response::c_str() const {
    return (_response_str.c_str());
}

} // NAMESPACE http
} // NAMESPACE ws
