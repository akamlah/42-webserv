#include "../include/Kqueue.hpp"

namespace ws {

int Kqueue::kevent ()
{
    _triggered_events = ::kevent(_kq, _changelist, _nchanges, _eventlist, _nevents, _timeout);
    if (_triggered_events <= 0)
        throw std::invalid_argument( "\n kevent() error \n" );
    return _triggered_events;
}

} // NAMESPACE ws
