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
    std::cout << RED << "Error: " << Status()[status] << NC << std::endl;
    #endif
    (void)status;
    (void)msg;
    throw Response::ResponseException();
}

Response::Response(const Request& request): _status(request.status()), client_socket(request.get_client()) {
    #if DEBUG
    // std::cout << RED << "rsp: PARSED REQUEST STATUS: " << request.status() << NC << std::endl;
    // std::cout << CYAN << "rsp: PARSED HEADER:\n" \
    // << "\tMethod: " << request.header.method << "\n" \
    // << "\tTarget: " << request.header.target << "\n" \
    // << "\tVersion: " << request.header.version << NC << std::endl;
    // std::cout << CYAN << "rsp: PARSED FIELDS:\n";
    // for (std::map<std::string, std::string>::iterator it = request.fields.begin(); it != request.fields.end(); it++) {
    //     std::cout << it->first << "|" << it->second << std::endl;
    // }
    // std::cout << NC << std::endl;
    #endif

    std::stringstream stream_status_line;
    stream_status_line << WS_HTTP_VERSION << CHAR_SP << Status()[status()] << CHAR_CR << CHAR_LF;
    _status_line = stream_status_line.str();

    // TEST ONLY:
    // _status = WS_501_NOT_IMPLEMENTED;

    // fetch path from config struct later, for now hardcode example
    // or use the DEFAULT CONFIG -> todo
    if (status() == WS_200_OK)
        _root = "./example_sites/example2";
    else
        _root = "./default_pages/errors";

    if (request.header.target == "/") {
        if (status() == WS_200_OK)
            _file = "/index.html"; // later: index field of config file
        else {
            std::stringstream stream_file;
            stream_file << "/error_" << status() << ".html";
            _file = stream_file.str();
        }
    }
    else
        _file = request.header.target;

    std::string path = _root + _file;

    #if DEBUG
    std::cout << "RESPONSE PATH = " << path << std::endl;
    #endif

    std::stringstream fields_stream;
    if ((request.fields.find("sec-fetch-dest"))->second == "image") {
        // std::cout << "is image" << std::endl;
        fields_stream << "Content-Type: image/jpeg" << CHAR_CR << CHAR_LF;
        // fields_stream << "Transfer-Encoding: chunked" << CHAR_CR << CHAR_LF;
        // fields_stream << "Connection: keep-alive" << CHAR_CR << CHAR_LF;
    }
// -------- header composition ^^ ---------------

    try {
        std::ifstream page_file(path);
        // if (!page_file.is_open()) ...
        std::stringstream buffer;

        buffer << _status_line << fields_stream.str() << CHAR_CR << CHAR_LF;
        std::string _response_str = buffer.str();
        // std::cout << "RESPONSE:\n" << _response_str << std::endl;
        buffer << page_file.rdbuf() << CHAR_CR << CHAR_LF;
        _response_str = buffer.str();
        if (send(client_socket.fd, _response_str.c_str(), strlen(_response_str.c_str()), 0) < 0)
            throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
        std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        throw_status(WS_500_INTERNAL_SERVER_ERROR);
    }
}

Response::~Response() {}

const char *Response::c_str() const {
    return (_response_str.c_str());
}

} // NAMESPACE http
} // NAMESPACE ws

