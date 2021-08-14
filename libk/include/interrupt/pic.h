#pragma once

#define PIC_CMD_MASTER  0x20
#define PIC_CMD_SLAVE   0xA0
#define PIC_DATA_MASTER 0x21
#define PIC_DATA_SLAVE  0xA1
#define PIC_CMD_EOI     0x20

void pic_init();
void pic_disable();
