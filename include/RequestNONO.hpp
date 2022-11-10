
#ifndef REQUEssST_HPP
# define REQUEssST_HPP

#include <string>
#include <iostream>
#include <map>
#include <list>
#include <sstream>

#include "utility.hpp"
#include "http_tokens.hpp"
#include "RequestParser.hpp"

namespace ws {
namespace http {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Debugging

// Activate/deactivate debugging for this file if DEBUG flag is set
#ifndef DEBUG_REQUEST
 #define DEBUG_REQUEST 1 // 0
#endif

#if DEBUG
 #ifndef WS_req_debug
  #ifdef DEBUG_REQUEST
//  #define WS_req_debug(message) (std::cout << __FILE__ << ": " << message << std::endl)
//  #define WS_req_debug(message) (std::cout << __FILE__ << ":" << __LINE__ << " " << message << std::endl)
   #define WS_req_debug(message) (std::cout << message << std::endl)
   #define WS_req_debug_n(message) (std::cout << message)
  #else
   #define WS_req_debug(message) ((void) 0)
   #define WS_req_debug_n(message) ((void) 0)
  #endif
 #endif
#else
 #define WS_req_debug(message) ((void) 0)
 #define WS_req_debug_n(message) ((void) 0)
#endif

class Request {

    // member classes/ types - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    public:

        enum rq_state { REQ_ERROR = 0, REQ_OK };

    //  PARSER

    private:

        // ------------------------------------------------------------------------
        // HTTP REQUEST PARSER - fills Request data structures, given a _buffer
        // ------------------------------------------------------------------------
        class Parser { // member class http::Request::Parser
            private:

                // writing data to:
                Request& _request;

                // counters / tmp counters
                size_t _bparsed; // bytes parsed total
                size_t _nb_lines;
                size_t _line_len; // tmp -> make scoped ?
                // size_t _word_len; // tmp -> make scoped ?
                size_t _nb_leading_crlf; // skip initial CRLFs

                int    _nb_host_fields;

                // utility c _buffers
                char _request_line[REQUEST_LINE_LENGTH];
                char _word[10000];
                char _buffer[BUFFER_SIZE];
                // state
                enum prs_state {
                    CLEAR = 0,
                    HEADER_START, // might be multiple CRLFs before status line.
                    START_REQUEST_LINE,
                    REQUEST_LINE_DONE,
                    START_FILEDS,
                    HEADER_DONE,
                    START_BODY,
                    BODY_DONE
                };

                prs_state _state;

            public:
                Parser(Request& request) : _request(request) { init(); }
                ~Parser() {}

                // a way to return error while parsing and thereby setting various flags/messages
                // in 1 function call (wrap return)
                rq_state wrap_error(const int status, const std::string& message = std::string()) const {
                    if (!message.empty()) { WS_req_debug_n(RED << message << ": " << NC); }
                    WS_req_debug(RED << "Error: " << _request._tokens.status_phrases[status] << NC);
                    _request._error_message = message; // save optional error message
                    _request._status = status; // set HTTP status code
                    _request._state = REQ_ERROR; // change status to error
                    return (REQ_ERROR);
                }

                // returns bytes parsed. ?
                int parse_header(char* buffer, size_t brecv) {
                    memcpy(_buffer, buffer, brecv + 1);
                    std::cout << YELLOW << "parsing header" << NC << std::endl;
                    init();
                    while (_bparsed <= brecv) {
                        if (_state < HEADER_DONE && check_byte_header(_buffer + _bparsed) == REQ_ERROR) {
                            WS_req_debug("Bad encoding found at " << brecv << " line " << _nb_lines);
                            return (REQ_ERROR);
                        }
                        if (_buffer[_bparsed] == '\0') { // [ ? ] - should never happen
                            WS_req_debug("Nullbyte found at " << brecv << " line " << _nb_lines);
                            break ;
                        }
                        ++_bparsed;
                        ++_line_len;
                        if (_state < HEADER_DONE && _buffer[_bparsed - 1] == LF_int) {
                            ++_nb_lines;
                            if (!(_line_len >= 2 && _buffer[_bparsed - 2] == CR_int))
                                return (wrap_error(WS_400_BAD_REQUEST, "LF not preceded by CR"));
                            if (parse_previous_header_line((char *)_buffer + _bparsed - _line_len) == REQ_ERROR)
                                return (REQ_ERROR);
                            if (_state == HEADER_DONE) // final CRLF
                                break ;
                        }
                    }
                    switch (_state) // end of loop (either nullbyte or all bytes parsed) final check:
                    {
                        case CLEAR :
                            return (wrap_error(WS_400_BAD_REQUEST, "Empty request header"));
                        case HEADER_START :
                            return (wrap_error(WS_400_BAD_REQUEST, "No complete request line found"));
                        case REQUEST_LINE_DONE :
                        case START_FILEDS :
                            if (!_nb_host_fields)
                                return (wrap_error(WS_400_BAD_REQUEST, "No host field provided"));
                        default:
                            return (REQ_OK);
                    }
                }

                void reset() { // [ + ] ALL VARS!
                    _bparsed = 0;
                    _state = CLEAR;
                    memset(_request_line, 0, REQUEST_LINE_LENGTH);
                    memset(_word, 0, MAX_WORD_LENGTH);
                }

            private:

                void init() { // [ + ] ALL VARS!
                    _bparsed = 0;
                    _state = CLEAR;
                    memset(_request_line, 0, REQUEST_LINE_LENGTH);
                    memset(_word, 0, MAX_WORD_LENGTH);
                }

                rq_state check_byte_header(char* byte) {

                    std::cout << YELLOW << "check byte header" << NC << std::endl;

                    if (!((int)*byte < 0x80 && (int)*byte >= 0))
                        return (wrap_error(WS_400_BAD_REQUEST, "Bad encoding"));
                    if (_line_len > 1 && *(byte - 1) == CR_int && *byte != LF_int)
                        return (wrap_error(WS_400_BAD_REQUEST, "CR not followed by LF"));
                    return (REQ_OK);
                }
        size_t  word_length;
        int     word_count;

                rq_state parse_previous_header_line(const char* line) {
                    std::cout << YELLOW << "parse previous line header" << NC << std::endl;
                    std::cout << CYAN << line << NC <<  std::endl;
                    if (_state < START_REQUEST_LINE && (_line_len == 2 && line[_line_len - 2] == CR_int))
                        ++_nb_leading_crlf;
                    else if (_state >= START_FILEDS && (_line_len == 2 && _buffer[_bparsed - 2] == CR_int)) {
                        if (DEBUG)  // final CRLF
                            std::cout << "final CRLF after fields" << std::endl;
                        _state = HEADER_DONE;
                    }
                    else {
                        if (isspace(line[0]) && _state < HEADER_DONE)
                            return (wrap_error(WS_400_BAD_REQUEST, "Whitespace at line begin"));
                        if (_state < START_REQUEST_LINE) {
                            if (parse_request_line(line) == REQ_ERROR)
                                return (REQ_ERROR);
                            _state = START_REQUEST_LINE;
                        }
                        else
                            parse_field_line(line);
                    }
                    _line_len = 0;
                    return (REQ_OK);
                }

                rq_state parse_request_line(const char* line) {
                    std::cout << YELLOW << "parsing request line" << NC << std::endl;

                    WS_req_debug("Parsing request line " << line);
                    size_t i = 0;
                    int skip = 0;
                    if (_line_len > REQUEST_LINE_LENGTH)
                        return (wrap_error(WS_501_NOT_IMPLEMENTED, "Header too long"));
                    strlcpy(_request_line, line, _line_len + 1);
                    if (!(_request_line[_line_len - 2] == CR_int && _request_line[_line_len - 1] == LF_int))
                        return (wrap_error(WS_400_BAD_REQUEST, "No CRLF at end of line"));
                    while (i < _line_len) {
                        if ( i < _line_len - 2 && isspace(_request_line[i]) && _request_line[i] != SP_int)
                            return (wrap_error(WS_400_BAD_REQUEST, "Bad spacing"));
                        if (_request_line[i] == SP_int || _request_line[i] == CR_int) { // end of a word
                            if (i + 1 < _line_len && _request_line[i + 1] == SP_int)
                                return (wrap_error(WS_400_BAD_REQUEST, "Consecutive spaces in start line"));
                            if (parse_next_word_request_line(i, skip) == REQ_ERROR)
                                return (REQ_ERROR);
                            word_length = 0;
                            skip = 1;
                        }
                        word_length++;
                        i++;
                        if (_request_line[i] == LF_int) {
                        if (word_count < 3)
                                return (wrap_error(WS_400_BAD_REQUEST, "Too few words in start line"));
                            break ;
                        }
                    }
                    return (REQ_OK);
                }

                bool is_method(const char *word, size_t word_length) const {
                    const char methods[4][10] = {"GET", "HEAD", "POST", "DELETE"};
                    int i = 0;
                    while (i < 4) {
                        if (!strncmp(methods[i], word, word_length) && word_length == strlen(methods[i]))
                            return true;
                        i++;
                    }
                    return false;
                }

                bool replace_placeholders(std::string& token) {
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

                rq_state parse_next_word_request_line(int i, int skip) {
                    // std::cout << YELLOW << "parsing word req line" << NC << std::endl;

                    ++word_count;
                    if (word_count > 3)
                        return (wrap_error(WS_400_BAD_REQUEST, "Too many words in start line"));
                    strlcpy(_word, _request_line + i - word_length + skip, word_length + 1 - skip); // null terminated copy of word
                    if (word_count == 1) {
                        if (!is_method(_word, word_length))
                            return (wrap_error(WS_501_NOT_IMPLEMENTED, "Unknown method"));
                        _request._header.method = _word;
                    }
                    if (word_count == 2) {
                        _request._header.target = _word;
                        if (!replace_placeholders(_request._header.target))
                            return (wrap_error(WS_400_BAD_REQUEST, "Bad format uri"));
                    }
                    if (word_count == 3) {
                        if ((strlen(_word) != strlen(WS_HTTP_VERSION) || strncmp(WS_HTTP_VERSION, _word, strlen(WS_HTTP_VERSION))))
                            return (wrap_error(WS_400_BAD_REQUEST, "Bad version name"));
                        _request._header.version = _word;
                    }
                    return (REQ_OK);
                }

                void str_tolower(std::string& line) {
                    for (std::string::iterator it = line.begin(); it != line.end(); it++)
                        *it = tolower(*it);
                }

                rq_state parse_field_line(std::string line) {
                    std::cout << YELLOW << "parsing field line" << NC << std::endl;
                    std::cout  << "|" << line << "|" << std::endl;

                    if (_line_len > MAX_FIELD_LENGTH)
                        return (wrap_error(WS_431_REQUEST_HEADER_FIELDS_TOO_LARGE, "Too large header field"));
                    _state = START_FILEDS;
                    std::string name;
                    std::string values;
                    size_t colon;
                    line = line.substr(0, line.length() - 2); // cut CRLF - presence checked on before
                    colon = line.find(':');
                    if (colon == line.npos)
                        return (wrap_error(WS_400_BAD_REQUEST, "Missing ':' in field line"));
                    if (!colon || (colon && line[colon - 1] == ' '))
                        return (wrap_error(WS_400_BAD_REQUEST, "Space before ':'"));
                    if (line.find("boundary") == std::string::npos)
                        str_tolower(line);
                    name = line.substr(0, colon);
                    if (line.find("boundary") != std::string::npos)
                        str_tolower(name);
                    if (name == "host")
                        ++_nb_host_fields;
                    if (_nb_host_fields > 1)
                        return (wrap_error(WS_400_BAD_REQUEST, "Too many host fields"));
                    if (colon != line.length() && line[colon + 1] == ' ') // optional whitespace
                        colon++;
                    if (colon != line.length() && line[colon + 1] == ' ')
                        return (wrap_error(WS_400_BAD_REQUEST, "Too many whitespaces after ':' in field line"));
                    values = (colon == line.length()) ? std::string() : line.substr(colon + 1);
                    _request._header.fields.make_field(name, values);
                    return (REQ_OK);
                }

        }; // member class http::Request::Parser
        // ------------------------------------------------------------------------


    // DATA STRUCTURES

    private:

        struct HeaderFields { // member class http::Request::HeaderFields

            public:
                typedef std::map<std::string, std::list<std::string> >::iterator iterator;
                typedef std::map<std::string, std::list<std::string> >::const_iterator const_iterator;

            private:
                std::map<std::string, std::list<std::string> > _map;
                friend class Request;

            public:
                HeaderFields() {}
                HeaderFields(const HeaderFields& other) { _map = other._map; }
                HeaderFields& operator=(const HeaderFields& other) { _map = other._map; return (*this); }
                ~HeaderFields() {}

                // splits the string by comma ',' and saves avary substring as element in a list,
                // wich in turn is inserted as value of the map node of key 'name'.
                void make_field(const std::string& name, const std::string& values) {
                    std::list<std::string> list;
                    size_t pos = 0;
                    size_t prev = 0;
                    while ((pos = values.find(',', prev)) != std::string::npos) {
                        while (prev != std::string::npos && values[prev] == ' ')
                            prev++;
                        if (prev != std::string::npos)
                            list.push_back(values.substr(prev, pos - prev));
                        prev = pos + 1;
                    }
                    if (prev != std::string::npos) {
                        while (prev != std::string::npos && values[prev] == ' ')
                            prev++;
                        list.push_back(values.substr(prev, pos - prev));
                    }
                    _map.insert(std::pair<std::string, std::list<std::string> >(name, list));
                }
        }; // member class http::Request::HeaderFields

        typedef struct HeaderFields headerfields_t;

        struct Header { // member class http::Request::Header
            std::string method;
            std::string target;
            std::string version;
            headerfields_t fields;
        }; // member class http::Request::Header

    public:

        typedef struct Header header_t;

    // member attributes - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    private:

        Parser _parser;

        //  DATA

        header_t _header;
        std::stringstream _body;

        // STATE

        rq_state _state; //    enum rq_state { ERROR = 0, OK };
        std::string _error_message;
        int _status; //     HTTP status code emitted (as in 'http_tokens.hpp')
        const http::Tokens& _tokens; // is rvalue so they don't have to be built every time again

        friend class Parser;
        friend class Response;

    public:

        Request(const http::Tokens& tokens) : _parser(*this), _tokens(tokens) { init(); }
        Request(const Request& other) : _parser(*this), _tokens(other._tokens) { copy(other); }
        Request& operator=(const Request& other) { reset(); copy(other); return (*this); }
        ~Request() {} // DEALLOC IF ALLOC [ ! ]

    // accessors - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        const header_t& header() const { return (_header); }
        std::string body() const { return (_body.str()); }
        rq_state state() const { return (_state); }

        // ACCESS HEADER FILEDS :

        // cfr with master - old bug in find() [ ! ] and adapt to new structure
        bool field_is_value(const std::string& field_name, const std::string& value) const {
            headerfields_t::const_iterator ite = _header.fields._map.find(field_name);
            if (ite != _header.fields._map.end())
                for (std::list<std::string>::const_iterator lite = ite->second.begin(); lite != ite->second.end(); lite++)
                    if (*lite == value)
                        return (true);
            return (false);
        }

        bool has_field_of_name(const std::string& field_name) const {
            headerfields_t::const_iterator ite = _header.fields._map.find(field_name);
            if (ite != _header.fields._map.end())
                        return (true);
            return (false);
        }

        std::list<std::string> get_field_value(const std::string& field_name) const {
            headerfields_t::const_iterator ret = _header.fields._map.find(field_name);
            if (ret != _header.fields._map.end())
                return (ret->second);
            return (std::list<std::string>() );
        }

    // core - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

        void reset() {
            _parser.reset();
            memset(std::addressof(_header), 0, sizeof(header_t));
            _body.str("");
            _body.clear();
            _state = REQ_OK;
            _error_message = std::string();
            _status = WS_200_OK;
        }

        // parses until last CRLF after HTTP header, on a _buffer with brcv bytes written on.
        void parse_header(char* _buffer, size_t brecv) {
            if (_parser.parse_header(_buffer, brecv) == REQ_ERROR) {
                // error
            }
        }

    private:

        void init() {
            memset(std::addressof(_header), 0, sizeof(header_t));
            _state = REQ_OK;
            _status = WS_200_OK;
        }

        void copy(const Request& other) {
            memcpy(std::addressof(_header), std::addressof(other._header), sizeof(header_t));
            _body.str("");
            _body.clear();
            _body << other._body;
            _state = other._state;
            _error_message = other._error_message;
            _status = other._status;
        }

}; //class Request

} // NAMESPACE http
} // NAMESPACE ws
#endif // REQUEST_HPP
