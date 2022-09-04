/* ************************************************************************ */
/*                                                                          */
/*                              Class: Response                             */
/*                                                                          */
/* ************************************************************************ */

#include "../include/Response.hpp"

namespace ws {

// exceptions

Response::Response(const Request& request) {
    // HERE ALL HTTP response PROTOCOL IMPLEMENTED: generate header and fetch content.
    
    (void)request; // request comes in parsed and with filled attribute fields that can be read.

    // temporary:
    FILE *html_data = fopen("./example_sites/example1/index.html", "r");
    if (!html_data)
        std::cout << "BAD PATH" << std::endl;
    char response_data[1024];
    if (fgets(response_data, 1023, html_data) == NULL) {
        std::cout << CYAN << "Error: File not found or empty" << NC << std::endl;
    }
    body = response_data;
    header = "HTTP/1.1 200 OK\r\n\n";
    response = header + body;
}

Response::~Response() {}

const char *Response::c_str() const {
    return (response.c_str());
}

} // NAMESPACE ws

