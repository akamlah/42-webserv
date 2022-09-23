/* ************************************************************************ */
/*                                                                          */
/*                              Class: Request                              */
/*                                                                          */
/* ************************************************************************ */

#ifndef __REQUEST_HPP__
# define __REQUEST_HPP__

#include "utility.hpp"
#include "Socket.hpp"
#include "http_type_traits.hpp"

#include <string>
#include <iostream>
#include <map>

namespace ws {
namespace http {

class Request;

class parser {
    public:

        parser();
        ~parser();
        int parse(Request& request, const int fd);
        int error_status(Request& request, const int status, const char* msg = NULL) const ;

    private:

        size_t line_length;
        size_t nb_lines;
        size_t msg_length;
        size_t nb_empty_lines_beginning;
        size_t word_length;
        int host_fields;
        int word_count;
        bool start_content;
        bool request_line_done;
        bool header_done;
        //  A server that receives a method longer than any that it implements SHOULD 
        //  respond with a 501 (Not Implemented) status code -> BUFFER SIZES -> [ ! ] centralise later
        unsigned char buffer[BUFFER_SIZE];
        char request_line[REQUEST_LINE_LENGTH];
        char word[10000];

        bool __is_method(const char *word, size_t word_length) const;
        int __get_byte(Request& request, int fd);
        int __parse_previous_line(Request& request, const char* line);
        int __parse_request_line(Request& request, const char* line);
        int __parse_next_word_request_line(Request& request, int i, int skip);
        int __parse_field_line(Request& request, const char* line);
}; // CLASS parser

/*
    only thing that remains to check for now after the parsing is the validity of the target
*/
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

        class BadRead: public ws::exception {
                virtual const char* what() const throw();
        };
        class EofReached: public ws::exception {
                virtual const char* what() const throw();
        };

        Request();
        Request(const Request& other);
        Request& operator=(const Request& other);
        ~Request();

        int parse(const int fd);
        bool field_is_value(const char* field_name, const char* value) const;
        std::string get_field_value(const char* field_name) const;
        int status() const;
        bool is_persistent() const;

    private:

        static const char methods[4][10]; /* = {"GET", "HEAD", "POST", "DELETE"}; */

        header_t header;
        std::map<std::string, std::string> fields;
        bool _is_persistent;
        std::string error_msg;
        int _status;

}; // CLASS Request

} // NAMESPACE http
} // NAMESPACE ws
#endif // __REQUEST_HPP__
