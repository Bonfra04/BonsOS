#pragma once

#ifdef NDEBUG
    #define kernel_trace(format, ...) ((void)0)
    #define kernel_warn(format, ...) ((void)0)
#else

#define kernel_trace(format, ...) __kernel_log("[Trace] " format "\n", ##__VA_ARGS__)
#define kernel_warn(format, ...) __kernel_log("[Warn] %s:%d: " format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#endif

#define kernel_log(format, ...) __kernel_log(format, ##__VA_ARGS__)
void __kernel_log(const char* format, ...);
