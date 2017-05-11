#ifndef SIGNALS_H_INCLUDED
#define SIGNALS_H_INCLUDED

namespace stride {
namespace util {

/// Sets up a signal handler that handles SIGSEGV signals by printing
/// stack traces and exiting. This does nothing if HANDLE_SIGNALS is
/// set to zero.
void setup_segfault_handler();
}
}

#endif