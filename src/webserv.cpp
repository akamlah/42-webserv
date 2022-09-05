#include "../include/Server.hpp"
#include "../include/Socket.hpp"
#include "../include/utility.hpp"
#include "../include/Config.hpp"


int main(int argc, char **argv) {

    if (argc != 2)
    {
        std::cout << "Wrong number of argument!.. try: ./webserv [configuration file]\n";
        return (0);
    }
    try
    {
        ws::Config ourserv(argv[1]);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    // 1 parse config file
    // have a class object Config initialized with path of file, parsing done inside

    // 2 instantiate servers and types and addresses
    // form niw constructor is Socket, later pass a "ServerConfig" object to server in constructor
    // try {
        ws::Socket server_socket(AF_INET, SOCK_STREAM, 0); // these might better be server parameters ?
        ws::Server server(server_socket, 8001);
    // }
    // catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }

    try { server.listen(100); /* also bind */ }
    catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }

    // all of next code in a "server run" function ?
    while (1) {

        // #if __APPLE__ //then use kqueue, else epoll ?

            // -> multiplexing -> handle inside server class ?

            ws::Socket new_server_connection;
            try { server.accept(new_server_connection); }
            catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }
            try { server.handle_connection(new_server_connection); }
            catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }

        // #endif

    }

    return (0);
}
