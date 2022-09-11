/* ************************************************************************ */
/*                                                                          */
/*                              utility                                     */
/*                                                                          */
/* ************************************************************************ */

#ifndef __UTILITY_HPP__
# define __UTILITY_HPP__

#include <stdexcept>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

namespace ws {

typedef struct c_data {
	int		port;
	std::string	host;
	std::string	root;
	std::string	index;
} config_data;

// some colors for output
#define RED "\033[0;31m"
#define CYAN "\033[0;36m"
#define NC "\033[0m"

#define DEBUG 0

// maybe we will not need this
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
