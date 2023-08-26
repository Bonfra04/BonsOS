#include <drivers/usb/usb.h>
#include <drivers/usb/uhci/uhci.h>
#include <drivers/usb/ehci/ehci.h>
#include <panic.h>

#include <containers/darray.h>
#include <string.h>
#include <stdlib.h>

#include <memory/pfa.h>
#include <log.h>

static usb_bus_t* usb_busses;

void usb_init()
{
    usb_busses = darray(usb_bus_t, 0);
    ehci_init();
    uchi_init();
}

static uint64_t alloc_address(usb_bus_t* bus)
{
    for(uint64_t i = 0; i < USB_BUS_MAX_DEVICES; i++)
        if(bus->devices[i] == NULL)
            return i + 1;
    return 0;
}

static void usb_register_device(usb_bus_t* bus)
{
    uint64_t addr = alloc_address(bus);
    if(addr == 0)
        kernel_panic("Too many usb devices on the same bus");

    usb_device_t* device = calloc(sizeof(usb_device_t), 1);
    device->bus = bus;
    device->addr = addr;
    bus->devices[addr - 1] = device;

    kernel_trace("Getting USB standard descriptor");
    if(usb_get_standard_descriptor(&(usb_device_t){.bus = bus, .addr = 0}, USB_DESCRIPTOR_DEVICE, 0, &device->descriptor, sizeof(usb_device_descriptor_t)) != USB_TRANSFER_STATUS_OK)
        return;

    kernel_trace("Registering USB device at address %d", addr);
    if(usb_set_address(bus, addr) != USB_TRANSFER_STATUS_OK)
        return;

    device->configurations = calloc(sizeof(usb_configuration_t), device->descriptor.num_configurations);

    kernel_trace("Reading USB configurations");
    for(size_t i = 0; i < device->descriptor.num_configurations; i++)
    {
        usb_configuration_descriptor_t* config_desc = &device->configurations[i].descriptor;
        usb_get_standard_descriptor(device, USB_DESCRIPTOR_CONFIGURATION, 0, config_desc, sizeof(usb_configuration_descriptor_t));
    
        uint8_t buff[config_desc->total_length];
        usb_get_standard_descriptor(device, USB_DESCRIPTOR_CONFIGURATION, 0, buff, config_desc->total_length);
        uint8_t* ptr = buff + sizeof(usb_configuration_descriptor_t);
        
        device->configurations[i].interfaces = darray(usb_interface_t, 0);
        usb_interface_t* curr_int = NULL;
        for(; ptr < buff + config_desc->total_length; ptr = ptr + ((usb_generic_descriptor_t*)ptr)->length)
        {
            switch (((usb_generic_descriptor_t*)ptr)->type)
            {
            case USB_DESCRIPTOR_INTERFACE:
            {
                if(curr_int != NULL)
                    darray_pack(curr_int->endpoints);
                usb_interface_t interface = {
                    .descriptor = *(usb_interface_descriptor_t*)ptr,
                    .endpoints = darray(usb_endpoint_t, 0)
                };
                darray_append(device->configurations[i].interfaces, interface);
                curr_int = &device->configurations[i].interfaces[darray_length(device->configurations[i].interfaces) - 1];
            }    
            break;

            case USB_DESCRIPTOR_ENDPOINT:
            {
                usb_endpoint_t endpoint = {
                    .descriptor = *(usb_endpoint_descriptor_t*)ptr
                };
                darray_append(curr_int->endpoints, endpoint);
            }
            break;
            }
        }

        curr_int->num_endpoints = darray_length(curr_int->endpoints);
        darray_pack(curr_int->endpoints);
        device->configurations[i].num_inferfaces = darray_length(device->configurations[i].interfaces);
        darray_pack(device->configurations[i].interfaces);
    }
}

extern size_t packet_size(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint);

void usb_register_hci(void* data, uint64_t num_ports, const usb_hci_driver_t* driver)
{
    kernel_trace("Registering USB host controller");

    darray_append(usb_busses, (usb_bus_t){});
    usb_bus_t* bus = &usb_busses[darray_length(usb_busses) - 1];
    memset(bus->devices, 0, sizeof(usb_device_t*) * USB_BUS_MAX_DEVICES);
    bus->hci = (usb_hci_t) {
        .data = data,
        .num_ports = num_ports,
        .driver = driver
    };

    kernel_trace("Probing USB ports");
    for(uint64_t i = 0; i < bus->hci.num_ports; i++)
    {
        if(!bus->hci.driver->reset_port(bus->hci.data, i))
            continue;
        if(bus->hci.driver->port_status(bus->hci.data, i) == USB_PORT_STATUS_NOT_CONNECT)
            continue;

        kernel_trace("Found connected USB device");

        uint8_t* eight = pfa_alloc(1);
        if(usb_get_standard_descriptor(&(usb_device_t){.bus = bus, .addr = 0}, USB_DESCRIPTOR_DEVICE, 0, eight, packet_size(bus, 0, 0)) != USB_TRANSFER_STATUS_OK)
            return;

        kernel_trace("Got first descriptor");

        if(!bus->hci.driver->reset_port(bus->hci.data, i))
            continue;
        if(bus->hci.driver->port_status(bus->hci.data, i) == USB_PORT_STATUS_NOT_CONNECT)
            continue;

        kernel_trace("Registering");
        usb_register_device(bus);
    }
}

void usb_register_driver(const usb_driver_t* driver)
{
    for(uint64_t i = 0; i < darray_length(usb_busses); i++)
    {
        usb_bus_t* bus = &usb_busses[i];
        for(uint64_t j = 0; j < USB_BUS_MAX_DEVICES; j++)
        {
            usb_device_t* device = bus->devices[j];
            if(device == NULL)
                continue;

            for(uint64_t k = 0; k < device->descriptor.num_configurations; k++)
            {
                usb_configuration_t* config = &device->configurations[k];
                for(uint64_t l = 0; l < config->num_inferfaces; l++)
                {
                    usb_interface_descriptor_t* interface = &config->interfaces[l].descriptor;
                    
                    if(driver->match & USB_DRIVER_MATCH_CLASS && interface->class != driver->class)
                        continue;
                    if(driver->match & USB_DRIVER_MATCH_SUBCLASS && interface->subclass != driver->subclass)
                        continue;
                    if(driver->match & USB_DRIVER_MATCH_PROTOCOL && interface->protocol != driver->protocol)
                        continue;

                    usb_set_configuration(device, device->configurations[k].descriptor.configuration_value);
                    usb_set_interface(device, device->configurations[k].interfaces[l].descriptor.interface_number);
                    device->curr_configuration = k;
                    device->curr_interface = l;

                    driver->init(device);
                    goto next;
                }
            }
            next:{}
        }
    }
}

usb_endpoint_t* usb_find_endpoint(const usb_device_t* device, bool in, usb_endpoint_type_t type)
{
    usb_interface_t* interface = &device->configurations[device->curr_configuration].interfaces[device->curr_interface];
    for(size_t i = 0; i < interface->num_endpoints; i++)
    {
        usb_endpoint_t* endpoint = &interface->endpoints[i];
        if(endpoint->descriptor.endpoint_type == type && endpoint->descriptor.direction == in)
            return endpoint;
    }
    return NULL;
}

usb_endpoint_t* usb_get_endpoint(const usb_device_t* device, uint8_t ep_number)
{
    if(device->configurations)
    {
        for(size_t i = 0; i < device->descriptor.num_configurations; i++)
        {
            usb_configuration_t* config = &device->configurations[i];
            for(size_t j = 0; j < config->num_inferfaces; j++)
            {
                usb_interface_t* interface = &config->interfaces[j];
                for(size_t k = 0; k < interface->num_endpoints; k++)
                {
                    usb_endpoint_t* endpoint = &interface->endpoints[k];
                    if(endpoint->descriptor.endpoint_number == ep_number)
                        return endpoint;
                }
            }
        }
    }

    return NULL;
}

usb_device_t* usb_get_device(const usb_bus_t* bus, uint64_t addr)
{
    for (size_t i = 0; i < USB_BUS_MAX_DEVICES; i++)
    {
        if(bus->devices[i] != NULL && bus->devices[i]->addr == addr)
            return bus->devices[i];
    }
    return NULL;
}
