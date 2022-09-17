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

/*
    only thing that remains to check for now after the parsing is the validity of the target
*/
class Request {

    public:

        class BadRead: public ws::exception {
                virtual const char* what() const throw();
        };
        class EofReached: public ws::exception {
                virtual const char* what() const throw();
        };

    private:

        typedef struct s_header {
            std::string method;
            std::string target;
            std::string version;
        } header_t;

    public:

        Request(const Socket& client);
        Request(const Request& other);
        Request& operator=(const Request& other);
        ~Request();

        void parse();
        const Socket& get_client() const;
        
        Status _status;

        int status() const { return (_status.get_current()); }

    private:

        class parser {
            public:

                parser();
                ~parser();
                int parse(Request& request, int fd);
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
                //  respond with a 501 (Not Implemented) status code -> BUFFER SIZES
                unsigned char buffer[BUFFER_SIZE];
                char request_line[REQUEST_LINE_LENGTH];
                char word[10000];

                bool __is_method(const char *word, size_t word_length) const;
                int __get_byte(Request& request, int fd);
                int __parse_previous_line(Request& request, const char* line);
                int __parse_request_line(Request& request, const char* line);
                int __parse_next_word_request_line(Request& request, int i, int skip);
                int __parse_field_line(Request& request, const char* line);
        }; // CLASS Request::parser

        static const char methods[4][10]; /* = {"GET", "HEAD", "POST", "DELETE"}; */

    public: // make private or response friend of request

        parser parser;
        header_t header;
        std::map<std::string, std::string> fields;
        const Socket client_socket;
        std::string error_msg;
        // Status status;

}; // CLASS Request

} // NAMESPACE http
} // NAMESPACE ws
#endif // __REQUEST_HPP__
