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
#include "http_type_traits.hpp"
#include <fstream>
#include <sstream>

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

        Response(const Tokens& tokens);
        Response(const Request& request, const Tokens& tokens);
        // + cpy constr
        // + cpy assign ope
        ~Response();

        int status() const { return (_status); }
        void send(const int fd);
        bool is_persistent() const;

    private:
        const char* throw_status(int status, const char* msg = NULL) const ;
        // mainly for target check (fstream open error handeling)
        int error_status(Request& request, const int status, const char* msg = NULL) const ;

        Request _request;
        int _status;
        bool _is_persistent;

        Resource _resource;
        std::stringstream _fields_stream; // buffered header fields
        std::stringstream _body; // buffered resource body if any
        std::string _response_str; // the whole response

        const Tokens _tokens;

    private:
        
        void __add_field(const std::string& field_name, const std::string& value);
        void __set_type(const std::string& type, const std::string& subtype = "");

        std::string __generate_status_line() const;

        void __identify_resource(); // calls :
            void __identify_resource_path();
            void __extract_resource_extension();
            void __identify_resource_type();

        void __handle_type();
        void __buffer_target_body();
        void __decide_persistency();
        void __generate_response();

}; // CLASS Response

} // NAMESPACE http
} // NAMESPACE ws

#endif // __RESPONSE_HPP__
