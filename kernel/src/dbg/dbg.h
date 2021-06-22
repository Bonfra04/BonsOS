#pragma once

#define dbg_break() asm volatile("int 3");

void dbg_init();