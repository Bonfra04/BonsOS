#include <drivers/storage/usb_msd.h>
#include <drivers/usb/usb.h>
#include <drivers/storage/scsi.h>

#include <stdlib.h>
#include <string.h>
#include <stdalign.h>

#include "usb_msd_types.h"

#define USB_CLASS_MASS_STORAGE 0x08
#define USB_SUBCLASS_MASS_STORAGE_SCSI 0x06
#define USB_PROTOCOL_MASS_STORAGE_BULK_ONLY 0x50

#define USB_REQUEST_GET_MAX_LUN 0xFE

#define CBW_DIRECTION_OUT (0 << 7)
#define CBW_DIRECTION_IN  (1 << 7)

static bool send_scsi_command(void* device, scsi_command_t* command, void* data, size_t transfer_length)
{
    usb_msd_device_t* dev = (usb_msd_device_t*)device;

    alignas(32) usb_msd_cbw_t cbw;
    memset(&cbw, 0, sizeof(usb_msd_cbw_t));
    cbw.tag = dev->tag++;
    cbw.signature = 0x43425355;

    cbw.transfer_length = transfer_length;
    cbw.flags = command->write ? CBW_DIRECTION_OUT : CBW_DIRECTION_IN;
    cbw.lun = 0;
    cbw.command_length = command->packet_len;
    memset(cbw.scsi_command, 0, 16);
    memcpy(cbw.scsi_command, command->packet, command->packet_len);

    usb_transfer_bulk_out(dev->device->bus, dev->device->addr, dev->endpoint_out, &cbw, sizeof(usb_msd_cbw_t));

    usb_endpoint_t* ep = command->write ? dev->endpoint_out : dev->endpoint_in; 
    if(command->write)
        usb_transfer_bulk_out(dev->device->bus, dev->device->addr, ep, data, transfer_length);
    else
         usb_transfer_bulk_in(dev->device->bus, dev->device->addr, ep, data, transfer_length);
    
    alignas(32) usb_msd_csw_t csw;
    memset(&csw, 0, sizeof(usb_msd_csw_t));
    usb_transfer_bulk_in(dev->device->bus, dev->device->addr, dev->endpoint_in, &csw, sizeof(usb_msd_csw_t));

    return csw.signature == 0x53425355 && csw.status == 0;
}

static uint8_t get_lun_count(usb_msd_device_t* msd_device)
{
    usb_device_t* usb_dev = msd_device->device;
    usb_request_packet_t setup;
    setup.type = USB_REQUEST_DIR_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_REQUEST_RECP_INTERFACE;
    setup.request = USB_REQUEST_GET_MAX_LUN;
    setup.value = 0;
    setup.index = usb_dev->configurations[usb_dev->curr_configuration].interfaces[usb_dev->curr_interface].descriptor.interface_number;
    setup.size = 1;

    uint8_t data[1];
    usb_transfer_status_t status = usb_transfer_control_in(usb_dev->bus, usb_dev->addr, 0, &setup, data, setup.size);
    if(status != USB_TRANSFER_STATUS_OK)
        return 0;
    return data[0];
}

static void init_device(usb_device_t* device)
{
    usb_msd_device_t* msd_device = malloc(sizeof(usb_msd_device_t));
    msd_device->device = device;

    msd_device->endpoint_in = usb_find_endpoint(device, true, USB_ENDPOINT_BULK);
    msd_device->endpoint_out = usb_find_endpoint(device, false, USB_ENDPOINT_BULK);

    // TODO: support multiple luns
    uint8_t lun_count = get_lun_count(msd_device); 

    scsi_driver_t scsi_driver;
    scsi_driver.data = msd_device;
    scsi_driver.max_packet_size = 16;
    scsi_driver.send_scsi_cmd = send_scsi_command;

    scsi_register_device(scsi_driver);
}

static usb_driver_t driver = {
    .match = USB_DRIVER_MATCH_CLASS | USB_DRIVER_MATCH_SUBCLASS | USB_DRIVER_MATCH_PROTOCOL,
    .class = USB_CLASS_MASS_STORAGE,
    .subclass = USB_SUBCLASS_MASS_STORAGE_SCSI,
    .protocol = USB_PROTOCOL_MASS_STORAGE_BULK_ONLY,
    .init = init_device,
};

void usb_msd_init()
{
    usb_register_driver(&driver);
}
