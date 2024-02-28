#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdio.h> // for puts()
#include <stdarg.h> // for va_args

#include "../config/config.h"


#ifdef LOG_TO_PRINTF
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

#ifdef TRACE_TO_PRINTF
    #define TRACE(...) trace_impl(__func__, __VA_ARGS__)
#else
    #define TRACE(...)
#endif


// A trick to add quotes to the content of a preprocessor macro (converts it into a string literal)
#define Q(x) #x
#define QUOTE(x) Q(x)

#define ERROR(condition)  report_error_impl(condition, "ERROR detected: (" #condition ") in " __FILE__ ":" QUOTE(__LINE__))


// TODO: Add toggles for log/abort/nothing
inline
bool report_error_impl(bool condition, const char* message)
{
    if (condition)
    {
        puts(message);
        fflush(stdout);
    }

    return condition;
}

inline
void trace_impl(const char* func_name, const char* message, ...)
{
        printf("%s(): ", func_name);

        va_list args;
        va_start (args, message);
        vprintf(message, args);
        va_end (args);

        printf("\n");
        fflush(stdout);
}


#endif /* UTIL_H */

