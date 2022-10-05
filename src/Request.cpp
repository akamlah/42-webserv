/*                                                                          */
/* ************************************************************************ */
/*                              Class: Request                              */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Request.hpp"
#include <fstream>

namespace ws {
namespace http {

// --------------------------------------------------------------------------------------------------------

// const char* Request::EofReached::what() const throw() { // can be removed.
//     return ("EOF reached!");
// }

// --------------------------------------------------------------------------------------------------------

Request::Request(): _is_persistent(true), _status(WS_200_OK),
    _is_chunked(false), _waiting_for_chunks(false) { }

Request::Request(const Request& other): _is_persistent(true), _status(WS_200_OK),
    _is_chunked(false), _waiting_for_chunks(false) {
    header.method = other.header.method;
    header.target = other.header.target;
    header.version = other.header.version;
    fields = other.fields;
    _status = other._status;
}

Request& Request::operator=(const Request& other) {
    header.method = other.header.method;
    header.target = other.header.target;
    header.version = other.header.version;
    fields = other.fields;
    _status = other._status;
    _is_chunked = other._is_chunked;
    _waiting_for_chunks = other._waiting_for_chunks;
    return (*this);
}

Request::~Request() { }

// --------------------------------------------------------------------------------------------------------

bool Request::field_is_value(const std::string& field_name, const std::string& value) const {
    std::map<std::string, std::list<std::string> >::const_iterator ite = _fields._map.find(field_name);
    if (ite != _fields._map.end()) {
        for (std::list<std::string>::const_iterator lite = ite->second.begin(); lite != ite->second.end(); lite++) {
            if (*lite == value)
                return (true);
        }
    }
    return (false);
}

bool Request::has_field_of_name(const std::string& field_name) const {
    std::map<std::string, std::list<std::string> >::const_iterator ite = _fields._map.find(field_name);
    if (ite != _fields._map.end())
                return (true);
    return (false);
}

std::list<std::string> Request::get_field_value(const std::string& field_name) const {
    return (_fields._map.find(field_name)->second);
}

int Request::status() const { return (_status); }

bool Request::is_persistent() const { return (_is_persistent);}

int Request::parse(const int fd) {
    if (!_waiting_for_chunks) {
        _parser.parse(*this, fd);
        if (DEBUG) {
            std::cout << "\n\n\n" << std::endl;
            std::cout << RED << "PARSED REQUEST STATUS: " << _status << NC << std::endl;
            std::cout << CYAN << "PARSED HEADER:\n" \
            << "\tMethod: " << header.method << "\n" \
            << "\tTarget: " << header.target << "\n" \
            << "\tVersion: " << header.version << NC << std::endl;
            std::cout << CYAN << "PARSED FIELDS:\n" << NC;
            for (std::map<std::string, std::list<std::string> >::const_iterator it = _fields._map.begin(); it != _fields._map.end(); it++) {
                std::cout << CYAN << it->first<< NC << "|" ;
                for (std::list<std::string>::const_iterator itl = it->second.begin(); itl != it->second.end(); itl++)
                    std::cout << YELLOW << *itl << NC << "|";
                std::cout << std::endl;
            }
        }
        #if (DEBUG)
            std::cout << "request msg length after parse: " << _parser.msg_length << std::endl;
            std::cout << CYAN << "\nPARSER: Message recieved: ---------\n" << NC << _parser.buffer << std::endl;
            std::cout << CYAN << "-----------------------------------\n" << NC << std::endl;
            std::cout << CYAN << "\nBODY IS:---------------------------\n" << this->_body.str() << NC << std::endl;
            std::cout << CYAN << "-----------------------------------\n" << NC << std::endl;
            std::cout << GREEN << "Is persistent: ";
            std::cout << this->_is_persistent << NC << std::endl;
            std::cout << "\n\n\n" << std::endl;
        #endif
    }
    else {
        _parser.parse_chunks(*this, fd);
    }
    return (_status);
}

// --------------------------------------------------------------------------------------------------------

static void str_tolower(std::string& line) {
    for (std::string::iterator it = line.begin(); it != line.end(); it++)
        *it = tolower(*it);
}

// --------------------------------------------------------------------------------------------------------

// PUT THIS BACK TO REQUEST ?

// find '%', replace patterns [(%hh or)?] %HH with corresponding ascii character
// TEST: http://localhost:9999/data/mytext.txt?ab c&d ef&hij&k  lm&nop&qrs&tuv &wxy%hh
// errors:
// 1 -> "bad request" if end of uri: '%x', '%' or '%xx', where 'xx' != hexadecimal number
// 2 -> "bad request" if found in uri: '%xx', where 'xx' != hexadecimal number
bool Request::replace_placeholders(std::string& token) {
    std::string character;
    size_t i = 0;
    size_t delimiter_pos = 0;
    size_t prev_delimiter_pos = 0;
    while ((delimiter_pos = token.find('%', prev_delimiter_pos)) != std::string::npos) {
        if (delimiter_pos > token.length() - 3) {
            token.erase(delimiter_pos, std::string::npos);
            return (false); // 1 : will lead to a 400
        }
        character = token.substr(delimiter_pos + 1, 2);
        try {
            std::string repl;
            repl += (char)std::stoi(character, nullptr, 16);
            token = token.replace(delimiter_pos, 3, repl);
            i++;
        }
        catch (std::exception& e) {
            return (false); // 2
        }
    }
    return (true);
}

// --------------------------------------------------------------------------------------------------------
// HTTP REQUEST PARSER
// --------------------------------------------------------------------------------------------------------

parser::parser(): line_length(0), nb_lines(0), msg_length(0), nb_empty_lines_beginning(0),
    word_length(0), host_fields(0), word_count(0), start_content(false), request_line_done(false),
    header_done(false), start_fields(false), body_done(false)
{
    bzero(buffer, BUFFER_SIZE);
    bzero(request_line, REQUEST_LINE_LENGTH);
    bzero(word, 10000);
}

parser::~parser() {}

int parser::error_status(Request& request, const int status, const char* msg) const {
    #if DEBUG
    if (msg)
        std::cout << RED << msg << ": " << NC;
    std::cout << RED << "Error: " << StatusPhrase()[status] \
        << NC << std::endl; // temporary
    #endif

    request.error_msg = msg;
    request._status = status;
    request._is_persistent = false;
    return (status);
}

// implemented methods: -> CENTRALISE later!!
const char Request::methods[4][10] = {"GET", "HEAD", "POST", "DELETE"};

bool parser::__is_method(const char *word, size_t word_length) const {
    int i = 0;
    while (i < 4) {
        if (!strncmp(Request::methods[i], word, word_length) && word_length == strlen(Request::methods[i]))
            return true;
        i++;
    }
    return false;
}

// PARSE CHUNKS ---------------------------------------------------------------------------------

int parser::parse_chunks(Request& request, int fd) {
    // std::cout << YELLOW << "parsing chunks! :D" << NC << std::endl;
    int status = request._status;
    while (msg_length < BUFFER_SIZE) {
        if (!(status = __get_byte(request, fd)))
            break ;
        if (buffer[msg_length] == '\0') {
            std::cout << "EOF CHUNKS" << std::endl;
            break ;
        }
        if (status != WS_200_OK)
            return (status) ; // if 0 it is end of file
        msg_length++;
        ++line_length;
        if (buffer[msg_length - 1] == LF_int) {
            if (line_length > 1 && buffer[msg_length - 2] != CR_int)
                return(error_status(request, WS_400_BAD_REQUEST, "no CR before LF"));
            if (line_length == 2 && ((char *)buffer + msg_length - line_length)[line_length - 2] == CR_int) // [ ! ]
                std::cout << "empty line?" << std::endl;
            
        }
    }
    return (status);
}

// PARSE FIRST (header & body if not chunked) ---------------------------------------------------------------------

// [ ! ] [ + ]
// The presence of a message body in a request is signaled by a Content-Length 
// or Transfer-Encoding header field. Request message framing is independent of method semantics.
// A server MAY reject a request that contains both Content-Length and Transfer-Encoding
//  or process such a request in accordance with the Transfer-Encoding alone.
// For messages that do not include content, the Content-Length indicates the 
// size of the selected representation (Section 8.6 of [HTTP]).
int parser::__parse_body(Request& request, int fd) {
    size_t body_length = 0;
    int status = request._status;
    // A server MAY reject a request that contains both Content-Length and Transfer-Encoding
    if (request.has_field_of_name("content-length")) {
        if (request.has_field_of_name("transfer-encoding")) // [ ? ]
            return(error_status(request, WS_400_BAD_REQUEST, "Non compatible content-length and transfer-encoding fields detected"));
        request._content_length = std::atoi(request.get_field_value("content-length").begin()->c_str());
    }
    if (!request.has_field_of_name("content-length")) {
        if (!request.has_field_of_name("transfer-encoding")) // [ ? ]
            return(error_status(request, WS_400_BAD_REQUEST, "Message content detected but no content-length or transfer-encoding fields provided"));
        else {
            // check TE
            // request._waiting_for_chunks = true;
            // and catch that in parser.parse() [ ! ]
            return (status);
        }
    }
    while (body_length <= request._content_length && msg_length < BUFFER_SIZE) {
        if (!(status =  __get_byte(request, fd))) {
            std::cout << "get byte returned 0" << std::endl;
            break ;
        }
        // if (buffer[msg_length] != '\0') break ; ?
        request._body << buffer[msg_length]; // adds byte to body
        ++body_length;
        ++msg_length;
        // give error if content too long and exceeds buffer size (msg_length + content-length)
    }
    std::cout << GREEN << "DONE! :D" << NC << std::endl;
    body_done = true;
    return (status);
}

// goes through byte by byte and at every newline reads the previous line into the data structure.
int parser::parse(Request& request, int fd) {
    if (DEBUG)
        std::cout << "about to parse a new request on fd: " << fd << std::endl;
    int status = request._status;
    while (msg_length < BUFFER_SIZE) {
        if (!(status = __get_byte(request, fd)))
            break ;

    // keep BEFORE null byte check, bcs I want to check contet length field even if no body provided.
        if (header_done && !body_done) {
            status = __parse_body(request, fd);
            break ;
        }
        // if (buffer[msg_length] == '\0') { // [ ! ]
        //     std::cout << "EOF" << std::endl;
        //     break ;
        // }

        // if (status != WS_200_OK) // NEED THIS [ ? ]
        //     return (status) ; // if 0 it is end of file

        ++msg_length;
        ++line_length;

        if (!header_done) {
            if (buffer[msg_length - 1] == LF_int) { // if newline found:
                ++nb_lines;
                if (!(status = __parse_previous_line(request, (char *)buffer + msg_length - line_length, fd))) {
                    if (DEBUG)
                        std::cout << "final CRLF 2" << std::endl;
                    break ;
                }
                if (status != WS_200_OK)
                    return (status);
            }
        }
    } // end loop

    // check that minimum was provided
    if (!start_content)
        return (error_status(request, WS_400_BAD_REQUEST, "Empty request header"));
    if (!request_line_done)
        return (error_status(request, WS_400_BAD_REQUEST, "No request line provided"));
    if (!host_fields)
        return (error_status(request, WS_400_BAD_REQUEST, "No host field provided"));
    buffer[msg_length] = '\0';

    return (status);
}

// reads a byte and does some primary checks
// encoding must be a superset of US-ASCII [USASCII] -> max 128 (hex 80) (RFC 9112)
// ANY CR_int not folowed by LF_int is invalid and message is rejected (RFC 9112)
int parser::__get_byte(Request& request, int fd) {
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
int parser::__parse_previous_line(Request& request, const char* line, const int fd) {
    std::cout << YELLOW << "parsing previous line" << NC << std::endl;
    (void)fd;
    int status = WS_200_OK; // [ - ]
    std::cout << "LINE: " << line << std::endl;
    if (!start_content && (line_length == 2 && line[line_length - 2] == CR_int))
        ++nb_empty_lines_beginning;
    else if (start_fields && (line_length == 2 && buffer[msg_length - 2] == CR_int)) {
        if (DEBUG)  // final CRLF
            std::cout << "final CRLF after fields" << std::endl;
        header_done = true;
        if (request.field_is_value("transfer-encoding", "chunked")) { // there are more cases [ + ]
            request._is_chunked = true;
            request._waiting_for_chunks = true;
            parse_chunks(request, fd); // start reading them already
            // set body to done ?
            return (0);
        }
        if (!(request.header.method == "POST"))    // POST not only case with body [ + ]
            body_done = true;
        if (body_done)
            return (0);
    }
    else {
        start_content = true;
        if (isspace(line[0]) && !header_done)
            return (error_status(request, WS_400_BAD_REQUEST, "Whitespace at line begin"));
        if (!request_line_done && !header_done) {
            if ((status = __parse_request_line(request, line)) != WS_200_OK)
                return (status);
            request_line_done = true;
        }
        else
            __parse_field_line(request, line);
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
int parser::__parse_request_line(Request& request, const char* line) {
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
int parser::__parse_next_word_request_line(Request& request, int i, int skip) {
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
        // move this part back to response ? no whitespace allowed in target -> chack that
        request.header.target = word; // see later if it is valid
        if (!Request::replace_placeholders(request.header.target))
            return (error_status(request, WS_400_BAD_REQUEST, "Bad format uri"));
    }
    if (word_count == 3) {
        if ((strlen(word) != strlen(WS_HTTP_VERSION) || strncmp(WS_HTTP_VERSION, word, strlen(WS_HTTP_VERSION))))
            return (error_status(request, WS_400_BAD_REQUEST, "Bad version name"));
        request.header.version = word;
    }
    return (WS_200_OK);
}

// A server MUST respond with a 400 (Bad Request) status code to any HTTP/1.1 request message
// that lacks a Host header field and to any request message that contains more than one Host
// header field line or a Host header field with an invalid field value.
// TODO:
//  If the target URI includes an authority component, then a client MUST send a
// field value for Host that is identical to that authority component
int parser::__parse_field_line(Request& request, std::string line) {
    if (line_length > MAX_FIELD_LENGTH)
        return (error_status(request, WS_431_REQUEST_HEADER_FIELDS_TOO_LARGE, "Too large header field"));
    line = line.substr(0, line.length() - 2); // cut CRLF - presence checked on before
    std::string name;
    std::string values;
    start_fields = true;
    size_t colon;
    colon = line.find(':');
    if (colon == line.npos)
        return (error_status(request, WS_400_BAD_REQUEST, "Missing ':' in field line"));
    if (!colon || (colon && line[colon - 1] == ' '))
        return (error_status(request, WS_400_BAD_REQUEST, "Space before ':'"));
    str_tolower(line);
    name = line.substr(0, colon);
    if (name == "host")
        ++host_fields;
    if (host_fields > 1)
        return (error_status(request, WS_400_BAD_REQUEST, "Too many host fields"));
    if (colon == line.length())
        return (error_status(request, WS_400_BAD_REQUEST, "Empty field"));
    if (line[colon + 1] == ' ') // optional whitespace
        colon++;
    if (colon == line.length())
        return (error_status(request, WS_400_BAD_REQUEST, "Empty field"));
    if (line[colon + 1] == ' ')
        return (error_status(request, WS_400_BAD_REQUEST, "Too many whitespaces after ':' in field line"));
    values = line.substr(colon + 1);
    request._fields.make_field(name, values);
    request.fields.insert(std::pair<std::string, std::string>(name, values)); // [ - ]
    return (WS_200_OK);
}

// --------------------------------------------------------------------------------------------------------

HeaderFields::HeaderFields(){}

HeaderFields::HeaderFields(const HeaderFields& other)
    { _map = other._map; }

HeaderFields& HeaderFields::operator=(const HeaderFields& other) 
    { _map = other._map; return(*this); }

HeaderFields::~HeaderFields() {}

// separates key and value by firse ':' and values by ','
// [ + ] error check on map insert ?
void HeaderFields::make_field(const std::string& name, const std::string& values) {
    std::list<std::string> list;
    size_t delimiter_pos = 0;
    size_t prev_delimiter_pos = 0;

    while ((delimiter_pos = values.find(',', prev_delimiter_pos)) != std::string::npos) {
        while (prev_delimiter_pos != std::string::npos && values[prev_delimiter_pos] == ' ')
            prev_delimiter_pos++;
        if (prev_delimiter_pos != std::string::npos)
            list.push_back(values.substr(prev_delimiter_pos, delimiter_pos - prev_delimiter_pos));
        prev_delimiter_pos = delimiter_pos + 1;
    }
    if (prev_delimiter_pos != std::string::npos) {
        while (prev_delimiter_pos != std::string::npos && values[prev_delimiter_pos] == ' ')
            prev_delimiter_pos++;
        list.push_back(values.substr(prev_delimiter_pos, delimiter_pos - prev_delimiter_pos));
    }
    _map.insert(std::pair<std::string, std::list<std::string> >(name, list));
}

// --------------------------------------------------------------------------------------------------------

} // NAMESPACE http
} // NAMESPACE ws
