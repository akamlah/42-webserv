/* ************************************************************************ */
/*                                                                          */
/*                              Class: Request                              */
/*                                                                          */
/* ************************************************************************ */

#ifndef __REQUEST_HPP__
# define __REQUEST_HPP__

#include <string>
#include <unistd.h> // read
#include <iostream>
#include "utility.hpp"
#include "Socket.hpp"

namespace ws {

class Request {

    public:
        class BadRead: public ws::exception {
                virtual const char* what() const throw();
        };
        class EofReached: public ws::exception {
                virtual const char* what() const throw();
        };

        Request(const Socket& new_connection);
        // + cpy constr
        // + cpy assign ope
        ~Request();

    // pvt ?
        /*
        methods: enum ? // subject: at least GET, POST and DELETE methods
            GET
            HEAD
            POST
            PUT
            DELETE
            CONNECT
            OPTIONS
            TRACE
            PATCH
        */
        std::string method; // or enum ?
        std::string protocol;

}; // CLASS Request

} // NAMESPACE ws

#endif // __REQUEST_HPP__
