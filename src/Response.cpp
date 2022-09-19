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
    {
        // problem to solve how the config data is reachable by all needed class?
        // _root = config data??
        // _root = "./example_sites/phptestsite";
        _root = "./example_sites/example2";
    }
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
            // std::cout << "FUCK------111\n";
        if (_file.find(".ico") != std::string::npos)
            // fields_stream << "Content-Type: image/x-icon\nCache-Control: no-cache" << CHAR_CR << CHAR_LF;
            fields_stream << "Content-Type: image/x-icon"/*  << CHAR_CR << CHAR_LF */;
        else if (_file.find(".png") != std::string::npos)
            // fields_stream << "Content-Type: image/x-icon\nCache-Control: no-cache" << CHAR_CR << CHAR_LF;
            fields_stream << "Content-Type: image/png"/*  << CHAR_CR << CHAR_LF */;
        else
        {
            // fields_stream << "Content-Type: image/png\nCache-Control: no-cache" << CHAR_CR << CHAR_LF;
            fields_stream << "Content-Type: image/jpeg"/* << CHAR_CR << CHAR_LF */;
            // fields_stream << "Content-Type: image/jpeg" << CHAR_CR << CHAR_LF;
            // std::cout << "FUCK------\n";
        }
            // fields_stream << "Content-Type: image/x-icon" << CHAR_CR << CHAR_LF;
        // fields_stream << "Transfer-Encoding: chunked" << CHAR_CR << CHAR_LF;
        // fields_stream << "Connection: keep-alive" << CHAR_CR << CHAR_LF;
    }
    // cgi php part
    // if (std::string::size_type locate = _file.find(".php") != std::string::npos && locate + 4 == _file.length())
    if (_file.find(".php") != std::string::npos)
    {
        fields_stream << "Content-Type: text/event-stream\nCache-Control: no-cache" << CHAR_CR << CHAR_LF;
    }
// -------- header composition ^^ ---------------

    try {
        if (_file.find(".php") != std::string::npos)
        {
               Cgi test;
                std::string phpresp;
                // phpresp = "HTTP/1.1 200 OK\nContent-Type: text/event-stream\nCache-Control: no-cache\n";
                // phpresp +=  test.executeCgi("./example_sites/phptestsite/send_sse.php");
                phpresp +=  test.executeCgi(path);
            std::stringstream buffer;

            buffer << _status_line << fields_stream.str() << CHAR_CR << CHAR_LF;
            std::string _response_str = buffer.str();

            buffer << phpresp << CHAR_CR << CHAR_LF; // write php data in

            _response_str = buffer.str();
            if (send(client_socket.fd, _response_str.c_str(), strlen(_response_str.c_str()), 0) < 0)
                throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
            std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
        }
        else if (_file.find(".jpg") != std::string::npos || _file.find(".png") != std::string::npos ||  _file.find(".ico") != std::string::npos)
        {
            std::ifstream fin(path, std::ios::in | std::ios::binary);
            // std::ifstream fin(path, std::ios::in);
            std::ostringstream oss;
            // std::stringstream oss;

            // std::string _response_str = oss.str();  // write before the file staff

            // oss << _status_line << fields_stream.str() << CHAR_CR << CHAR_LF;
            oss << fin.rdbuf();
            // std::cout << oss.str() << "\n";
            // 	std::ofstream outfile ("this.png");
            // outfile << oss.str();
            // outfile.close();
            std::stringstream ss;
            ss << oss.str().length();
            std::string str = ss.str();
            fields_stream << CHAR_LF << "accept-ranges: bytes\ncontent-length: " << str << CHAR_CR << CHAR_LF;
            // std::ostringstream buffer;

            // buffer << _status_line << fields_stream.str() << CHAR_CR << CHAR_LF;
            // buffer << oss << CHAR_CR << CHAR_LF;

            // _response_str = buffer.str();
            // // _response_str += _status_line + fields_stream.str() + CHAR_LF + "content-length: " + str + CHAR_CR + CHAR_LF + oss.str() +  CHAR_CR + CHAR_LF;
            // // _response_str += oss.str() + << CHAR_CR << CHAR_LF;
            // if (send(client_socket.fd, _response_str.c_str(), strlen(_response_str.c_str()), 0) < 0)
            //     throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
            // std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
            // if (send(client_socket.fd, data.c_str(), strlen(data.c_str()), 0) < 0)
            // std::string data(oss.str());

            // --------------------- clear state -------------------------
            // std::ifstream page_file(path);
            // if (page_file.fail())
            // {
            //     std::cout << "I'm not working\n";
            // }
            
            // if (!page_file.is_open()) ...
            std::ostringstream buffer;
            // std::stringstream buffer;

            // buffer << fields_stream.str() << CHAR_CR << CHAR_LF;
            buffer << _status_line << fields_stream.str() << CHAR_CR << CHAR_LF;

            std::string _response_str = buffer.str();  // write before the file staff

            buffer << oss.str() << CHAR_CR << CHAR_LF;
            // std::cout << _response_str.length() << "before ||| ---------------fuck thos---------------------------------\n";
            _response_str = buffer.str();
            // std::cout << _response_str.length() << "after      ---------------fuck thos---------------------------------\n";

            // _response_str << std::noskipws;
            // std::cout << "RESPONSE:\n" << _response_str.data() << std::endl;
            std::vector<char> temp;
            // char temp[_response_str.length()];
            // char *temp;
            // temp = (char *)malloc(_response_str.length());
            for (size_t i = 0; i < _response_str.length(); i++)
            {
                // temp[i] =_response_str[i];
                temp.push_back(_response_str[i]);
                // std::cout << _response_str[i] << "\t" << (int) _response_str[i] << std::endl;

            }

            // std::cout << temp<< std::endl;
            
            // size_t bob =  strlen(_response_str.c_str());
            // size_t bubi = _response_str.length();
            // size_t bebi = _response_str.length();
            // std::cout << "Here is bob the migthy: " << bob << " end: " << bob << std::endl;
            // std::cout << "RESPONSE:\n" << _response_str << std::endl;
            if (send(client_socket.fd,  &(*(temp.begin())) , _response_str.length(), 0) < 0)
                throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
            std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
        }
        else
        {

            std::ifstream page_file(path);
            if (page_file.fail())
            {
                std::cout << "I'm not working\n";
            }
            
            // if (!page_file.is_open()) ...
            std::stringstream buffer;

            buffer << _status_line << fields_stream.str() << CHAR_CR << CHAR_LF;

            std::string _response_str = buffer.str();  // write before the file staff

            buffer << page_file.rdbuf() << CHAR_CR << CHAR_LF;
            _response_str = buffer.str();
            // std::cout << "RESPONSE:\n" << _response_str << std::endl;
            // std::cout << "RESPONSE:\n" << _response_str << std::endl;
            if (send(client_socket.fd, _response_str.c_str(), strlen(_response_str.c_str()), 0) < 0)
                throw_status(WS_500_INTERNAL_SERVER_ERROR, "Error sending data");
            std::cout << CYAN << "Response class: Server sent data" << NC << std::endl;
        }

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

