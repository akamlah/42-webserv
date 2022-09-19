/* ************************************************************************ */
/*                                                                          */
/*                              Class: Response                             */
/*                                                                          */
/* ************************************************************************ */

#ifndef __RESPONSE_HPP__
# define __RESPONSE_HPP__

#include "Request.hpp"
#include "Socket.hpp"
#include "utility.hpp"
#include "http_type_traits.hpp"
#include <fstream>
#include <sstream>

namespace ws {
namespace http {


class Response {
    
    public:

        class ResponseException: public ws::exception {
            public:
                virtual const char* what() const throw();
        };

        // + exceptions

        Response(const Request& request);
        // + cpy constr
        // + cpy assign ope
        ~Response();

        const char *c_str() const; // full response to c string
        int status() const { return (_status.get_current()); }

    private:

        const char* throw_status(int status, const char* msg = NULL) const ;
        // mainly for target check (fstream open error handeling)
        int error_status(Request& request, const int status, const char* msg = NULL) const ;

        Request _request;

        Status _status;
        const Socket client_socket;
        
        std::string _path;
        std::stringstream _fields_stream;
        std::string _response_str;
    
    public:

        bool keep_alive; // set to fase if error occurs or connection not meant to be persistent

    private:

        std::string __generate_status_line() const ;
        bool __decide_persistency();
        void __set_target_path();
        void __buffer_target(); // ?
    
        void __set_content_type();
        void __add_field(const char* field_name, const char* value);
        

}; // CLASS Response

} // NAMESPACE http
} // NAMESPACE ws

#endif // __RESPONSE_HPP__
