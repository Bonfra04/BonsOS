#pragma once

#include "../lib/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t inportb(uint16_t port);
void outportb(uint16_t port, uint8_t value);

#ifdef __cplusplus
}
#endif