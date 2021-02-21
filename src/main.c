
#include "stm32l432xx.h"

#include <rb_queue.h>
#include <bootloader.h>

int main (void)
{

    // Ensure that Vector table offset is carried over from linker script
    // VTOR is set in the SystemInit() function to 0x08000000, as long as we do not
    // Have an interrupt from SystemInit() to main(), we should be fine.
    extern uint32_t* g_pfnVectors;
    SCB->VTOR = (uint32_t) (&g_pfnVectors);

    /*************
     * Peripheral Setup
     *************/

    // Enable syscfg timer and line0 interrupts
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB;

    // Enable external interrupt on line 0 for rising & falling 
    EXTI->IMR1 |= EXTI_IMR1_IM0;
    EXTI->RTSR1 |= EXTI_RTSR1_RT0;
    EXTI->FTSR1 |= EXTI_FTSR1_FT0;

    // Setup GPIOB
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    // PB0 input and PB3 output
    GPIOB->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE3_Msk);
    GPIOB->MODER |= (0x01UL << GPIO_MODER_MODE3_Pos);


    /*************
     * Queue & Data Structure Setup
     *************/

    initRBQueue(&rx_message_q, (uint8_t*)rx_array, 10, sizeof(BLMessage_rx_t));


    /*************
     * Enable IRQ lines
     *************/

    // Enable EXTI interrupt for line 0
    NVIC_EnableIRQ(EXTI0_IRQn);


    /*************
     * Main program loop
     *************/

    // Basic super loop for handling messages from an interrupt function.
    // Example of toggling an LED for a "METADATA" message
    BLMessage_rx_t rx_message;
    while(1)
    {
        while(isRBQueueEmpty(&rx_message_q))
            asm("WFE");

        rbDequeue(&rx_message_q, &rx_message);

        switch(rx_message.type)
        {
            case(METADATA):
                GPIOB->ODR = GPIOB->ODR ^ GPIO_ODR_OD3;
                break;

            default:
                break;
        }
    }

}

/**
 * @brief Dummy IRQ for faking CAN messages. Will follow a sequence of messages defined somewhere...
 * 
 */
void EXTI0_IRQHandler () {
    EXTI->PR1 |= EXTI_PR1_PIF0;

    BLMessage_rx_t new_message = {
        .type = METADATA,
        .data = {0}
    };

    rbEnqueue(&rx_message_q, &new_message);
}