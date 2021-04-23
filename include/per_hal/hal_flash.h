/**
 * @file hal_flash.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-03-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef PER_HAL_FLASH
#define PER_HAL_FLASH

#include "stm32f429xx.h"

// Flash magic numbers obtained from family reference manual
#define FLASH_KEY_1 0x45670123
#define FLASH_KEY_2 0xCDEF89AB

void flashWriteU32(uint32_t address, uint32_t value);

#endif