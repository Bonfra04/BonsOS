#include <drivers/usb/usb.h>
#include <stdalign.h>

#include <log.h>

usb_transfer_status_t usb_set_address(const usb_bus_t* bus, uint64_t addr)
{
    alignas(0x20) usb_request_packet_t setup;
    setup.type = USB_REQUEST_DIR_HOST_TO_DEVICE | USB_REQUEST_TYPE_STANDARD;
    setup.request = USB_REQUEST_SET_ADDRESS;
    setup.value = addr;
    setup.index = 0;
    setup.size = 0;

    kernel_log("\nSetup packet(0x%p): type: %#x, request: %#x, value: %#x, index: %#x, size: %#x\n", &setup, setup.type, setup.request, setup.value, setup.index, setup.size);

    return usb_transfer_control_out(bus, 0, 0, &setup);
}

usb_transfer_status_t usb_get_standard_descriptor(const usb_device_t* dev, usb_descriptor_type_t desc_type, uint32_t desc_index, void* buffer, size_t size)
{
    usb_request_packet_t setup;
    setup.type = USB_REQUEST_DIR_DEVICE_TO_HOST | USB_REQUEST_TYPE_STANDARD;
    setup.request = USB_REQUEST_GET_DESCRIPTOR;
    setup.value = (desc_type << 8) | desc_index;
    setup.index = 0;
    setup.size = size;

    return usb_transfer_control_in(dev->bus, dev->addr, 0, &setup, buffer, setup.size);
}

size_t usb_get_string_length(const usb_device_t* dev, uint8_t index)
{
    usb_request_packet_t setup;
    setup.type = USB_REQUEST_DIR_DEVICE_TO_HOST | USB_REQUEST_TYPE_STANDARD;
    setup.request = USB_REQUEST_GET_DESCRIPTOR;
    setup.value = (USB_DESCRIPTOR_STRING << 8) | index;
    setup.index = 0;
    setup.size = 2;

    uint8_t data[2];
    usb_transfer_status_t status = usb_transfer_control_in(dev->bus, dev->addr, 0, &setup, data, setup.size);
    if(status != USB_TRANSFER_STATUS_OK)
        return 0;
    return data[0];
}

bool usb_get_string(const usb_device_t* dev, uint8_t index, char* str)
{
    size_t length = usb_get_string_length(dev, index);
    if(length == 0)
        return false;

    usb_request_packet_t setup;
    setup.type = USB_REQUEST_DIR_DEVICE_TO_HOST | USB_REQUEST_TYPE_STANDARD;
    setup.request = USB_REQUEST_GET_DESCRIPTOR;
    setup.value = (USB_DESCRIPTOR_STRING << 8) | index;
    setup.index = 0;
    setup.size = length;

    usb_transfer_status_t status = usb_transfer_control_in(dev->bus, dev->addr, 0, &setup, str, setup.size);
    return status == USB_TRANSFER_STATUS_OK;    
}

usb_transfer_status_t usb_set_configuration(const usb_device_t* dev, uint8_t config)
{
    usb_request_packet_t setup;
    setup.type = USB_REQUEST_DIR_HOST_TO_DEVICE | USB_REQUEST_TYPE_STANDARD;
    setup.request = USB_REQUEST_SET_CONFIGURATION;
    setup.value = config;
    setup.index = 0;
    setup.size = 0;

    return usb_transfer_control_out(dev->bus, dev->addr, 0, &setup);
}

usb_transfer_status_t usb_set_interface(const usb_device_t* dev, uint8_t interface)
{
    usb_request_packet_t setup;
    setup.type = USB_REQUEST_DIR_HOST_TO_DEVICE | USB_REQUEST_TYPE_STANDARD | USB_REQUEST_RECP_INTERFACE;
    setup.request = USB_REQUEST_SET_INTERFACE;
    setup.value = interface;
    setup.index = 0;
    setup.size = 0;

    return usb_transfer_control_out(dev->bus, dev->addr, 0, &setup);
}
