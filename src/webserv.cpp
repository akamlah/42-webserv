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
        ws::Config configData(argv[1]);
        ws::Socket server_socket(AF_INET, SOCK_STREAM, 0); // these might better be server parameters ?
        ws::Server server(server_socket, configData.getConfigData());
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
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (-1);
    }
    
    // 1 parse config file
    // have a class object Config initialized with path of file, parsing done inside

    // 2 instantiate servers and types and addresses
    // form niw constructor is Socket, later pass a "ServerConfig" object to server in constructor
    // try {
    // }
    // catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }


    return (0);
}
