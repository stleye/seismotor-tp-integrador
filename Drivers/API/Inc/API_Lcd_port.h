/*
 * API_LCDport.h
 *
 *  Created on: 25 mar. 2023
 *      Author: ipave
 */


#include <stdio.h>
#include "stm32f4xx_hal.h"  		/* <- HAL include */
#include <stdbool.h>
#include "API_lcd.h"

extern void Error_Handler(void);

_Bool LCD_HW_init(void);
void LCD_Write_Byte(uint8_t valor);
void LCD_Delay(uint32_t ms);
void GPIO_I2C(I2C_HandleTypeDef  *hi2c);


