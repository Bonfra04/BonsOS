#pragma once

#define ALIGN_UP(addr, align) ((uint64_t)(addr) + (align) * ((uint64_t)(addr) % (align) != 0) - (uint64_t)(addr) % (align))
#define ALIGN_DOWN(addr, align) ((uint64_t)(addr) - (uint64_t)(addr) % (align))

#define ALIGN_4K_UP(addr) ALIGN_UP(addr, 0x1000)
#define ALIGN_4K_DOWN(addr) ALIGN_DOWN(addr, 0x1000)

#define ALIGN_2M_UP(addr) ALIGN_UP(addr, 0x200000)
#define ALIGN_2M_DOWN(addr) ALIGN_DOWN(addr, 0x200000)
