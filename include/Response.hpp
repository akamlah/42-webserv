/* ************************************************************************ */
/*                                                                          */
/*                              Class: Response                             */
/*                                                                          */
/* ************************************************************************ */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "Request.hpp"
#include "utility.hpp"
#include "CGI.hpp"
#include "Config.hpp"
#include "http_type_traits.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <fcntl.h> // open
#include <unistd.h> //
#include <errno.h>
#include <list>
#include <cstdio>


namespace ws {
namespace http {

struct Resource {
    // as in URI:
    std::string path; // everything between '/' and '?'
    std::string query; // everything between '?' and '#'
    // matched:
    std::string root; // as in config
    std::string file; // as in uri, or config in case of idex.html/.php
    std::string abs_path; // root + file -> ready for syscall open
    // interpreted:
    std::string type;
    std::string subtype;
    std::string extension;
};

class Response {

    public:

        class ResponseException: public ws::exception {
            public:
                virtual const char* what() const throw();
        };

        Response(const Request& request, const config_data& config, const Tokens& tokens);
        ~Response();

        int status() const { return (_status); }
        void send(const int fd);
        bool is_persistent() const;
        static void append_slash(std::string& path);
        static void remove_leading_slash(std::string& path);

    private:
        int throw_error_status(int status, const char* msg = NULL) ;
        // mainly for target check (fstream open error handeling)

        const Request&      _request;
        const config_data&  _config;
        const Tokens&       _tokens;
        int                 _status;
        bool                _is_persistent;
        Resource            _resource;
        std::stringstream   _fields_stream; // buffered header fields
        std::stringstream   _body; // buffered resource body if any
        std::string         _response_str; // the whole response

        std::string error_msg;

    private:

        void respond_to_delete();
        void build_response();
        void respond_to_error();
        void respond_get();
        void respond_cgi_get();
        void respond_post();

        void respond_cgi_post();
        std::string cgiRespCreator();
        std::string cgiRespCreator_post();
        bool getValid(const std::string & nameof);

        void add_field(const std::string& field_name, const std::string& value);
        void add_formatted_timestamp();
        std::string generate_status_line() const;
        void identify_resource(); // calls :
            void interpret_target();
            void validate_target_abs_path();
            void extract_resource_extension();
            void identify_resource_type();
        void upload_file();
        void response_to_string();

}; // CLASS Response

} // NAMESPACE http
} // NAMESPACE ws

#endif // RESPONSE_HPP
