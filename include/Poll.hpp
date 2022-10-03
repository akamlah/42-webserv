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

typedef struct Pollfd
{
    struct pollfd elem;
    Pollfd(int fd = 0, short events = 0, short revents = 0);
}new_Pollfd;

struct Poll
{
    std::vector<struct Pollfd>  fds;
    int                         timeout;
    bool                        compress_array;

    class PollError: public ws::exception {
        virtual const char* what() const throw();
    };
    Poll();
    ~Poll();
    void set_timeout(int ts);
    int  get_fd(int index) const;
    void add_to_poll(int fd = 0, short events = 0, short revents = 0);
    void poll() ;
    void compress();
    void close_all();
};

} // NAMESPACE ws

#endif
