#include <memory/paging_test.h>

#include <stdint.h>
typedef struct {
   uint8_t present:1;
   uint8_t readwrite:1;
   uint8_t user:1;
   uint8_t writethru:1;
   uint8_t cachedisable:1;
   uint8_t access:1;
   uint8_t reserved:3;
   uint8_t avl:3;
   uint64_t address:36;
   uint16_t ignore:15;
   uint8_t execdisable:1;
} __attribute__((packed)) pml4t_entry;

typedef struct {
   uint8_t present:1;
   uint8_t readwrite:1;
   uint8_t user:1;
   uint8_t writethru:1;
   uint8_t cachedisable:1;
   uint8_t access:1;
   uint8_t reserved:3;
   uint8_t avl:3;
   uint64_t address:36;
   uint16_t ignore:15;
   uint8_t execdisable:1;
} __attribute__((packed)) pdpt_entry;

typedef struct {
   uint8_t present:1;
   uint8_t readwrite:1;
   uint8_t user:1;
   uint8_t writethru:1;
   uint8_t cachedisable:1;
   uint8_t access:1;
   uint8_t dirty:1;
   uint8_t size:1;
   uint8_t global:1;
   uint8_t avl:3;
   uint64_t address:36;
   uint16_t ignore:15;
   uint8_t execdisable:1;
} __attribute__((packed)) page_dir_entry;

typedef struct {
   uint8_t present:1;
   uint8_t readwrite:1;
   uint8_t user:1;
   uint8_t writethru:1;
   uint8_t cachedisable:1;
   uint8_t access:1;
   uint8_t dirty:1;
   uint8_t pat:1;
   uint8_t global:1;
   uint8_t avl:3;
   uint64_t address:36;
   uint16_t ignore:11;
   uint8_t pke:4;
   uint8_t execdisable:1;
} __attribute__((packed)) page_table_entry;

pml4t_entry pml4_table[512] __attribute__((aligned(4096)));
pdpt_entry pdp_table[512] __attribute__((aligned(4096)));
page_dir_entry page_dir[512*4] __attribute__((aligned(4096)));
page_table_entry page_table[512*512*4] __attribute__((aligned(4096)));

void identity_map_everything()
{
    pml4_table[0].present = 1;
    pml4_table[0].readwrite = 1;
    pml4_table[0].user = 0; // You'll need to set this to 1 if you are using usermode
    pml4_table[0].execdisable = 0;
    pml4_table[0].address = ((intptr_t)&pdp_table[0])>>12;
    
    for (int i = 0; i < 4; i++) {
        pdp_table[0].present = 1;
        pdp_table[0].readwrite = 1;
        pdp_table[0].user = 0; // 1 if using usermode
        pdp_table[0].execdisable = 0;
        pdp_table[0].address = ((intptr_t)&page_dir[i*512])>>12;
    }

    for (int i = 0; i < 512; i++) {
        page_dir[i].present = 1;
        page_dir[i].readwrite = 1;
        page_dir[i].user = 0; // 1 if using usermode
        page_dir[i].address = ((intptr_t)&page_table[i*512])>>12;
    }

    for (uint64_t i = 0; i < 512*512; i++) {
        page_table[i].present = 1;
        page_table[i].readwrite = 1;
        page_table[i].user = 0; // 1 if using usermode
        page_table[i].address = i;
    }

    tty_printf("I got here");
   
    uint64_t new_cr3 = (uint64_t)(intptr_t)&pml4_table[0];
    asm volatile("mov cr3, %0":: "r"(new_cr3)); // (Since you said you were using Intel Syntax)

    tty_printf("It worked fine"); // Never printed
    tty_printf("Success");
}