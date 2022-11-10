/* ************************************************************************ */
/*                                                                          */
/*                              utility                                     */
/*                                                                          */
/* ************************************************************************ */

#ifndef __UTILITY_HPP__
# define __UTILITY_HPP__

#include <stdexcept>
#include <iostream>

namespace ws {

// some colors for output
#define RED "\033[0;31m"
#define CYAN "\033[0;36m"
#define YELLOW "\033[0;33m"
#define GREEN "\033[0;32m"
#define NC "\033[0m"

#ifndef DEBUG
#define DEBUG 0 // rule "make dbg" defines this at compiletime
#endif

class exception : public std::exception { }; // might put stuff in here ?

// throw error and print errno to stdout + eventually a custom message
template<class Exception>
void throw_print_error(const Exception& e, const char* message = NULL) {
    if (message)
        std::cout << RED << message << ": " << NC;
    std::cout << RED << strerror(errno) << NC << std::endl;
    throw e;
}

} // NAMESPACE ws

#endif // __UTILITY_HPP__
