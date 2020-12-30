#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <device/keyboard/vk_layout.h>

#ifdef __cplusplus
extern "C" {
#endif

void kb_init(void);

bool kb_is_key_down(uint8_t key);
uint8_t kb_wait_hit(void);

// Returns a valid char if the vk can be translated to char, else 0xFF + the VK code
int kb_getch(void);

char vk_to_char(uint8_t vk);

#ifdef __cplusplus
}
#endif