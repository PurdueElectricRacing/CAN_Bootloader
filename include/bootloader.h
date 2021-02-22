#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <rb_queue.h>
#include <stdint.h>

// These will eventually be repalced by auto-generated CAN message structures.
typedef enum {
    M_NONE,
    M_FLAG_SET,       // Set boot mode to prog or launch
    M_METADATA,       // Send metadata for app data
    M_APP_DATA        // Application data
} BLMessageType_e;  

typedef struct {
    BLMessageType_e type;
    uint8_t data[8];
} BLMessage_rx_t;

typedef enum {
    S_WAIT_FOR_FLAG,  // Initial state on startup, wait fo prog or boot flag message
    S_RECOVERY,       // Neither flag was set, wait for flag to be sent over CAN
    S_CRC_CHECK,      // Pre-app launch CRC check
    S_LAUNCH_APP,     // Launching application code
    S_WAIT_FOR_META,  // Waiting to recieve application metadata
    S_FLASH_APP,      // Recieve app program data and wirting to flash
    S_VALIDATE_FLASH  // Validating flash and generating CRC
} BLState_e;

typedef struct {
    BLState_e state;
    BLMessageType_e type;
    BLState_e (*fn)(BLMessage_rx_t*);
} FSMTableEntry_t;


void bootloaderMain();
void bootloaderInit();


// Ring buffer queue for CAN rxMessages
rb_queue_t rx_message_q;
BLMessage_rx_t rx_array [10];

#endif