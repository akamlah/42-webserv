#ifndef POLL_HPP
#define POLL_HPP

#include "utility.hpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <vector>

namespace ws {

typedef struct address_struct
{
    struct sockaddr_in6 _address;
    address_struct()
    {
        memset(&_address, 0, sizeof(_address));
        memcpy(&_address.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    }
}s_address;

typedef struct Pollfd
{
    struct pollfd elem;
    Pollfd(int fd = 0, short events = 0, short revents = 0);
}new_Pollfd;

class Poll
{
public:
    std::vector<struct Pollfd>  fds;
    int                         timeout;
    bool                        compress_array;

public:
    class PollError: public ws::exception {
        virtual const char* what() const throw();
    };
    Poll();
    ~Poll();
    void set_timeout(int ts);
    void add_to_poll(int fd = 0, short events = 0, short revents = 0);
    void poll() ;
    void compress();
    void close_all();
};

} // NAMESPACE ws

#endif
