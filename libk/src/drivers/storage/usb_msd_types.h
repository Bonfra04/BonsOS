#pragma once

#include <drivers/usb/usb.h>

typedef struct usb_msd_device
{
    usb_device_t* device;
    usb_endpoint_t* endpoint_in;
    usb_endpoint_t* endpoint_out;
    uint32_t tag;
} usb_msd_device_t;

typedef struct usb_msd_cbw
{
    uint32_t signature;
    uint32_t tag;
    uint32_t transfer_length;
    uint8_t flags;
    uint8_t lun;
    uint8_t command_length;
    uint8_t scsi_command[16];
} __attribute__((packed)) usb_msd_cbw_t;

typedef struct usb_msd_csw
{
    uint32_t signature;
    uint32_t tag;
    uint32_t residue;
    uint8_t status;
} __attribute__((packed)) usb_msd_csw_t;
