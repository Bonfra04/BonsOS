#include <drivers/usb/usb.h>

#include <string.h>
#include <stdalign.h>

size_t packet_size(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint)
{
    usb_device_t* dev = usb_get_device(bus, addr);
    if(dev == NULL)
    {
        usb_port_status_t status = bus->hci.driver->port_status(bus->hci.data, addr);
        return status == USB_PORT_STATUS_CONNECT_FULL_SPEED ? 64 : status == USB_PORT_STATUS_CONNECT_LOW_SPEED ? 8 : 8;
    }
    else
    {
        usb_endpoint_t* ep = usb_get_endpoint(dev, endpoint);
        if(ep == NULL)
        {
            return dev->descriptor.max_packet_size;
        }
        else
        {
            usb_endpoint_descriptor_t* epd = &usb_get_endpoint(dev, endpoint)->descriptor;
            size_t pksiz = epd->max_packet_size;
            return pksiz;
        }
    }
}

usb_transfer_status_t usb_transfer_control_in(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* setup, void* payload, size_t size)
{
    size_t pksiz = packet_size(bus, addr, endpoint);
    size_t num_packets = 2 + size / pksiz + (size % pksiz != 0);
    usb_packet_t packets[num_packets];
    memset(packets, 0, sizeof(usb_packet_t) * num_packets);

    packets[0].type = USB_PACKET_TYPE_SETUP;
    packets[0].maxlen = 8;
    packets[0].buffer = setup;
    packets[0].toggle = 0;

    int toggle = 1;
    int pid = 1;
    for(int i = 0; i < size; i += pksiz)
    {
        packets[pid].type = USB_PACKET_TYPE_IN;
        packets[pid].maxlen = pksiz;
        packets[pid].buffer = payload + i;
        packets[pid].toggle = toggle;
        toggle = !toggle;
        pid++;
    }

    packets[pid].type = USB_PACKET_TYPE_OUT;
    packets[pid].maxlen = 0x800;
    packets[pid].buffer = NULL;
    packets[pid].toggle = 1;
    
    return bus->hci.driver->transfer_packets(bus->hci.data, addr, endpoint, packets, num_packets);
}

usb_transfer_status_t usb_transfer_control_out(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* setup)
{
    usb_packet_t packets[2];
    memset(packets, 0, sizeof(usb_packet_t) * 2);

    packets[0].type = USB_PACKET_TYPE_SETUP;
    packets[0].maxlen = 8;
    packets[0].buffer = setup;
    packets[0].toggle = 0;
    
    packets[1].type = USB_PACKET_TYPE_IN;
    packets[1].maxlen = 0x800;
    packets[1].buffer = NULL;
    packets[1].toggle = 1;

    return bus->hci.driver->transfer_packets(bus->hci.data, addr, endpoint, packets, 2);
}

usb_transfer_status_t usb_transfer_bulk_out(const usb_bus_t* bus, uint64_t addr, usb_endpoint_t* endpoint, void* payload, size_t size)
{
    size_t pksiz = packet_size(bus, addr, endpoint->descriptor.endpoint_number);
    size_t num_packets = size / pksiz + (size % pksiz != 0);
    usb_packet_t packets[num_packets];
    memset(packets, 0, sizeof(usb_packet_t) * num_packets);

    int pid = 0;
    while(size > 0)
    {
        packets[pid].type = USB_PACKET_TYPE_OUT;
        packets[pid].maxlen = size > pksiz ? pksiz : size;
        packets[pid].buffer = payload + pid * pksiz;
        packets[pid].toggle = endpoint->toggle;
        endpoint->toggle = !endpoint->toggle;
        pid++;
        size = size > pksiz ? size - pksiz : 0;
    }

    usb_transfer_status_t  res = bus->hci.driver->transfer_packets(bus->hci.data, addr, endpoint->descriptor.endpoint_number, packets, num_packets);
    return res;
}

usb_transfer_status_t usb_transfer_bulk_in(const usb_bus_t* bus, uint64_t addr, usb_endpoint_t* endpoint, void* payload, size_t size)
{
    size_t pksiz = packet_size(bus, addr, endpoint->descriptor.endpoint_number);
    size_t num_packets = size / pksiz + (size % pksiz != 0);
    usb_packet_t packets[num_packets];
    memset(packets, 0, sizeof(usb_packet_t) * num_packets);

    int pid = 0;
    for(int i = 0; i < size; i += pksiz)
    {
        packets[pid].type = USB_PACKET_TYPE_IN;
        packets[pid].maxlen = pksiz;
        packets[pid].buffer = payload + i;
        packets[pid].toggle = endpoint->toggle;
        endpoint->toggle = !endpoint->toggle;
        pid++;
    }

    usb_transfer_status_t res = bus->hci.driver->transfer_packets(bus->hci.data, addr, endpoint->descriptor.endpoint_number, packets, num_packets);
    return res;
}
