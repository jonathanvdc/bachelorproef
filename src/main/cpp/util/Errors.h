#ifndef STRIDE_ERRORS_H_INCLUDED
#define STRIDE_ERRORS_H_INCLUDED

#include <exception>
#include <string>

#define FATAL_ERROR(message) throw runtime_error(std::string(__func__) + "> " + (message))

#endif // end-of-include-guard
