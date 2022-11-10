#include "../include/Server.hpp"

namespace ws {

/*
    Member type Server: (in Server.hpp) '_sig_func', defined as:
        typedef void (*_sig_func)(int);
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CLASS Server: signals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Server::_sig_func Server::signal(int signo, Server::_sig_func func) {
    WS_events_debug("Setting up signal " << signo);
    struct sigaction action, oact;
    action.sa_handler = func;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (signo == SIGALRM) {
        #ifdef SA_INTERRUPT
            action.sa_flags |= SA_INTERRUPT;
        #endif
    }
    else {
        #ifdef SA_RESTART
            action.sa_flags |= SA_RESTART;
        #endif
    }
    if (sigaction(signo, &action, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

void Server::sigint(int signo) {
    WS_events_debug(" SIGINT");
    (void)signo;
    exit(0);
}

void Server::sigquit(int signo) {
    WS_events_debug(" SIGQUIT");
    (void)signo;
    exit(0);
}

void Server::sigterm(int signo) {
    WS_events_debug(" SIGTERM");
    (void)signo;
    exit(0);
}

void Server::sigpipe(int signo) {
    WS_events_debug(" SIGPIPE");
    (void)signo;
}

} // namespace ws
