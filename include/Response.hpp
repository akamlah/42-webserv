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
#include "CGI.hpp"
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

        Response(const Request& request, const config_data & configData);
        Response(const Request& request);
        void runSendCig( const std::string & path );
        void sendPicResp( const std::string & path );
        void createFieldStream();
        // + cpy constr
        // + cpy assign ope
        ~Response();

        const char *c_str() const; // full response to c string
        int status() const { return (_status.get_current()); }

    private:
        const char* throw_status(int status, const char* msg = NULL) const ;
        // mainly for target check (fstream open error handeling)
        int error_status(Request& request, const int status, const char* msg = NULL) const ;

        Status _status;
        const Socket client_socket;
        const Request request;
        std::string _status_line;
        std::string _root;
        std::string _file;
        std::string _response_str;
        std::stringstream fields_stream;

        // status-line = HTTP-version SP status-code SP [ reason-phrase ]

}; // CLASS Response

} // NAMESPACE http
} // NAMESPACE ws

#endif // __RESPONSE_HPP__
