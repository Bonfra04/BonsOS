#pragma once

#include <drivers/pci.h>

#include <stddef.h>
#include <stdbool.h>

typedef struct usb_generic_descriptor
{
    uint8_t length;
    uint8_t type;
} __attribute__((packed)) usb_generic_descriptor_t;

typedef struct usb_device_descriptor
{
    uint8_t length;
    uint8_t type;
    uint16_t usb_release_num;
    uint8_t class;
    uint8_t subclass;
    uint8_t protocol;
    uint8_t max_packet_size;
    uint16_t vendor;
    uint16_t product;
    uint16_t dev_release_num;
    uint8_t manufacturer_index;
    uint8_t product_index;
    uint8_t serial_number_index;
    uint8_t num_configurations;
} __attribute__((packed)) usb_device_descriptor_t;

typedef struct usb_configuration_descriptor
{
    uint8_t length;
    uint8_t type;
    uint16_t total_length;
    uint8_t num_interfaces;
    uint8_t configuration_value;
    uint8_t configuration_index;
    uint8_t attributes;
    uint8_t max_power;
} __attribute__((packed)) usb_configuration_descriptor_t;

typedef struct usb_interface_descriptor
{
    uint8_t length;
	uint8_t descriptor_type;
	uint8_t interface_number;
	uint8_t alternate_setting;
	uint8_t num_endpoints;
	uint8_t class;
	uint8_t subclass;
	uint8_t protocol;
	uint8_t interface_string;
} __attribute__((packed)) usb_interface_descriptor_t;

typedef struct usb_endpoint_descriptor
{
    uint8_t length;
    uint8_t descriptor_type;
    struct
    {
        uint8_t endpoint_number : 4;
        uint8_t reserved0 : 3;
        uint8_t direction : 1;
    };
    struct
    {
        uint8_t endpoint_type : 2;
        uint8_t reserved1 : 6;
    };
    uint16_t max_packet_size;
    uint8_t interval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

enum usb_standard_request
{
    USB_REQUEST_GET_STATUS = 0,
    USB_REQUEST_CLEAR_FEATURE = 1,
    USB_REQUEST_SET_FEATURE = 3,
    USB_REQUEST_SET_ADDRESS = 5,
    USB_REQUEST_GET_DESCRIPTOR = 6,
    USB_REQUEST_SET_DESCRIPTOR = 7,
    USB_REQUEST_GET_CONFIGURATION = 8,
    USB_REQUEST_SET_CONFIGURATION = 9,
    USB_REQUEST_GET_INTERFACE = 10,
    USB_REQUEST_SET_INTERFACE = 11,
    USB_REQUEST_SYNCH_FRAME = 12,
};

typedef struct usb_request_packet
{
    uint8_t type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t size;
} __attribute__((packed)) usb_request_packet_t;

#define USB_REQUEST_DIR_HOST_TO_DEVICE (0 << 7)
#define USB_REQUEST_DIR_DEVICE_TO_HOST (1 << 7)
#define USB_REQUEST_TYPE_STANDARD (0 << 5)
#define USB_REQUEST_TYPE_CLASS (1 << 5)
#define USB_REQUEST_TYPE_VENDOR (2 << 5)
#define USB_REQUEST_RECP_DEVICE (0 << 0)
#define USB_REQUEST_RECP_INTERFACE (1 << 0)
#define USB_REQUEST_RECP_ENDPOINT (2 << 0)
#define USB_REQUEST_RECP_OTHER (3 << 0)

#define PCI_CLASS_SBC 0x0C
#define PCI_SUBCLASS_USB 0x03

#define USB_PROG_UHCI 0x00
#define USB_PROG_OHCI 0x10
#define USB_PROG_EHCI 0x20
#define USB_PROG_XHCI 0x30

#define USB_BUS_MAX_DEVICES 127

typedef enum usb_port_status
{
    USB_PORT_STATUS_NOT_CONNECT,
    USB_PORT_STATUS_CONNECT_LOW_SPEED,
    USB_PORT_STATUS_CONNECT_FULL_SPEED,
} usb_port_status_t;

typedef enum usb_transfer_status
{
    USB_TRANSFER_STATUS_OK = 0,
    USB_TRANSFER_STATUS_STALL,
    USB_TRANSFER_STATUS_NAK,
    USB_TRANSFER_STATUS_CRC_ERROR,
    USB_TRANSFER_STATUS_BABBLE,
} usb_transfer_status_t;

typedef enum usb_packet_type
{
    USB_PACKET_TYPE_SETUP,
    USB_PACKET_TYPE_IN,
    USB_PACKET_TYPE_OUT,
} usb_packet_type_t;

typedef enum usb_descriptor_type
{
    USB_DESCRIPTOR_DEVICE = 1,
    USB_DESCRIPTOR_CONFIGURATION = 2,
    USB_DESCRIPTOR_STRING = 3,
    USB_DESCRIPTOR_INTERFACE = 4,
    USB_DESCRIPTOR_ENDPOINT = 5,
} usb_descriptor_type_t;

typedef struct usb_packet
{
    usb_packet_type_t type;
    uint32_t maxlen;
    bool toggle;
    void* buffer;
} usb_packet_t;

typedef struct usb_hci_driver
{
    void (*reset_port)(void* data, uint64_t port);
    usb_port_status_t (*port_status)(void* data, uint64_t port);
    usb_transfer_status_t (*transfer_packets)(void* data, uint64_t addr, uint64_t endpoint, const usb_packet_t* packets, size_t num_packets);
} usb_hci_driver_t;

typedef struct usb_hci
{
    const usb_hci_driver_t* driver;
    void* data;
    uint64_t num_ports;
} usb_hci_t;

typedef struct usb_bus usb_bus_t;

typedef struct usb_endpoint
{
    usb_endpoint_descriptor_t descriptor;
} usb_endpoint_t;

typedef struct usb_interface
{
    usb_interface_descriptor_t descriptor;
    usb_endpoint_t* endpoints;
    size_t num_endpoints;
} usb_interface_t;

typedef struct usb_configuration
{
    usb_configuration_descriptor_t descriptor;
    usb_interface_t* interfaces;
    size_t num_inferfaces;
} usb_configuration_t;

typedef struct usb_device
{
    usb_bus_t* bus;
    uint64_t addr;

    uint8_t curr_configuration, curr_interface;

    usb_device_descriptor_t descriptor;
    usb_configuration_t* configurations;
} usb_device_t;

typedef struct usb_bus
{
    usb_hci_t hci;
    usb_device_t* devices[USB_BUS_MAX_DEVICES];
} usb_bus_t;

#define USB_DRIVER_MATCH_NONE 0
#define USB_DRIVER_MATCH_CLASS 1
#define USB_DRIVER_MATCH_SUBCLASS 2
#define USB_DRIVER_MATCH_PROTOCOL 4

typedef struct usb_driver
{
    uint8_t match;
    uint8_t class, subclass, protocol;
    void (*init)(usb_device_t* device);
} usb_driver_t;

/**
 * @brief initializes usb
 */
void usb_init();

/**
 * @brief registers a host controller interface
 * @param[in] data pointer to the hci private data
 * @param[in] num_ports number of ports on the hci
 * @param[in] driver pointer to the hci driver
 */
void usb_register_hci(void* data, uint64_t num_ports, const usb_hci_driver_t* driver);

/**
 * @brief registers a usb driver
 * @param[in] driver pointer to the driver
 */
void usb_register_driver(const usb_driver_t* driver);

/**
 * @brief transfers data from a device to the host
 * @param[in] bus pointer to the bus
 * @param[in] addr address of the device
 * @param[in] endpoint endpoint of the device
 * @param[in] setup pointer to the setup packet
 * @param[in] payload pointer to the payload
 * @param[in] size size of the payload
*/
usb_transfer_status_t usb_transfer_in(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* setup, void* payload, size_t size);

/**
 * @brief transfers data from the host to a device
 * @param[in] bus pointer to the bus
 * @param[in] addr address of the device
 * @param[in] endpoint endpoint of the device
 * @param[in] setup pointer to the setup packet
*/
usb_transfer_status_t usb_transfer_out(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* setup);

/**
 * @brief sets the address of a device
 * @param[in] bus pointer to the bus
 * @param[in] addr address of the device
 * @return status of the transfer
*/
usb_transfer_status_t usb_set_address(const usb_bus_t* bus, uint64_t addr);

/**
 * @brief gets a standard descriptor from a device
 * @param[in] dev pointer to the device
 * @param[in] desc_type type of the descriptor
 * @param[in] desc_index index of the descriptor
 * @param[in] buffer pointer to the buffer
 * @param[in] size size of the buffer
 * @return status of the transfer
*/
usb_transfer_status_t usb_get_standard_descriptor(const usb_device_t* dev, usb_descriptor_type_t desc_type, uint32_t desc_index, void* buffer, size_t size);

/**
 * @brief gets the length of a string descriptor
 * @param[in] dev pointer to the device
 * @param[in] index index of the string descriptor
 * @return length of the string descriptor
*/
size_t usb_get_string_length(const usb_device_t* dev, uint8_t index);

/**
 * @brief gets a string descriptor from a device
 * @param[in] dev pointer to the device
 * @param[in] index index of the string descriptor
 * @param[in] str pointer to the string
*/
bool usb_get_string(const usb_device_t* dev, uint8_t index, char* str);

/**
 * @brief sets the configuration of a device
 * @param[in] dev pointer to the device
 * @param[in] config configuration to set
 * @return status of the transfer
*/
usb_transfer_status_t usb_set_configuration(const usb_device_t* dev, uint8_t config);

typedef enum usb_endpoint_type
{
    USB_ENDPOINT_CONTROL = 0b00,
    USB_ENDPOINT_ISOCHRONOUS = 0b01,
    USB_ENDPOINT_BULK = 0b10,
    USB_ENDPOINT_INTERRUPT = 0b11
} usb_endpoint_type_t;

/**
 * @brief gets an endpoint from the current interface of a device
 * @param[in] device pointer to the device
 * @param[in] in true if the endpoint is an input endpoint
 * @param[in] type type of the endpoint
 * @return pointer to the endpoint
*/
usb_endpoint_t* usb_get_endpoint(const usb_device_t* device, bool in, usb_endpoint_type_t type);


usb_transfer_status_t usb_transfer_bulk_out(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* payload, size_t size);
usb_transfer_status_t usb_transfer_bulk_in(const usb_bus_t* bus, uint64_t addr, uint64_t endpoint, void* payload, size_t size);