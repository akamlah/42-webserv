/* ************************************************************************ */
/*                                                                          */
/*                              Class: KQueue                               */
/*                                                                          */
/* ************************************************************************ */

#ifndef __KQUEUE_HPP__
# define __KQUEUE_HPP__

#include "Socket.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "utility.hpp"

#include <arpa/inet.h> // htons etc
#include <string.h> // bezero
#include <iostream>
#include <sys/event.h> // kqueue

namespace ws {

class Kqueue {

public:
    int kevent ();
private:
    int _kq;
    int _nchanges;
    int _nevents;
    const struct kevent *_changelist;
    struct kevent *_eventlist;
    const struct timespec *_timeout;
    int _triggered_events;
};

}

#endif
