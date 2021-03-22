#ifdef STM32L4
#include "stm32l432xx.h"
#endif

#include "stm32f429xx.h"
#include "hal_can_f4.h"


#include <rb_queue.h>
#include <bootloader.h>

// TODO: Replace with linker script references.
uint32_t SAVED_CRC;
uint32_t SAVED_APP_LENGTH;
uint32_t BOOT_FLAG;
uint32_t APP_FLASH_START;

int main (void)
{

    // Ensure that Vector table offset is carried over from linker script
    // VTOR is set in the SystemInit() function to 0x08000000, as long as we do not
    // Have an interrupt from SystemInit() to main(), we should be fine.
    extern uint32_t* g_pfnVectors;
    SCB->VTOR = (uint32_t) (&g_pfnVectors);

    /*************
     * Queue & Data Structure Setup
     *************/
    bootloaderInit();

    /*************
     * Peripheral Setup
     *************/
    initCAN1();

    /*************
     * Enable IRQ lines
     *************/

    // CAN1 Interrupts
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    /*************
     * Main program loop
     *************/
    bootloaderMain();

    /*************
     * Peripheral Teardown
     *************/
    deinitCAN1();
    NVIC_DisableIRQ(CAN1_RX0_IRQn);

}

CanMsgTypeDef can_rx_msg;
extern rb_queue_t rx_message_q;
void CAN1_RX0_IRQHandler() 
{
    // Copy CAN frame into message buffer
    *(&can_rx_msg.Data[0]) = (CAN1->sFIFOMailBox[0].RDLR);
    *(&can_rx_msg.Data[4]) = (CAN1->sFIFOMailBox[0].RDHR);

    if (rbEnqueue(&rx_message_q, &can_rx_msg))
    {
        CAN1->RF0R |= (CAN_RF0R_RFOM0); // Release this mailbox
        NVIC_ClearPendingIRQ(CAN1_RX0_IRQn);
    } else {
        // We filled up our message queue! Do something about it
    }    
}