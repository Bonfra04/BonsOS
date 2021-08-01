#include "vesa.h"
#include "../lib/string.h"
#include "realmode.h"

typedef struct vbe_descriptor
{
    char signature[4];
    uint16_t version;       // VBE version; high byte is major version, low byte is minor version
    uint32_t oem;           // segment:offset pointer to OEM
    uint32_t capabilities;  // bitfield that describes card capabilities
    uint16_t modes_off;     // offset pointer to list of supported video modes
    uint16_t modes_seg;     // segment pointer to list of supported video modes
    uint16_t video_memory;  // amount of video memory in 64KB blocks
    uint16_t software_rev;  // software revision
    uint32_t vendor;        // segment:offset to card vendor string
    uint32_t product_name;  // segment:offset to card model name
    uint32_t product_rev;   // segment:offset pointer to product revision
    char reserved[222];     // reserved for future expansion
    char oem_data[256];     // OEM BIOSes store their strings in this area
} __attribute__ ((packed)) vbe_descriptor_t;

typedef struct vbe_mode_descriptor {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;         // number of bytes per horizontal line
    uint16_t width;         // width in pixels
    uint16_t height;        // height in pixels
    uint8_t w_char;
    uint8_t y_char;	
    uint8_t planes;
    uint8_t bpp;            // bits per pixel in this mode
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;   // physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__ ((packed)) vbe_mode_descriptor_t;

void vesa_enable_mode(uint16_t mode)
{
    rm_regs_t regs;
    regs.eax = 0x4F02;
    regs.ebx = mode | 0x4000; // enable LFB
    regs.edi = 0; // maybe some BIOSes need this
    rm_int(0x10, &regs, &regs);
}

bool vesa_find_mode(uint16_t width, uint16_t height, uint8_t depth, vesa_descriptor_t* vesa_desc)
{
    vbe_descriptor_t vbe_desc;
    memset(&vbe_desc, 0, sizeof(vbe_descriptor_t));
    strcpy(vbe_desc.signature, "VBE2");

    rm_regs_t rm_regs;

    // get VBE BIOS info
    rm_regs.eax = 0x4F00;
    rm_regs.edi = &vbe_desc;
    rm_int(0x10, &rm_regs, &rm_regs);

    if(rm_regs.eax != 0x004F)
        return false;

    uint16_t* modes = (uint16_t*)(vbe_desc.modes_seg * 0x10 + vbe_desc.modes_off);

    while(*modes != 0xFFFF)
    {
        vbe_mode_descriptor_t mode_desc;
        rm_regs.eax = 0x4F01;
        rm_regs.edi = &mode_desc;
        rm_regs.ecx = *modes;
        rm_int(0x10, &rm_regs, &rm_regs);

        if(rm_regs.eax != 0x4F)
            return false;

        if(mode_desc.width == width && mode_desc.height == height && mode_desc.bpp == depth)
        {
            vesa_desc->id = *modes;
            vesa_desc->width = mode_desc.width;
            vesa_desc->height = mode_desc.height;
            vesa_desc->depth = mode_desc.bpp;
            vesa_desc->pitch = mode_desc.pitch;
            vesa_desc->framebuffer = (void*)mode_desc.framebuffer;
            return true;
        }

        modes++;
    }

    return false;
}