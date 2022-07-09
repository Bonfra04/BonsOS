#pragma once

#include <stdint.h>

#define FIXED_POW 10
static const uint64_t fixed_power[FIXED_POW] = { 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 10000000000 };

typedef int64_t fixed64_t;

static inline fixed64_t fixed64_make(int32_t integer, int32_t fraction, uint8_t decimal_offset)
{
    register fixed64_t integer_part = ((int64_t)integer & UINT32_MAX) << 32;
    register fixed64_t udecimal_part = ((uint64_t)fraction << 32) / fixed_power[decimal_offset >= FIXED_POW ? FIXED_POW - 1 : decimal_offset];
    register fixed64_t decimal_part = (integer >= 0 ? +1 : -1) * (udecimal_part & UINT32_MAX);
    return integer_part + decimal_part;
}

static inline fixed64_t fixed64div(fixed64_t a, fixed64_t b)
{
    return ((__int128_t)a << 32) / b;
}

static inline fixed64_t fixed64mul(fixed64_t a, fixed64_t b)
{
    return ((__int128_t)a * b) >> 32;
}

static inline int64_t fixed64_to_int(fixed64_t a)
{
    return (int64_t)a >> 32;
}
