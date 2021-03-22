/**
 * @file hal_crc_f4.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-03-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef PER_HAL_CRC_F4
#define PER_HAL_CRC_F4

#include "stm32f429xx.h"

void initCRC();
void deinitCRC();

uint32_t accum32CRC(uint32_t data);
uint32_t calculateFlashCRC(uint32_t start, uint32_t length);

#endif