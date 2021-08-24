#pragma once

#include <stdint.h>
#include <interrupt/interrupt.h>

void pit_prepare_one_shot(uint32_t frequency);
void pit_wait_one_shot();
