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

static BLState_e setBootFlags(BLMessage_rx_t* msg);
static BLState_e checkBootFlags(BLMessage_rx_t* msg);
static BLState_e processMetadata(BLMessage_rx_t* msg);
static BLState_e checkFlashedCRC(BLMessage_rx_t* msg);
static BLState_e flashApp(BLMessage_rx_t* msg);
static BLState_e validateFlash(BLMessage_rx_t* msg);
static BLState_e launchApp(BLMessage_rx_t* msg);

static FSMTableEntry_t transition_table[] = 
{
    {S_WAIT_FOR_FLAG, M_FLAG_SET, setBootFlags},        // Waiting for flag, got external message
    {S_WAIT_FOR_FLAG, M_NONE, checkBootFlags},          // Waiting for flag, but timed out

    {S_RECOVERY, M_FLAG_SET, setBootFlags},             // In recovery mode, recieved new flags

    {S_CRC_CHECK, M_NONE, checkFlashedCRC},             // Going to check CRC
    
    {S_WAIT_FOR_META, M_METADATA,  processMetadata},    // Waiting for meta, got metadata message

    {S_FLASH_APP, M_APP_DATA, flashApp},                // Rx a piece of program data and write to flash

    {S_VALIDATE_FLASH, M_NONE, validateFlash},          // Validate Flash CRC and store to flash

    {S_LAUNCH_APP, M_NONE, launchApp}                   // Clean up peripherals and launch application
};

/**
 * @brief Based on the current operating state, call the state function corresponding to the type of the incoming message
 * See the @ref transition_table for the mapping of state functions. Next state is determined by the return value of state functions.
 * 
 * @param message Bootloader message recieved
 */
static BLState_e bootloaderFSM(BLState_e currentState, BLMessage_rx_t *message)
{
    for(int i = 0; i < sizeof(transition_table)/sizeof(FSMTableEntry_t); i++)
    {
        FSMTableEntry_t *entry = &(transition_table[i]);
        if (currentState == entry->state && message->type == entry->type)
        {
            return entry->fn(message);
        }
    }
    
    // Maybe handle some sort of invalid message error?
    return currentState;
}

/**
 * @brief Begin main bootloader super loop. Wait for a message in te FIFO queue and send it to the FSM.
 * 
 */
void bootloaderMain()
{
    BLState_e currentState = S_WAIT_FOR_FLAG;
    BLMessage_rx_t fsmMessage;
    while (1)
    {
        if (!isRBQueueEmpty(&rx_message_q))
        {
            rbDequeue(&rx_message_q, &fsmMessage);
            currentState = bootloaderFSM(currentState, &fsmMessage);
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
    initRBQueue(&rx_message_q, (uint8_t*)rx_array, sizeof(rx_array)/sizeof(BLMessage_rx_t), sizeof(BLMessage_rx_t));
}


static BLState_e setBootFlags(BLMessage_rx_t* msg)
{
    return S_RECOVERY;
}

static BLState_e checkBootFlags(BLMessage_rx_t* msg)
{
    return S_RECOVERY;
}

static BLState_e processMetadata(BLMessage_rx_t* msg)
{
    return S_RECOVERY;
}

static BLState_e checkFlashedCRC(BLMessage_rx_t* msg)
{
    return S_RECOVERY;
}

static BLState_e flashApp(BLMessage_rx_t* msg)
{
    return S_RECOVERY;
}

static BLState_e validateFlash(BLMessage_rx_t* msg)
{
    return S_RECOVERY;
}

static BLState_e launchApp(BLMessage_rx_t* msg)
{
    return S_RECOVERY;
}