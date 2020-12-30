#include <device/disk.h>
#include <panic.h>

extern void ata_lba_read(uint64_t addr, uint32_t lba, uint16_t amount);

void read_disk(uint64_t address, uint32_t lba, uint16_t amount, disk_type_t diskType)
{
    switch (diskType)
    {
    case HARD_DISK:
        ata_lba_read(address, lba, amount);
        break;

    default:
        kenrel_panic("Unknown disk type: 0x%X", diskType);
    }
}