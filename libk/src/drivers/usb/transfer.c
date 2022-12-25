#include <drivers/usb/usb.h>

#include <string.h>

usb_transfer_status_t usb_transfer_in(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* setup, void* payload, size_t size)
{
    size_t num_packets = 2 + size / 8 + (size % 8 != 0);
    usb_packet_t packets[num_packets];
    memset(packets, 0, sizeof(usb_packet_t) * num_packets);

    packets[0].type = USB_PACKET_TYPE_SETUP;
    packets[0].maxlen = 8;
    packets[0].buffer = setup;

    int toggle = 1;
    int pid = 1;
    for(int i = 0; i < size; i += 8)
    {
        packets[pid].type = USB_PACKET_TYPE_IN;
        packets[pid].maxlen = 8;
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

usb_transfer_status_t usb_transfer_out(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* setup)
{
    usb_packet_t packets[2];
    memset(packets, 0, sizeof(usb_packet_t) * 2);

    packets[0].type = USB_PACKET_TYPE_SETUP;
    packets[0].maxlen = 8;
    packets[0].buffer = setup;
    
    packets[1].type = USB_PACKET_TYPE_IN;
    packets[1].maxlen = 0x800;
    packets[1].buffer = NULL;
    packets[1].toggle = 1;

    return bus->hci.driver->transfer_packets(bus->hci.data, addr, endpoint, packets, 2);
}

#include <log.h>

static void dump_packet(uint8_t* packet, size_t size)
{
    int k = 0;
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            for(int l = 0; l < 8; l++, k++)
                kernel_log("%02x ", packet[k]);
            kernel_log("\b-");
        }
        kernel_log("\b\n");
    }
}

usb_transfer_status_t usb_transfer_bulk_out(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* payload, size_t size)
{
    size_t num_packets = size / 64 + (size % 64 != 0);
    usb_packet_t packets[num_packets];
    memset(packets, 0, sizeof(usb_packet_t) * num_packets);

    int toggle = 0;
    int pid = 0;
    for(int i = 0; i < size; i += 64)
    {
        packets[pid].type = USB_PACKET_TYPE_OUT;
        packets[pid].maxlen = 64;
        packets[pid].buffer = payload + i;
        packets[pid].toggle = toggle;
        toggle = !toggle;
        dump_packet(packets[pid].buffer, packets[pid].maxlen);
        pid++;
    }

    return bus->hci.driver->transfer_packets(bus->hci.data, addr, endpoint, packets, num_packets);
}

usb_transfer_status_t usb_transfer_bulk_in(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* payload, size_t size)
{
    size_t num_packets = size / 64 + (size % 64 != 0);
    usb_packet_t packets[num_packets];
    memset(packets, 0, sizeof(usb_packet_t) * num_packets);

    int toggle = 1;
    int pid = 0;
    for(int i = 0; i < size; i += 64)
    {
        packets[pid].type = USB_PACKET_TYPE_IN;
        packets[pid].maxlen = 64;
        packets[pid].buffer = payload + i;
        packets[pid].toggle = toggle;
        toggle = !toggle;
        pid++;
    }

    return bus->hci.driver->transfer_packets(bus->hci.data, addr, endpoint, packets, num_packets);
}