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

        Response(const Request& request, const int fd);
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
        std::string _response_str;
        std::string _type;


    private:

        std::string __generate_status_line() const ;
        bool __decide_persistency();
        void __set_target_path();
        void __buffer_target(); // ?
    
        void __set_content_type_field();
        void __add_field(const std::string& field_name, const std::string& value);
        

}; // CLASS Response

} // NAMESPACE http
} // NAMESPACE ws

#endif // __RESPONSE_HPP__
