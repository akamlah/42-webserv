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

class KQueue {

public:
    int kevent ()
    {
        _triggered_events = ::kevent(_kq, _changelist, _nchanges, _eventlist, _nevents, _timeout);
        if (_triggered_events <= 0)
            throw std::invalid_argument( "\n kevent() error \n" );
    }
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