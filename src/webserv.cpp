#include "../include/Server.hpp"
#include "../include/Socket.hpp"
#include "../include/utility.hpp"
#include "../include/Config.hpp"

#define PORT 8001
#define BACKLOG 100

int main(int argc, char **argv) {

    if (argc != 2)
    {
        std::cout << "Wrong number of argument!.. try: ./webserv [configuration file]\n";
        return (0);
    }
    try
    {
        ws::Config configData(argv[1]);
        configData.setConfigData();

        // std::cout << "port in 0: " << configData.getAllConfigData()[0].port << std::endl;
        // std::cout << "port in 1: " << configData.getAllConfigData()[1].port << std::endl;
        // std::cout << "port in 2: " << configData.getAllConfigData()[2].port << std::endl;
        std::cout << "number of server: " << configData.getnumberOfServers() <<  std::endl;

        // ws::Socket server_socket(AF_INET6, SOCK_STREAM, 0); // these might better be server parameters ?
        // ws::Socket server_socket1(AF_INET6, SOCK_STREAM, 0, PORT); // these might better be server parameters ?

        ws::Server server(configData.getConfigData());
        // ws::Server server(server_socket1, PORT);
        try {
                server.listen(BACKLOG);
                // server1.listen(BACKLOG);
        /* also bind */ }
        catch (ws::exception& e) { std::cout << e.what() << std::endl; return (-1); }

        // all of next code in a "server run" function ?


            #if __APPLE__ //then use kqueue, else epoll ?
                // -> multiplexing -> handle inside server class ?

                try {
                     server.run(3 * 60 * 1000); 
                }
                catch (ws::exception& e) { std::cout << e.what() << std::endl; return -1; }

              #endif

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (-1);
    }
    return (0);
}
