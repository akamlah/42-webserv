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

namespace ws {
namespace http {

struct Resource {
    std::string path;
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

        // + exceptions

        // Response(const Tokens& tokens);
        Response(const Request& request, const config_data& config, const Tokens& tokens);
        // + cpy constr
        // + cpy assign ope
        ~Response();

        int status() const { return (_status); }
        void send(const int fd);
        bool is_persistent() const;

    private:
        int error_status(int status, const char* msg = NULL) ;
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

    private:

        void __build_response();
        void __respond_to_error();
        void __respond_get();
        void __respond_post();

        void __add_field(const std::string& field_name, const std::string& value);
        void __add_formatted_timestamp();
        std::string __generate_status_line() const;
        void __identify_resource(); // calls :
            void __identify_resource_path();
                void __validate_target();
            void __extract_resource_extension();
            void __identify_resource_type();
        void __handle_type();
        void __buffer_target_body();
        void __decide_persistency();
        void __response_to_string();

}; // CLASS Response

} // NAMESPACE http
} // NAMESPACE ws

#endif // __RESPONSE_HPP__
