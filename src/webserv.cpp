#include "../include/Server.hpp"
#include "../include/Socket.hpp"
#include "../include/utility.hpp"
#define PORT 8001
#define BACKLOG 100

int main() {

    // 1 parse config file
    // have a class object Config initialized with path of file, parsing done inside

    // 2 instantiate servers and types and addresses
    // form niw constructor is Socket, later pass a "ServerConfig" object to server in constructor
    // try {
        ws::Socket server_socket(AF_INET6, SOCK_STREAM, 0); // these might better be server parameters ?
        ws::Server server(server_socket, PORT);
    // }
    // catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }

    try { server.listen(BACKLOG); /* also bind */ }
    catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }

    // all of next code in a "server run" function ?
    ws::Socket new_server_connection;
    while (1) {

        #if __APPLE__ //then use kqueue, else epoll ?
            // -> multiplexing -> handle inside server class ?
            try { server.run(3 * 60 * 1000); }
            catch (ws::exception& e) { std::cout << e.what() << std::endl; return -1; }
            // try { server.accept(); }
            // catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }
            // try { server.handle_connection(new_server_connection); }
            // catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }

        #endif

    }

    return (0);
}
