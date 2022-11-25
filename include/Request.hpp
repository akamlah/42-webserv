/* ************************************************************************ */
/*                                                                          */
/*                              Class: Request                              */
/*                                                                          */
/* ************************************************************************ */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "utility.hpp"
#include "http_tokens.hpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <list>
#include <sstream>
#include <sys/socket.h>

namespace ws {
namespace http {

class Request;

class HeaderFields {
    friend class Request;
public:
    HeaderFields();
    HeaderFields(const HeaderFields& other);
    HeaderFields& operator=(const HeaderFields& other);
    ~HeaderFields();
    void make_field(const std::string& name, const std::string& values);
private:
    std::map<std::string, std::list<std::string> > _map;
};


class parser {

    public:

        parser();
        ~parser();

        int parse(Request& request, const char* buffer, int brecv);
        int error_status(Request& request, const int status, const char* msg = NULL) const ;
        void reset();

#if (!DEBUG)
    private:
#endif

        size_t  line_length;
        size_t  nb_lines;
        size_t  msg_length;
        size_t  nb_empty_lines_beginning;
        size_t  word_length;
        int     host_fields;
        int     word_count;

        bool    start_content;
        bool    request_line_done;
        bool    header_done;
        bool    start_fields;
        bool    body_done;

        unsigned char   buffer[BUFFER_SIZE];
        char            request_line[REQUEST_LINE_LENGTH];
        char            word[10000];

#if (DEBUG)
    private:
#endif

        bool is_method(const char *word, size_t word_length) const;
        int get_byte(Request& request);
        int parse_body(Request& request );
        int parse_previous_header_line(Request& request, const char* line);
        int parse_request_line(Request& request, const char* line);
        int parse_next_word_request_line(Request& request, int i, int skip);
        int parse_field_line(Request& request, std::string line);
}; // CLASS parser


class Request {
    private:

        typedef struct s_header {
            std::string method;
            std::string target;
            std::string version;
        } header_t;

    friend class parser;
    friend class Response;

    public:

        Request();
        ~Request();

        int parse( const char* buffer, int brecv);

        int status() const;
        bool is_persistent() const;
        bool field_is_value(const std::string& field_name, const std::string& value) const;
        bool has_field_of_name(const std::string& field_name) const;
        std::list<std::string> get_field_value(const std::string& field_name) const;
        static bool replace_placeholders(std::string& token);
        void reset();
        header_t                            header;

    private:

        static const char                   methods[4][10];

        parser                              _parser;
        HeaderFields                        _fields;
        bool                                _is_persistent;
        std::string                         error_msg;
        int                                 _status;
        bool                                _is_chunked;
        std::stringstream                   _body;
        size_t                              _content_length;

    public:

        bool                                _waiting_for_chunks;

}; // CLASS Request


} // NAMESPACE http
} // NAMESPACE ws
#endif // REQUEST_HPP
