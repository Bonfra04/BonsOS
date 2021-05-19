#pragma once

#include <stdint.h>
#include <stddef.h>

void pfa_init_region(uint64_t base_address, uint64_t region_length);
void pfa_deinit_region(uint64_t base_address, uint64_t region_length);

void* pfa_alloc_page();
void pfa_free_page(void* page);

void* pfa_alloc_pages(size_t size);
void pfa_free_pages(void* page, size_t size);

void pfa_init(void* bitmap_addr);

uint64_t pfa_get_bitmap_size();

uint64_t pfa_get_free_pages();
uint64_t pfa_page_size();