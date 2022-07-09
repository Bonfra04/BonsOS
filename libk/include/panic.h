#pragma once

void __attribute__((noreturn)) __kernel_panic(const char* format, ...);

#define kernel_panic(format, ...) __kernel_panic("%s:%d " format, __FILE__, __LINE__, ##__VA_ARGS__)
