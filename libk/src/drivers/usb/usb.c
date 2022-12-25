#include <drivers/usb/usb.h>
#include <drivers/usb/uhci/uhci.h>
#include <panic.h>

#include <containers/darray.h>
#include <string.h>
#include <stdlib.h>

static usb_bus_t* usb_busses;

void usb_init()
{
    usb_busses = darray(usb_bus_t, 0);
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

    // TODO: real hw hangs inside this function
    if(usb_set_address(bus, addr) != USB_TRANSFER_STATUS_OK)
        return;

    usb_device_t* device = calloc(sizeof(usb_device_t), 1);
    device->bus = bus;
    device->addr = addr;

    if(usb_get_standard_descriptor(device, USB_DESCRIPTOR_DEVICE, 0, &device->descriptor, sizeof(usb_device_descriptor_t)) != USB_TRANSFER_STATUS_OK)
        return;

    device->configurations = calloc(sizeof(usb_configuration_t), device->descriptor.num_configurations);

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

    bus->devices[addr - 1] = device;
}

void usb_register_hci(void* data, uint64_t num_ports, const usb_hci_driver_t* driver)
{
    darray_append(usb_busses, (usb_bus_t){});
    usb_bus_t* bus = &usb_busses[darray_length(usb_busses) - 1];
    memset(bus->devices, 0, sizeof(usb_device_t*) * USB_BUS_MAX_DEVICES);
    bus->hci = (usb_hci_t) {
        .data = data,
        .num_ports = num_ports,
        .driver = driver
    };

    for(uint64_t i = 0; i < bus->hci.num_ports; i++)
    {
        bus->hci.driver->reset_port(bus->hci.data, i);
        usb_port_status_t status = bus->hci.driver->port_status(bus->hci.data, i);
        if(status == USB_PORT_STATUS_NOT_CONNECT)
            continue;

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

usb_endpoint_t* usb_get_endpoint(const usb_device_t* device, bool in, usb_endpoint_type_t type)
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
