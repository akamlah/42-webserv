/*                                                                          */
/* ************************************************************************ */
/*                              Class: Request                              */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Request.hpp"
#include <fstream>

namespace ws {
namespace http {

const char* Request::BadRead::what() const throw() {
    return ("Bad read!");
}
const char* Request::EofReached::what() const throw() {
    return ("EOF reached!");
}

// Request::Request(const Socket& client): client_socket(client), keep_alive(true) { }
// Request::Request(const int fd): fd(fd), keep_alive(true) { }
Request::Request(const Connection& c): _connection(c) { }

Request::Request(const Request& other): keep_alive(true) {
    header.method = other.header.method;
    header.target = other.header.target;
    header.version = other.header.version;
    fields = other.fields;
}

Request& Request::operator=(const Request& other) {
    header.method = other.header.method;
    header.target = other.header.target;
    header.version = other.header.version;
    fields = other.fields;
    return (*this);
}

bool Request::field_is_value(const char* field_name, const char* value) const {
    if ((fields.find(field_name))->second == value)
        return (true);
    return (false);
}

void Request::parse() {
    std::cout << "about to parse a new request on fd: " << fd << std::endl;

    parser.parse(*this);
    _connection._status
    if (status.get_current() != WS_200_OK)... should be handeled by response anyways

    // #if DEBUG
    std::cout << RED << "PARSED REQUEST STATUS: " << this->status() << NC << std::endl;
    std::cout << CYAN << "PARSED HEADER:\n" \
    << "\tMethod: " << header.method << "\n" \
    << "\tTarget: " << header.target << "\n" \
    << "\tVersion: " << header.version << NC << std::endl;
    std::cout << CYAN << "PARSED FIELDS:\n";
    for (std::map<std::string, std::string>::iterator it = fields.begin(); it != fields.end(); it++) {
        std::cout << it->first << "|" << it->second << std::endl;
    }
    std::cout << NC << std::endl;
    // #endif
}

Request::~Request() { /* free data ?*/ }

// --------------------------------------------------------------------------------------------------------
// HTTP REQUEST PARSER
// --------------------------------------------------------------------------------------------------------

Request::parser::parser(): line_length(0), nb_lines(0), msg_length(0), nb_empty_lines_beginning(0),
    word_length(0), host_fields(0), word_count(0), start_content(false), request_line_done(false),
    header_done(false)
{
    bzero(buffer, BUFFER_SIZE);
    bzero(request_line, REQUEST_LINE_LENGTH);
    bzero(word, 10000);
}

Request::parser::~parser() {}

int Request::parser::error_status(Request& request, const int status, const char* msg) const {
    // #if DEBUG
    if (msg)
        std::cout << RED << msg << ": " << NC;
    std::cout << RED << "Error: " << Status()[status] << NC << std::endl; // temporary
    // #endif
    request.error_msg = msg;
    request._status = status;
    request.keep_alive = false;
    return (status);
}

// implemented methods: -> CENTRALISE later!!
const char Request::methods[4][10] = {"GET", "HEAD", "POST", "DELETE"};

bool Request::parser::__is_method(const char *word, size_t word_length) const {
    int i = 0;
    while (i < 4) {
        if (!strncmp(methods[i], word, word_length) && word_length == strlen(methods[i]))
            return true;
        i++;
    }
    return false;
}

// goes through byte by byte and at every newline reads the previous line into the data structure.
int Request::parser::parse(Request& request, int fd) {
    int status = WS_200_OK;
    while (msg_length < BUFFER_SIZE) {
        if (!(status = __get_byte(request, fd)))
            break ;
        if (buffer[msg_length] == '\0') {
            request._status = 0;
            throw EofReached();
        }
        // std::cout << CYAN << "|" << buffer[msg_length] << "|" << NC;
        if (status != WS_200_OK)
            return (status) ; // if 0 it is end of file
        ++msg_length;
        ++line_length;
        if (buffer[msg_length - 1] == LF_int) { // if newline found:
            ++nb_lines;
            // do strlcpy here ?
            if (!(status = __parse_previous_line(request, (char *)buffer + msg_length - line_length))) {
                std::cout << "final CRLF 2" << std::endl;
                break ;
            }
            if (status != WS_200_OK)
                return (status);
        }
    }
    // check that minimum was provided
    if (!start_content)
        return (error_status(request, WS_400_BAD_REQUEST, "Empty request header"));
    if (!request_line_done)
        return (error_status(request, WS_400_BAD_REQUEST, "No request line provided"));
    if (!host_fields)
        return (error_status(request, WS_400_BAD_REQUEST, "No host field provided"));
    buffer[msg_length] = '\0';
            std::cout << msg_length << std::endl;
    // std::cout << CYAN << "PARSER: Message recieved: ---------\n\n" << NC << buffer;
    // std::cout << CYAN << "-----------------------------------\n" << NC << std::endl;
    std::cout << "Going on" << std::endl;
    return (status);
}

// reads a byte and does some primary checks
// encoding must be a superset of US-ASCII [USASCII] -> max 128 (hex 80) (RFC 9112)
// ANY CR_int not folowed by LF_int is invalid and message is rejected (RFC 9112)
int Request::parser::__get_byte(Request& request, int fd) {
    size_t bytes_read = 0;
    if ((bytes_read = recv(fd, buffer + msg_length, 1, MSG_DONTWAIT)) < 0)
        return (error_status(request, WS_500_INTERNAL_SERVER_ERROR, "Error recieving data"));
    if (!bytes_read)
        return (0);
    if (buffer[msg_length] >= 0x80)
        return (error_status(request, WS_400_BAD_REQUEST, "Bad encoding"));
    if (line_length > 1 && buffer[msg_length - 1] == CR_int && buffer[msg_length] != LF_int)
        return (error_status(request, WS_400_BAD_REQUEST, "CR not followed by LF"));
    return (WS_200_OK);
}

// chacks format and interprets line given
// SKIP initial CRLF lines if nothing before it
// Detect end of header with final CRLF
// if not a CRLF line or a CRLF line that is neither at the end nor at the beginning of the request
// if here in the middle ANY whitespace at line start is found -> reject
int Request::parser::__parse_previous_line(Request& request, const char* line) {
    int status = WS_200_OK;
    if (!start_content && (line_length == 2 && line[line_length - 2] == CR_int)) {
        // std::cout << "CRLF" << std::endl;
        ++nb_empty_lines_beginning;
    }
    else if (header_done && (line_length == 2 && buffer[msg_length - 2] == CR_int)) { // final CRLF
        std::cout << "final CRLF" << std::endl;
        // from here later start parsing mesasge BODY if any, else stop.
        return (0);
    }
    else {
        start_content = true;
        if (isspace(line[0]))
            return (error_status(request, WS_400_BAD_REQUEST, "Whitespace at line begin"));
        if (!request_line_done) {
            if ((status = __parse_request_line(request, line)) != WS_200_OK)
                return (status);
            request_line_done = true;
        }
        else
            __parse_field_line(request, line);
        header_done = true;
    }
    line_length = 0;
    return (WS_200_OK);
}

// if line not terminated by CRLF -> message rejected
// use a copy to null terminate line without modifyig whole buffer
// exclude any whitespace except SP and final CRLF
// if space after a space -> invalid format
//  A server that receives a method longer than any that it implements SHOULD 
//  respond with a 501 (Not Implemented) status code -> BUFFER SIZES
int Request::parser::__parse_request_line(Request& request, const char* line) {
    size_t i = 0;
    int skip = 0;
    int status = WS_200_OK;

    if (line_length > REQUEST_LINE_LENGTH)
        return (error_status(request, WS_501_NOT_IMPLEMENTED, "Header too long"));
    strlcpy(request_line, line, line_length + 1);
    if (!(request_line[line_length - 2] == CR_int && request_line[line_length - 1] == LF_int))
        return (error_status(request, WS_400_BAD_REQUEST, "No CRLF at end of line"));
    while (i < line_length) {
        if ( i < line_length - 2 && isspace(request_line[i]) && request_line[i] != SP_int)
            return (error_status(request, WS_400_BAD_REQUEST, "Bad spacing"));
        if (request_line[i] == SP_int || request_line[i] == CR_int) { // end of a word
            if (i + 1 < line_length && request_line[i + 1] == SP_int)
                return (error_status(request, WS_400_BAD_REQUEST, "Consecutive spaces in start line"));
            if ((status = __parse_next_word_request_line(request, i, skip) != WS_200_OK))
                return (status);
            word_length = 0;
            skip = 1;
        }
        word_length++;
        i++;
        if (request_line[i] == LF_int) {
           if (word_count < 3)
                return (error_status(request, WS_400_BAD_REQUEST, "Too few words in start line"));
            break ;
        }
    }
    return (WS_200_OK);
}

//  An origin server that receives a request method that is unrecognized or not implemented
// SHOULD respond with the 501 (Not Implemented) status code. 
// recognized and implemented, but not allowed for the target resource,
// SHOULD respond with the 405 (Method Not Allowed) status code.
// taregt size > TARGET_SIZE -> 414 (URI Too Long)
int Request::parser::__parse_next_word_request_line(Request& request, int i, int skip) {
    ++word_count;
    if (word_count > 3)
        return (error_status(request, WS_400_BAD_REQUEST, "Too many words in start line"));
    strlcpy(word, request_line + i - word_length + skip, word_length + 1 - skip); // null terminated copy of word
    if (word_count == 1) {
        if (!__is_method(word, word_length))
            return (error_status(request, WS_501_NOT_IMPLEMENTED, "Unknown method"));
        request.header.method = word;
    }
    if (word_count == 2) {
        request.header.target = word; // see later if it is valid
    }
    if (word_count == 3) {
        if ((strlen(word) != strlen(WS_HTTP_VERSION) || strncmp(WS_HTTP_VERSION, word, strlen(WS_HTTP_VERSION))))
            return (error_status(request, WS_400_BAD_REQUEST, "Bad version name"));
        request.header.version = word;
    }
    return (WS_200_OK);
}

static void str_tolower(char * str) {
    int i = 0;
    while(str[i]) {
        str[i] = tolower(str[i]);
        i++;
    }
}

// A server MUST respond with a 400 (Bad Request) status code to any HTTP/1.1 request message
// that lacks a Host header field and to any request message that contains more than one Host
// header field line or a Host header field with an invalid field value.
// TODO:
//  If the target URI includes an authority component, then a client MUST send a
// field value for Host that is identical to that authority component
int Request::parser::__parse_field_line(Request& request, const char* line) {
    char field_line_tmp[5000];
    bzero(field_line_tmp, 5000);
    strlcpy(field_line_tmp, line, line_length + 1);
    int i = 0;
    char name[5000];
    bzero(name, 5000);
    char value[5000];
    bzero(value, 5000); // [ ! ] TODO: clean up this buffer hell
    while (field_line_tmp[i]) {
        if (field_line_tmp[i] == ':') {
            strlcpy(name, field_line_tmp, i + 1);
            if (!i || (i && name[i - 1] == ' '))
                return (error_status(request, WS_400_BAD_REQUEST, "Space before ':'"));
            str_tolower(name);
            if (!strcmp("host", name))
                ++host_fields;
            if (host_fields > 1)
                return (error_status(request, WS_400_BAD_REQUEST, "Too many host fields"));
            i++;
            if (field_line_tmp[i] != ' ')
                return (error_status(request, WS_400_BAD_REQUEST, "Missing space after ':'"));
            while (field_line_tmp[i] == ' ')
                i++;
            strlcpy(value, field_line_tmp + i, strlen(&field_line_tmp[0] + i) - 1);
            str_tolower(value);
            request.fields.insert(std::pair<std::string, std::string>(name, value));
            return (WS_200_OK);
        }
        i++;
    }
    return (error_status(request, WS_400_BAD_REQUEST, "Missing ':' in field line"));
}

} // NAMESPACE http
} // NAMESPACE ws


// GET /favicon.ico HTTP/1.1
// Host: localhost:8001
// Connection: keep-alive
// sec-ch-ua: "Google Chrome";v="105", "Not)A;Brand";v="8", "Chromium";v="105"
// sec-ch-ua-mobile: ?0
// User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36
// sec-ch-ua-platform: "macOS"
// Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
// Sec-Fetch-Site: same-origin
// Sec-Fetch-Mode: no-cors
// Sec-Fetch-Dest: image
// Referer: http://localhost:8001/
// Accept-Encoding: gzip, deflate, br
// Accept-Language: en-GB,en-US;q=0.9,en;q=0.8
