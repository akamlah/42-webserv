#ifndef WS_POLL_DESCRIPTORS
# define WS_POLL_DESCRIPTORS

#include <poll.h>
#include <vector>

namespace ws {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// POLL DESCRIPTOR POOL
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class PollDescriptors {

    private:

        struct c_fd {
            struct pollfd _c_fd;
            c_fd(int fd, int events) { _c_fd.fd = fd; _c_fd.events = events; _c_fd.revents = 0; }
        };

        std::vector<c_fd> _c_poll_fds;

    public:

        PollDescriptors();
        PollDescriptors(const PollDescriptors& other);
        PollDescriptors& operator=(const PollDescriptors& other);
        ~PollDescriptors();

        size_t size() const;
        
        struct pollfd& operator[](int id);
        const struct pollfd& operator[](int id) const;

        int poll(int timeout);

        void add_descriptor(int fd, short events);
        void mark_to_cancel(int index);
        void compress();

};

} // namespace ws
#endif // WS_POLL_DESCRIPTORS
