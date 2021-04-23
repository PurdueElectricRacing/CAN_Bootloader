/**
 * @file bootloader.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Bootloader FSM implementation
 * @version 0.1
 * @date 2021-02-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <bootloader.h>


// Persistant Values
#define SHARED_FLASH __attribute__((section(".shared_flash")));
volatile uint32_t MAGIC_WORD        SHARED_FLASH;
volatile uint32_t SAVED_CRC         SHARED_FLASH;
volatile uint32_t SAVED_APP_LENGTH  SHARED_FLASH;
volatile uint32_t BOOT_FLAG         SHARED_FLASH;
volatile uint32_t APP_FLASH_START   SHARED_FLASH;

static BLState_e setBootFlags(BLMessageData_t* msg);
static BLState_e checkBootFlags(BLMessageData_t* msg);
static BLState_e processMetadata(BLMessageData_t* msg);
static BLState_e checkFlashedCRC(BLMessageData_t* msg);
static BLState_e flashApp(BLMessageData_t* msg);
static BLState_e validateFlash(BLMessageData_t* msg);
static BLState_e launchApp(BLMessageData_t* msg);

static bool decodeCANMsg(CanMsgTypeDef* canMessage, BLMessageData_t* fsmMessage);

static FSMTableEntry_t transition_table[] = 
{
    {S_WAIT_FOR_FLAG,  M_FLAG_SET,  setBootFlags},      // Waiting for flag, got external message
    {S_WAIT_FOR_FLAG,  M_NONE,      checkBootFlags},    // Waiting for flag, but timed out

    {S_RECOVERY,       M_FLAG_SET,  setBootFlags},      // In recovery mode, recieved new flags

    {S_CRC_CHECK,      M_NONE,      checkFlashedCRC},   // Going to check CRC
    
    {S_WAIT_FOR_META,  M_METADATA,  processMetadata},   // Waiting for meta, got metadata message

    {S_FLASH_APP,      M_APP_DATA,  flashApp},          // Rx a piece of program data and write to flash

    {S_VALIDATE_FLASH, M_NONE,      validateFlash},     // Validate Flash CRC and store to flash

    {S_LAUNCH_APP,     M_NONE,      launchApp}          // Clean up peripherals and launch application
};

/**
 * @brief Based on the current operating state, call the state function corresponding to the type of the incoming message
 * See the @ref transition_table for the mapping of state functions. Next state is determined by the return value of state functions.
 * 
 * @param message Bootloader message recieved
 */
static BLState_e bootloaderFSM(BLState_e currentState, BLMessageData_t *message)
{
    for(int i = 0; i < sizeof(transition_table)/sizeof(FSMTableEntry_t); i++)
    {
        FSMTableEntry_t *entry = &(transition_table[i]);
        if (currentState == entry->state && message->generic.message_type == entry->type)
        {
            return entry->fn(message);
        }
    }
    
    // Maybe handle some sort of invalid message error?
    return currentState;
}

/**
 * @brief Begin main bootloader super loop. Wait for a message in the FIFO queue and send it to the FSM.
 * 
 */
void bootloaderMain()
{
    BLState_e currentState = S_WAIT_FOR_FLAG;
    BLMessageData_t fsmMessage;
    CanMsgTypeDef canMessage;
    while (1)
    {
        if (!isRBQueueEmpty(&rx_message_q))
        {
            rbDequeue(&rx_message_q, &canMessage);
            
            if (decodeCANMsg(&canMessage, &fsmMessage)) // Ensure that message is valid type
            {
                currentState = bootloaderFSM(currentState, &fsmMessage);
            }
            
        }
        asm("wfi");
    }
}

/**
 * @brief Initalize all bootloader data structures before FSM starts
 * 
 */
void bootloaderInit()
{
    initRBQueue(&rx_message_q, (uint8_t*)rx_array, sizeof(rx_array)/sizeof(CanMsgTypeDef), sizeof(CanMsgTypeDef));
    initRBQueue(&tx_message_q, (uint8_t*)tx_array, sizeof(tx_array)/sizeof(CanMsgTypeDef), sizeof(CanMsgTypeDef));

    tempApplicationCRC = 0;
    tempApplicationLength = 0;
    flashedApplicationIndex = 0;

    if(MAGIC_WORD != 0xDEADBEEF)
    {
        flashWriteU32((uint32_t) &BOOT_FLAG,  0xFA);
    }
        
}

/**
 * @brief Translate CANRx message into bootloader message data structure.
 * 
 * @param canMessage 
 * @param fsmMessage 
 * @return true CAN message translated into a valid Bootloader message
 * @return false otherwise
 */
static bool decodeCANMsg(CanMsgTypeDef* canMessage, BLMessageData_t* fsmMessage)
{
    for(int i = 0; i < 8; i++)
        ((uint8_t*)&(fsmMessage->all_data))[i] = ((uint8_t*)canMessage->Data)[i];

    return fsmMessage->generic.message_type <= M_APP_DATA;
}

/**
 * @brief Set the Boot Flags object stored in Flash
 * 
 * @param msg 
 * @return BLState_e 
 */
static BLState_e setBootFlags(BLMessageData_t* msg)
{
    // BOOT_FLAG = msg->flag_set.operation_mode_flag;
    flashWriteU32((uint32_t) &BOOT_FLAG,  msg->flag_set.operation_mode_flag);
    return checkBootFlags(msg);
}

/**
 * @brief Decide to flash a new application or launch current one based on
 * saved boot flag.
 * 
 * @param msg 
 * @return BLState_e 
 */
static BLState_e checkBootFlags(BLMessageData_t* msg)
{
    if (BOOT_FLAG == FLAG_BOOT_TO_APP)
        return S_VALIDATE_FLASH;
    
    if (BOOT_FLAG == FLAG_FLASH_NEW_APP)
        return S_WAIT_FOR_META;
    
    return S_RECOVERY;
}

/**
 * @brief Store user supplied CRC and application lengths
 * 
 * @param msg 
 * @return BLState_e 
 */
static BLState_e processMetadata(BLMessageData_t* msg)
{
    tempApplicationLength   = msg->metadata.application_length;
    tempApplicationCRC      = msg->metadata.crc_value;
    flashedApplicationIndex = APP_FLASH_START;
    flashedApplicationEnd   = APP_FLASH_START + tempApplicationLength;

    // Metadata for application recieved, begin waiting for application data.
    return S_FLASH_APP;
}

/**
 * @brief Check the temparary CRC and lenght after flashing a new application
 * 
 * @param msg 
 * @return BLState_e 
 */
static BLState_e checkFlashedCRC(BLMessageData_t* msg)
{
    BLState_e nextState = S_RECOVERY;
    
    if (calculateCRC(APP_FLASH_START, tempApplicationLength) == tempApplicationCRC)
    {
        // Recieved length and CRC passed the check, store new values and reboot
        // SAVED_CRC = tempApplicationCRC;
        // SAVED_APP_LENGTH = tempApplicationLength;
        // BOOT_FLAG = FLAG_BOOT_TO_APP;

        flashWriteU32((uint32_t) &SAVED_CRC, tempApplicationCRC);
        flashWriteU32((uint32_t) &SAVED_APP_LENGTH, tempApplicationLength);
        flashWriteU32((uint32_t) &BOOT_FLAG, FLAG_BOOT_TO_APP);

        nextState = S_LAUNCH_APP;
    } else {
        // TODO: Send CRC Error
        // BOOT_FLAG = FLAG_FLASH_NEW_APP;
        flashWriteU32((uint32_t) &BOOT_FLAG, FLAG_FLASH_NEW_APP);

        nextState = S_WAIT_FOR_META;
    }

    deinitCRC();
    return nextState;
}

/**
 * @brief Recieve a word from outside world and program it according to a running counter.
 * Once all words have been recieved, proceed to check that all recieved data matches temp CRC.
 * 
 * @param msg 
 * @return BLState_e 
 */
static BLState_e flashApp(BLMessageData_t* msg)
{
    // *((uint32_t*)flashedApplicationIndex++) = msg->app_data.app_data;
    flashWriteU32((uint32_t) *((uint32_t*)flashedApplicationIndex++), msg->app_data.app_data);

    if (flashedApplicationIndex == flashedApplicationEnd)
    {
        return S_CRC_CHECK;
    }
    
    return S_FLASH_APP;
}

/**
 * @brief Validate currently stored Flash application with saved CRC and lenght values.
 * If validation is sucessful, we will enter the Launch App state.
 * Otherwise, bootloader will wait for a new application to be flashed.
 * 
 * @param msg Not Used
 * @return BLState_e Next State
 */
static BLState_e validateFlash(BLMessageData_t* msg)
{
    BLState_e nextState = S_RECOVERY;
    
    if (calculateCRC(APP_FLASH_START, SAVED_APP_LENGTH) == SAVED_CRC)
    {
        // We have verified the integrety of the current flash. Go ahead and launch the application
        nextState = S_LAUNCH_APP;
    } else {
        // TODO: Send CRC Error
        // BOOT_FLAG = FLAG_FLASH_NEW_APP;
        flashWriteU32((uint32_t) &BOOT_FLAG, FLAG_FLASH_NEW_APP);

        nextState = S_WAIT_FOR_META;
    }

    deinitCRC();
    return nextState;
}

/**
 * @brief Prepare bootloader to launch user application
 * 
 * @param msg Not Used
 * @return BLState_e 
 */
static BLState_e launchApp(BLMessageData_t* msg)
{
    return S_RECOVERY;
}

