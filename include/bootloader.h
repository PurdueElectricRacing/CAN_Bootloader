#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <rb_queue.h>
#include <stdint.h>
#include <hal_can_f4.h>
#include <hal_crc.h>

/*
*   Value Table Struct Definitions
*/

typedef enum {
    FLAG_IDLE_IN_RECOVERY = 0x0U,
    FLAG_FLASH_NEW_APP    = 0x1U,
	FLAG_BOOT_TO_APP      = 0x2U	
} BLBootFlag_e;

typedef enum {
    M_NONE      = 0x0U,
    M_FLAG_SET  = 0x1U,       // Set boot mode to prog or launch
    M_METADATA  = 0x2U,       // Send metadata for app data
    M_APP_DATA  = 0x3U        // Application data
} BLMessageType_e;  


typedef union {
    uint64_t all_data;

    struct {
        uint64_t message_type        : 4;
        uint64_t ecu_id              : 4;
        uint64_t not_used            : 56;
    } generic;

    struct {
        uint64_t message_type        : 4;
        uint64_t ecu_id              : 4;
        uint64_t app_data            : 32;
        uint64_t not_used            : 24;
    } app_data;
    
    struct {
        uint64_t message_type        : 4;
        uint64_t ecu_id              : 4;
        uint64_t operation_mode_flag : 2;
        uint64_t not_used            : 54;
    } flag_set;

    struct {
        uint64_t message_type        : 4;
        uint64_t ecu_id              : 4;
        uint64_t application_length  : 24;
        uint64_t crc_value           : 32;
    } metadata;
} BLMessageData_t;


typedef enum {
    S_WAIT_FOR_FLAG  = 0x0,  // Initial state on startup, wait fo prog or boot flag message
    S_RECOVERY       = 0x1,  // Neither flag was set, wait for flag to be sent over CAN
    S_CRC_CHECK      = 0x2,  // Pre-app launch CRC check
    S_LAUNCH_APP     = 0x3,  // Launching application code
    S_WAIT_FOR_META  = 0x4,  // Waiting to recieve application metadata
    S_FLASH_APP      = 0x5,  // Recieve app program data and wirting to flash
    S_VALIDATE_FLASH = 0x6,  // Validating flash and generating CRC
    S_REBOOT         = 0x7   // Prepare bootloader to perform a soft reboot.
} BLState_e;

typedef struct {
    BLState_e state;
    BLMessageType_e type;
    BLState_e (*fn)(BLMessageData_t*);
} FSMTableEntry_t;


void bootloaderInit();
void bootloaderMain();

// Ring buffer queue for CAN rxMessages
rb_queue_t rx_message_q;
CanMsgTypeDef rx_array [10];

// Ring buffer queue for CAN rxMessages
rb_queue_t tx_message_q;
CanMsgTypeDef tx_array [10];

// Persistant Values
extern uint32_t SAVED_CRC;
extern uint32_t SAVED_APP_LENGTH;
extern uint32_t BOOT_FLAG;
extern uint32_t APP_FLASH_START;

// Flashing New Application Globals
uint32_t tempApplicationCRC;        // Compare to calculated CRC
uint32_t tempApplicationLength;     // New application length
uint32_t flashedApplicationIndex;   // Current flash index
uint32_t flashedApplicationEnd;     // When to stop flashing

#endif