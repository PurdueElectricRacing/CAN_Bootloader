/**
 * @file hal_can_f4.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Basic CAN Peripheral HAL library for setting up CAN peripheral and sending messages
 * @version 0.1
 * @date 2021-02-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifdef STM32F4

#include <per_hal/hal_can.h>

/**
 * @brief Initilize CAN1 peripheral using PA11 and PA12
 * 
 * @return true Peripheral sucessfully initalized
 * @return false Peripheral stalled during initilization
 */
bool initCAN1()
{
    // Enable PA11 => CAN1_RX and PA12 => CAN_TX
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Set to alternate function mode #9 for both PA11 and PA12
    GPIOA->MODER &= ~(GPIO_MODER_MODE11_Msk | GPIO_MODER_MODE12_Msk);
    GPIOA->MODER |= GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1;
    GPIOA->AFR[1] |= (9 << GPIO_AFRH_AFSEL11_Pos) | (9 << GPIO_AFRH_AFSEL12_Pos);
    // GPIOA->OSPDR |= 
    
    // CAN RCC clock
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;

    // Leave SLEEP state
    CAN1->MCR &= ~CAN_MCR_SLEEP; 
    while(CAN1->MSR & CAN_MSR_SLAK)
        ; 

    // Enter INIT state
    CAN1->MCR |= CAN_MCR_INRQ;
    while(!(CAN1->MSR & CAN_MSR_INAK))
        ; 

    // Bit timing recovered from http://www.bittiming.can-wiki.info/
    CAN1->BTR = 0x001c0063;
    
    // Keep the bus active
    CAN1->MCR |= CAN_MCR_ABOM;

    // Loopback mode
    CAN1->BTR |= CAN_BTR_LBKM;
    
    // Setup filters for all IDs
    CAN1->FMR  |= CAN_FMR_FINIT;              // Enter init mode for filter banks
    CAN1->FM1R &= ~(CAN_FM1R_FBM0_Msk);       // Set bank 0 to mask mode
    CAN1->FS1R &= ~(1 << CAN_FS1R_FSC0_Pos);  // Set bank 0 to 16bit mode
    CAN1->FA1R |= (1 << CAN_FA1R_FACT0_Pos);  // Activate bank 0
    CAN1->sFilterRegister[0].FR1 = 0;         // Set mask to 0
    CAN1->sFilterRegister[0].FR2 = 0;
    CAN1->FMR  &= ~CAN_FMR_FINIT;             // Enable Filters

    // Enable FIFO0 RX message pending interrupt
    CAN1->IER |= CAN_IER_FMPIE0;

    // Enter NORMAL mode
    CAN1->MCR &= ~CAN_MCR_INRQ;
    while(CAN1->MSR & CAN_MSR_INAK)
        ;

    return true;
}

bool deinitCAN1()
{
    RCC->APB1RSTR |= RCC_APB1RSTR_CAN1RST;
    RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOARST;

    RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR &= ~RCC_APB1ENR_CAN1EN;
    return true;
}

/**
 * @brief Find an empty TX mailbox and transmit a CAN message if one is found.
 * Function will block until sucessful transmission of message until a specified timeout.
 * 
 * @param can CAN peripheral to transmit with
 * @param msgId Message ID
 * @return true Sucessful TX of message.
 * @return false Unable to find empty message or transmit took too long.
 */
bool txCANMessage(CAN_TypeDef* can, CanMsgTypeDef* msg)
{

    uint8_t txMbox = 0;
    uint32_t timeout = 0;
    uint32_t txOkay = 0;

    if (can->TSR & CAN_TSR_TME0)
    {
        txMbox = 0;
        txOkay = CAN_TSR_TXOK0;
    }
    else if (can->TSR & CAN_TSR_TME1)
    {
        txMbox = 1;
        txOkay = CAN_TSR_TXOK1;
    }
    else if (can->TSR & CAN_TSR_TME2)
    {
        txMbox = 2;
        txOkay = CAN_TSR_TXOK2;
    }
    else   
        return false;   // Unable to find Mailbox

    can->sTxMailBox[txMbox].TIR  = (msg->StdId << CAN_TI0R_STID_Pos);  // ID
    can->sTxMailBox[txMbox].TDTR = (msg->DLC << CAN_TDT0R_DLC_Pos);    // Data Length
    can->sTxMailBox[txMbox].TDLR = (uint32_t) *(&msg->Data[0]);        // Data
    can->sTxMailBox[txMbox].TDHR = (uint32_t) *(&msg->Data[4]);        // Data
    
    can->sTxMailBox[txMbox].TIR |= (0b1 << CAN_TI0R_TXRQ_Pos);   // Request TX

    while(!(can->TSR & txOkay) && timeout != TX_TIMEOUT)         // Wait for message to be sent within specified timeout
        timeout++;

    return timeout != TX_TIMEOUT;
}

#endif