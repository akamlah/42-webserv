#include "../include/Poll.hpp"

namespace ws {

const char* Poll::PollError::what() const throw() {
    return ("Poll error");
}

Pollfd::Pollfd(int fd, short events, short revents)
{
    elem.fd = fd;
    elem.events = events;
    elem.revents = revents;
}

Poll::Poll()
{

}

Poll::~Poll()
{

}

void Poll::set_timeout(int timeout_in_ms) { _timeout = timeout_in_ms; }


void Poll::add_to_poll(int fd, short events, short revents)
{
    _fds.push_back(new_Pollfd(fd, events, revents));
}



void Poll::poll()
{
    int temp = 0;

    temp = ::poll(&(_fds[0].elem), _fds.size(), _timeout);
    if (temp < 0)
        throw_print_error(PollError(), "poll() failed");
    if (temp == 0)
        throw_print_error(PollError(), "poll() timed out.  End program.");
}

void Poll::compress_array()
{
    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].elem.fd == -1)
        {
            _fds.erase(_fds.begin() + i);
            i--;
        }
    }
}

void Poll::close_all()
{
    for (size_t i = 0; i < _fds.size() ; i++)
    {
        if (_fds[i].elem.fd >= 0)
            close(_fds[i].elem.fd);
    }
}

} // NAMESPACE ws
