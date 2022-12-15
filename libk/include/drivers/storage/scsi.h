#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct scsi_command
{
    uint8_t packet[32];
    uint8_t packet_len;
    bool write;
} scsi_command_t;

typedef struct scsi_driver
{
    uint8_t max_packet_size;
    void* data;
    bool (*send_scsi_cmd)(void* device, scsi_command_t* command, void* data, size_t transfer_length);
} scsi_driver_t;

/**
 * @brief registers a scsi devic
 * @param[in] driver driver for the scsi device
 */
void scsi_register_device(scsi_driver_t driver);
