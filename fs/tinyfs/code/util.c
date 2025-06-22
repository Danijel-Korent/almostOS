#include "util.h"

// To create external symbol of inlined function in util.h
// Will not link without this if optimisation is turned off
bool report_error_impl(bool condition, const char* message);

void trace_impl(const char* func_name, const char* message, ...);
