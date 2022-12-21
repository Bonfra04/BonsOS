#pragma once

#define IO_USBCMD    0x00
#define IO_USBSTS    0x02
#define IO_USBINTR   0x04
#define IO_FRNUM     0x06
#define IO_FRBASEADD 0x08
#define IO_SOFMOD    0x0C
#define IO_PORTSC    0x10
#define IO_PORTSC1   (IO_PORTSC + 0)
#define IO_PORTSC2   (IO_PORTSC + 2)

#define USBCMD_MAXPACK (1 <<7)
#define USBCMD_CF      (1 << 6)
#define USBCMD_GRESET  (1 << 2)
#define USBCMD_HCRESET (1 << 1)
#define USBCMD_RUNSTOP (1 << 0)

#define USBSTS_HALTED  (1 << 5)
#define USBSTS_PRC_ERR (1 << 4)
#define USBSTS_SYS_ERR (1 << 3)
#define USBSTS_RES_DET (1 << 2)
#define USBSTS_ERR_INT (1 << 1)
#define USBSTS_INT     (1 << 0)

#define PORTSC_CONNECT_STATUS   (1 << 0)
#define PORTSC_STATUS_CHANGE    (1 << 1)
#define PORTSC_ENABLE           (1 << 2)
#define PORTSC_ENABLE_CHANGE    (1 << 3)
#define PORTSC_LOW_SPEED        (1 << 8)
#define PORTSC_PORT_RESET       (1 << 9)

#define USBINTR_TIMEOUT (1 << 0)
#define USBINTR_RESUME  (1 << 1)
#define USBINTR_IOC     (1 << 2)
#define USBINTR_SP      (1 << 3)

#define FRAMELIST_TERMINATE (1 << 0)
#define FRAMELIST_QH        (1 << 1)

#define SOFMOD_64 0x40

#define TD_TERMINATE        (1 << 0)
#define TD_STATUS_ACTIVE    (1 << 23)
#define TD_IOC              (1 << 24)
#define TD_LOW_SPEED        (1 << 26)
#define TD_PID_SETUP        0x2d
#define TD_PID_IN           0x69
#define TD_PID_OUT          0xe1
#define TD_DATA_TOGGLE      (1 << 19)

#define QH_TERMINATE        (1 << 0)

