// Boost has a custom assert macro, but it doesn't use printf
// formatting (I guess you are supposed to use boost::format).  Its
// stacktrace functionality is much more useful than straight
// backtrace(3), which doesn't demangle names.

#define BOOST_STACKTRACE_USE_BACKTRACE 1

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#include <boost/stacktrace.hpp>

#define CUSTOM_ASSERT(condition, ...)                                          \
    do {                                                                       \
        if (!__builtin_expect(!!(condition), 0))                               \
            custom_assert_failed(#condition, __func__, __FILE__, __LINE__,     \
                                 __VA_ARGS__);                                 \
    } while (0)

[[noreturn]] __attribute__((format(printf, 5, 6))) void
custom_assert_failed(const char *condition, const char *function,
                     const char *file, int line, const char *message, ...) {
    do {
        const std::size_t BACKTRACE_NUM_ELEM = 256;
        ::va_list ap, aq;
        ::va_start(ap, message);
        ::va_copy(aq, ap);
        std::size_t length = ::vsnprintf(nullptr, 0, message, ap) + 1;
        std::unique_ptr<char[]> buf(new char[length]);
        ::vsnprintf(buf.get(), length, message, aq);
        std::clog << file << ":" << line << " (" << function << "): Assertion `"
                  << condition << "' failed: " << buf.get() << std::endl
                  << boost::stacktrace::stacktrace();
        ::va_end(ap);
        ::va_end(aq);
    } while (0);
    ::abort();
}

int caller_a(int x);
int caller_b(int x);

int caller_a(int x) { return 1 + caller_b(x - 1); }

int caller_b(int x) {
    CUSTOM_ASSERT(x >= 0, "x must be nonnegative (got value %d)", x);
    return 1 + caller_a(x - 1);
}

int main(void) {
    caller_a(10);
    return 0;
}
