#include "disk.h"
#include "realmode.h"
#include "../lib/string.h"

typedef struct lba_packet
{
    uint8_t packet_size;
    uint8_t reserved;
    uint16_t sectors_count;
    uint32_t buffer;
    uint64_t lba;
} __attribute__ ((packed)) lba_packet_t;

static uint8_t buffer[512];
static uint16_t position;
static uint32_t lba;
static uint8_t drive;

#define LIN_TO_FAR_ADDR(linaddr) ((((linaddr) >> 4) << 16) | ((linaddr) & 0xf))

static void read_sector(uint32_t lba, uint8_t count, void* address)
{
    lba_packet_t packet;
    packet.packet_size = 0x10;
    packet.reserved = 0;
    packet.sectors_count = count;
    packet.buffer = LIN_TO_FAR_ADDR((uint32_t)address);
    packet.lba = lba;

    rm_regs_t regs, o;
    regs.esi = (uint32_t)&packet;
    regs.eax = 0x42 << 8;
    regs.edx = drive;
    rm_int(0x13, &regs, &regs);
}

void disk_init(uint8_t _drive)
{
    drive = _drive;
    position = 0;
    lba = 0;
    read_sector(lba, 1, &buffer);
}

void disk_seek(uint32_t new_pos)
{
    uint32_t new_lba = new_pos / 512;

    if(lba != new_lba)
        read_sector(new_lba, 1, &buffer);

    lba = new_lba;
    position = new_pos % 512;
}

void disk_read(uint32_t bytes, void* data)
{
    uint16_t readable = 512 - position;
    readable = (bytes < readable) ? bytes : readable;

    memcpy(data, &buffer[position], readable);
    position += readable;

    if(position == 512)
    {
        position = 0;
        lba++;
        read_sector(lba, 1, &buffer);
        if(bytes - readable > 0)
            disk_read(bytes - readable, (uint8_t*)data + readable);
    }
}