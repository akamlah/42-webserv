/* ************************************************************************ */
/*                                                                          */
/*                              Class: Connection                           */
/*                                                                          */
/* ************************************************************************ */

#ifndef __Connection_HPP__
# define __Connection_HPP__

#include "utility.hpp"
#include "http_type_traits.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"

#include <string>
#include <iostream>

namespace ws {
namespace http {

class Connection {

    public:

        // class : public ws::exception {
        //         virtual const char* what() const throw();
        // };

        Connection(const int fd, const http::Tokens& tokens, const config_data& config);
        Connection(const http::Tokens& tokens, const config_data& config);

        Connection(const Connection& other);
        ~Connection();

        int fd() const;
        bool is_good() const;
        bool is_persistent() const;
        int status() const;

        void establish(const int fd);
        void handle();

    private:

        int _fd;
        int _status;
        s_address _address;
        bool _is_persistent;
        Request _request;
        const Tokens& _tokens;
        const config_data& _config;

}; // CLASS Connection

} // NAMESPACE http
} // NAMESPACE ws
#endif // __Connection_HPP__
