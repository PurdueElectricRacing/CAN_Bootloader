#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <rb_queue.h>
#include <stdint.h>

// These will eventually be repalced by auto-generated CAN message structures.
typedef enum {
    MODE_SET = 0x1,     // Set boot mode to prog or launch
    METADATA = 0x2,     // Send metadata for app data
    APP_DATA = 0x3      // Application data
} BLMessageType_e;  

typedef struct {
    BLMessageType_e type;
    uint8_t data[64];
} BLMessage_rx_t;

// Ring buffer queue for CAN rxMessages
rb_queue_t rx_message_q;
BLMessage_rx_t rx_array [10];


#endif