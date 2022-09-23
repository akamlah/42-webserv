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

    // private:

    //     typedef struct s_header {
    //         std::string method;
    //         std::string target;
    //         std::string version;
    //     } header_t;    public:

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

        int status() const { return (_status); }
        void send(const int fd);
        bool is_persistent() const;

        void runSendCig( const std::string & path );
        void sendPicResp( const std::string & path );
        void createFieldStream();


    private:
        const char* throw_status(int status, const char* msg = NULL) const ;
        // mainly for target check (fstream open error handeling)
        int error_status(Request& request, const int status, const char* msg = NULL) const ;

        Request _request;
        int _status;
        bool _is_persistent;

        std::string _path;
        std::stringstream _fields_stream;
        std::stringstream _body;

        std::string _type;
        std::string _subtype;
        std::string _extension;

        std::string _response_str; // the whole response


    private:
        
        void __add_field(const std::string& field_name, const std::string& value);
        void __set_type(const std::string& type, const std::string& subtype = "");

        std::string __generate_status_line() const;
        void __set_target_path();
        void __set_content_type_field();
        void __handle_type();
        void __buffer_target_body();
        void __decide_persistency();
        void __generate_response();
        

}; // CLASS Response

} // NAMESPACE http
} // NAMESPACE ws

#endif // __RESPONSE_HPP__
