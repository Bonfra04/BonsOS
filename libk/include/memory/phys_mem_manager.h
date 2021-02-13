#pragma once

#include <stdint.h>
#include <stddef.h>

void pmm_init_region(uint64_t base_address, uint64_t region_length);
void pmm_deinit_region(uint64_t base_address, uint64_t region_length);

void* pmm_alloc_block();
void pmm_free_block(void* block);

void* pmm_alloc_blocks(size_t size);
void pmm_free_blocks(void* block, size_t size);

void pmm_init(void* bitmap_addr);

uint64_t pmm_get_bitmap_size();

uint64_t pmm_get_free_blocks();
uint64_t pmm_get_block_size();