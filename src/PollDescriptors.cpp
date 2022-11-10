#include "../include/PollDescriptors.hpp"

namespace ws {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// poll descriptor pool
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// construction/destruction/copy

PollDescriptors::PollDescriptors() {}

PollDescriptors::PollDescriptors(const PollDescriptors& other)
 : _c_poll_fds(other._c_poll_fds) {}

PollDescriptors& PollDescriptors::operator=(const PollDescriptors& other) {
    _c_poll_fds = other._c_poll_fds;
    return (*this);
}

PollDescriptors::~PollDescriptors() {}


// accessors

size_t PollDescriptors::size() const
    { return(_c_poll_fds.size()); }

struct pollfd& PollDescriptors::operator[](int id) 
    { return(_c_poll_fds[id]._c_fd); }

const struct pollfd& PollDescriptors::operator[](int id) const
    { return(_c_poll_fds[id]._c_fd); }


void PollDescriptors::add_descriptor(int fd, short events) {
    _c_poll_fds.push_back(c_fd(fd, events));
}


// core member functions

// fd has to be closed separately though
void PollDescriptors::mark_to_cancel(int index) {
    (*this)[index].fd = -1;
}

void PollDescriptors::compress() {
    for (size_t i = 0; i < size(); i++) {
        if (_c_poll_fds[i]._c_fd.fd == -1) {
            _c_poll_fds.erase(_c_poll_fds.begin() + i);
            i--;
        }
    }
}

// error handeling is up to user of this function, same as in c poll()
int PollDescriptors::poll(int timeout) {
    return (::poll(std::addressof(_c_poll_fds.data()->_c_fd), size(), timeout));
}

} // namespace ws
