/* ************************************************************************ */
/*                                                                          */
/*                              Class: Response                             */
/*                                                                          */
/* ************************************************************************ */

#ifndef __RESPONSE_HPP__
# define __RESPONSE_HPP__

#include "Request.hpp"
#include "utility.hpp"

namespace ws {

class Response {
    
    public:

        // + exceptions

        Response(const Request& request);
        // + cpy constr
        // + cpy assign ope
        ~Response();

        const char *c_str() const; // full response to c string

    private:
        // ?
        std::string header;
        std::string body;
        std::string response;

}; // CLASS Response

} // NAMESPACE ws

#endif // __RESPONSE_HPP__
