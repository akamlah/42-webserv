/* ************************************************************************ */
/*                                                                          */
/*                              Class: Response                             */
/*                                                                          */
/* ************************************************************************ */

#ifndef __RESPONSE_HPP__
# define __RESPONSE_HPP__

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
    std::string fragment; // from '#' to end
    // matched:
    std::string root; // as in config
    std::string file; // as in uri, or config in case of idex.html/.php
    std::string abs_path; // root + file -> ready for syscall open
    // interpreted:
    std::string type;
    std::string subtype;
    std::string extension;
};

// CGI ENV builder - - - - - - - - - - - - - - - - - - - - - - - - - 
// query format: x_www_form_urlencoded
// http://localhost:9999/data/mytext.txt?abc&def&hij&klm&nop&qrs&tuv&wxy

class Response {

    public:

        class ResponseException: public ws::exception {
            public:
                virtual const char* what() const throw();
        };

        // + exceptions

        // Response(const Tokens& tokens);
        Response(const Request& request, const config_data& config, const Tokens& tokens);
        // + cpy constr
        // + cpy assign ope
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

        void __build_response();
        void __respond_to_error();
        void __respond_get();
        void __respond_cgi_get();
        void __respond_post();
        void __respond_to_delete();

        void __respond_cgi_post();
        std::string cgiRespCreator();
        std::string cgiRespCreator_post();

        void __add_field(const std::string& field_name, const std::string& value);
        void __add_formatted_timestamp();
        std::string __generate_status_line() const;
        void __identify_resource(); // calls :
            void __interpret_target();
            void __validate_target_abs_path();
            void __extract_resource_extension();
            void __identify_resource_type();
        void __handle_type();
        void __upload_file();
        void __decide_persistency();
        void __response_to_string();

}; // CLASS Response

} // NAMESPACE http
} // NAMESPACE ws

#endif // __RESPONSE_HPP__
