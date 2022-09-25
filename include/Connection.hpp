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

#include <string>
#include <iostream>

namespace ws {
namespace http {

class Connection {

    public:

        // class : public ws::exception {
        //         virtual const char* what() const throw();
        // };

        Connection(const int fd);
        Connection();

        Connection(const Connection& other);
        Connection& operator=(const Connection& other);
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
        // const config_data& _server;

}; // CLASS Connection

} // NAMESPACE http
} // NAMESPACE ws
#endif // __Connection_HPP__
