/**
 * @file hal_crc.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-03-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "per_hal/hal_crc.h"

/**
 * @brief Initilize the CRC peripheral
 * 
 */
void initCRC()
{
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
    CRC->CR |= CRC_CR_RESET;
}

/**
 * @brief De-initilize the CRC peripheral
 * 
 */
void deinitCRC()
{
    RCC->AHB1RSTR |= RCC_AHB1RSTR_CRCRST;
    RCC->AHB1ENR &= ~RCC_AHB1ENR_CRCEN;
}

/**
 * @brief Accumulate a 32bit value into the CRC calculation.
 *  Return the accumulated CRC value
 * 
 * @param data Data to add to CRC calcluation
 * @return uint32_t Accumulated CRC value
 */
uint32_t accum32CRC(uint32_t data)
{
    CRC->DR = data;
    return CRC->DR;
}

/**
 * @brief Calculate CRC of an arbitrary memory region. Will reset any previous CRC calculations.
 * 
 * @param start Flash memory start location
 * @param length Number of words to accumulate
 * @return uint32_t 
 */
uint32_t calculateCRC(uint32_t start, uint32_t length)
{
    initCRC();
    for(uint32_t flash_index = start; flash_index < length + start; flash_index++)
        accum32CRC(*((uint32_t*)(flash_index)));
    uint32_t final_crc = CRC->DR;
    deinitCRC();
    return final_crc;
}