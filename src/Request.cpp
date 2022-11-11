/*                                                                          */
/* ************************************************************************ */
/*                              Class: Request                              */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Request.hpp"
#include <fstream>
#include <unistd.h>

namespace ws {
namespace http {

// --------------------------------------------------------------------------------------------------------

Request::Request(): _is_persistent(true), _status(WS_200_OK),
    _is_chunked(false), _waiting_for_chunks(false) { }

Request::~Request() { }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool Request::field_is_value(const std::string& field_name, const std::string& value) const {
    std::map<std::string, std::list<std::string> >::const_iterator ite = _fields._map.find(field_name);
    if (ite != _fields._map.end())
        for (std::list<std::string>::const_iterator lite = ite->second.begin(); lite != ite->second.end(); lite++)
            if (*lite == value)
                return (true);
    return (false);
}

bool Request::has_field_of_name(const std::string& field_name) const {
    std::map<std::string, std::list<std::string> >::const_iterator ite = _fields._map.find(field_name);
    if (ite != _fields._map.end())
                return (true);
    return (false);
}

// Returns empty list if no element found
std::list<std::string> Request::get_field_value(const std::string& field_name) const {
    std::map<std::string, std::list<std::string> >::const_iterator ret = _fields._map.find(field_name);
    if (ret != _fields._map.end())
        return (ret->second);
    return (std::list<std::string>() );
}

void Request::reset() {
    _parser.reset();
    memset(&header, 0, sizeof(header));
    error_msg = std::string();
    _is_chunked = 0;
    _body.str("");
    _body.clear();
    _content_length = 0;
    _fields._map.clear();
}

void parser::reset() {
    memset(&buffer, 0, BUFFER_SIZE);
    memset(&word, 0, 10000);
    memset(&request_line, 0, REQUEST_LINE_LENGTH);
    line_length = 0;
    nb_lines = 0;
    msg_length = 0;
    nb_empty_lines_beginning = 0;
    word_length = 0;
    host_fields = 0;
    word_count = 0;
    start_content = 0;
    request_line_done = 0;
    header_done = 0;
    start_fields = 0;
    body_done = 0;
}

int Request::status() const { return (_status); }

bool Request::is_persistent() const { return (_is_persistent);}

int Request::parse( const char* buffer, int brecv) {
    if (!_waiting_for_chunks) {
        _parser.parse(*this, buffer, brecv);
    }
    return (_status);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

static void str_tolower(std::string& line) {
    for (std::string::iterator it = line.begin(); it != line.end(); it++)
        *it = tolower(*it);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool Request::replace_placeholders(std::string& token) {
    std::string character;
    size_t i = 0;
    size_t delimiter_pos = 0;
    size_t prev_delimiter_pos = 0;
    while ((delimiter_pos = token.find('%', prev_delimiter_pos)) != std::string::npos) {
        if (delimiter_pos > token.length() - 3) {
            token.erase(delimiter_pos, std::string::npos);
            return (false);
        }
        character = token.substr(delimiter_pos + 1, 2);
        try {
            std::string repl;
            repl += (char)std::stoi(character, nullptr, 16);
            token = token.replace(delimiter_pos, 3, repl);
            i++;
        }
        catch (std::exception& e) {
            return (false);
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

int parser::error_status(Request& request, const int status, const char* msg) const {
    #if DEBUG
    if (msg)
        std::cout << RED << msg << ": " << NC;
    std::cout << RED << "Error: " << StatusPhrase()[status] << NC << std::endl; // temporary ?
    #endif

    request.error_msg = msg;
    request._status = status;
    request._is_persistent = false;
    return (status);
}

const char Request::methods[4][10] = {"GET", "HEAD", "POST", "DELETE"};

bool parser::is_method(const char *word, size_t word_length) const {
    int i = 0;
    while (i < 4) {
        if (!strncmp(Request::methods[i], word, word_length) && word_length == strlen(Request::methods[i]))
            return true;
        i++;
    }
    return false;
}

// PARSE HEADER - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

int parser::parse(Request& request,  const char* xbuffer, int brecv) {
    memcpy(buffer, xbuffer, brecv + 1);
    if (DEBUG) std::cout << "about to parse a new request on fd: " << std::endl;
    int status = request._status;
    while (msg_length < (size_t)brecv) {
        if (header_done && !body_done) {
            status = parse_body(request);
            body_done = true;
            break ;
        }
        if (!header_done && !(status = get_byte(request)))
            break ;
        if (status != WS_200_OK)
            return (status) ;
        if (buffer[msg_length] == '\0') {
            break ;
        }
        ++msg_length;
        ++line_length;
        if (!header_done && buffer[msg_length - 1] == LF_int) {
            ++nb_lines;
            char line[LINE_MAX];
            strlcpy(line, ((char *)buffer + msg_length - line_length), line_length + 1);
            if (!(status = parse_previous_header_line(request, line)))
                break ;
        }
        if (status != WS_200_OK)
            return (status);
    }
    if (!start_content)
        return (error_status(request, WS_400_BAD_REQUEST, "Empty request header"));
    if (!request_line_done)
        return (error_status(request, WS_400_BAD_REQUEST, "No request line provided"));
    if (!host_fields)
        return (error_status(request, WS_400_BAD_REQUEST, "No host field provided"));
    return (status);
}

int parser::get_byte(Request& request) {
    if (line_length > 1 && buffer[msg_length - 1] == CR_int && buffer[msg_length] != LF_int)
        return (error_status(request, WS_400_BAD_REQUEST, "CR not followed by LF"));
    return (WS_200_OK);
}

int parser::parse_previous_header_line(Request& request, const char* line) {
    int status = WS_200_OK;
    if (!start_content && (line_length == 2 && line[line_length - 2] == CR_int))
        ++nb_empty_lines_beginning;
    else if (start_fields && (line_length == 2 && buffer[msg_length - 2] == CR_int)) {
        if (DEBUG)
            std::cout << "final CRLF after fields" << std::endl;
        header_done = true;
    }
    else {
        start_content = true;
        if (isspace(line[0]) && !header_done)
            return (error_status(request, WS_400_BAD_REQUEST, "Whitespace at line begin"));
        if (!request_line_done && !header_done) {
            if ((status = parse_request_line(request, line)) != WS_200_OK)
                return (status);
            request_line_done = true;
        }
        else
            parse_field_line(request, line);
    }
    line_length = 0;
    return (WS_200_OK);
}

int parser::parse_request_line(Request& request, const char* line) {
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
        if (request_line[i] == SP_int || request_line[i] == CR_int) {
            if (i + 1 < line_length && request_line[i + 1] == SP_int)
                return (error_status(request, WS_400_BAD_REQUEST, "Consecutive spaces in start line"));
            if ((status = parse_next_word_request_line(request, i, skip) != WS_200_OK))
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

int parser::parse_next_word_request_line(Request& request, int i, int skip) {
    ++word_count;
    if (word_count > 3)
        return (error_status(request, WS_400_BAD_REQUEST, "Too many words in start line"));
    strlcpy(word, request_line + i - word_length + skip, word_length + 1 - skip);
    if (word_count == 1) {
        if (!is_method(word, word_length))
            return (error_status(request, WS_501_NOT_IMPLEMENTED, "Unknown method"));
        request.header.method = word;
    }
    if (word_count == 2) {
        request.header.target = word;
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

int parser::parse_field_line(Request& request, std::string line) {
    if (line_length > MAX_FIELD_LENGTH)
        return (error_status(request, WS_431_REQUEST_HEADER_FIELDS_TOO_LARGE, "Too large header field"));
    line = line.substr(0, line.length() - 2);
    std::string name;
    std::string values;
    start_fields = true;
    size_t colon;
    colon = line.find(':');
    if (colon == line.npos)
        return (error_status(request, WS_400_BAD_REQUEST, "Missing ':' in field line"));
    if (!colon || (colon && line[colon - 1] == ' '))
        return (error_status(request, WS_400_BAD_REQUEST, "Space before ':'"));
    if (line.find("boundary") == std::string::npos)
        str_tolower(line);
    name = line.substr(0, colon);
    if (line.find("boundary") != std::string::npos)
        str_tolower(name);
    if (name == "host")
        ++host_fields;
    if (host_fields > 1)
        return (error_status(request, WS_400_BAD_REQUEST, "Too many host fields"));
    if (colon != line.length() && line[colon + 1] == ' ')
        colon++;
    if (colon != line.length() && line[colon + 1] == ' ')
        return (error_status(request, WS_400_BAD_REQUEST, "Too many whitespaces after ':' in field line"));
    values = (colon == line.length()) ? std::string() : line.substr(colon + 1);
    request._fields.make_field(name, values);
    return (WS_200_OK);
}

// PARSE CONTENT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

int parser::parse_body(Request& request) {
    size_t body_length = 0;
    int status = request._status;
    request._content_length = 0;
    if (request.has_field_of_name("content-length")) {
        if (request.has_field_of_name("transfer-encoding"))
            return(error_status(request, WS_400_BAD_REQUEST, "Non compatible content-length and transfer-encoding fields detected"));
        request._content_length = std::atoi(request.get_field_value("content-length").begin()->c_str());
    }
    if (request.has_field_of_name("transfer-encoding")) {
        if (request.field_is_value("transfer-encoding", "chunked"))
            return(error_status(request, WS_501_NOT_IMPLEMENTED, "No chunked req. allowed"));
        else
            return (error_status(request, WS_501_NOT_IMPLEMENTED, "Transfer encoding not implemented"));
    }
    while (body_length <= request._content_length) {
        status =  get_byte(request);
        if (!status || buffer[msg_length] == '\0') {
            break ;
        }
        request._body << buffer[msg_length];
        ++body_length;
        ++msg_length;
    }
    if (!request.has_field_of_name("content-length") && !request._body.str().empty())
        return(error_status(request, WS_411_LENGTH_REQUIRED, 
            "Message content detected but no content-length or transfer-encoding fields provided"));
    return (status);
}

// --------------------------------------------------------------------------------------------------------
// Class HeaderFields
// --------------------------------------------------------------------------------------------------------

HeaderFields::HeaderFields(){}

HeaderFields::HeaderFields(const HeaderFields& other)
    { _map = other._map; }

HeaderFields& HeaderFields::operator=(const HeaderFields& other) 
    { _map = other._map; return(*this); }

HeaderFields::~HeaderFields() {}

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
