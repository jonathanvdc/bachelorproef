#ifndef HANDLE_SIGNALS
// Handle signals unless otherwise specified. This is especially useful for debugging
// segfaults that occur on a remote machine.
#define HANDLE_SIGNALS 1
#endif

#include <atomic>
#include "util/ExternalVars.h"
std::atomic<bool> stride::util::INTERRUPT(false);

#if HANDLE_SIGNALS

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace {
void handle_segfault(int sig)
{
	// This signal handler is based on the signal handler from this StackOverflow answer:
	//
	// http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
	//
	// by Todd Gamblin, edited by Violet Giraffe.

	void* array[50];
	size_t size;

	// Get void*s for all entries on the stack.
	size = backtrace(array, 50);

	// Print out all the frames to stderr.
	if (sig == SIGSEGV) {
		// This branch should always be taken.
		fprintf(stderr, "error: segmentation fault\n");
	} else {
		fprintf(stderr, "error: signal %d:\n", sig);
	}
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

void handle_interrupt(int sig) { stride::util::INTERRUPT.store(true); }
}

namespace stride {
namespace util {
void setup_segfault_handler() { signal(SIGSEGV, handle_segfault); }
void setup_interrupt_handler() { signal(SIGINT, handle_interrupt); }
}
}

#else

namespace stride {
namespace util {
void setup_segfault_handler() {}
void setup_interrupt_handler() {}
}
}

#endif