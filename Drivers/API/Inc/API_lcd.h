/*
 * API_lcd.h
 *
 *  Created on: 14 mar. 2023
 *      Author: ipave
 */

#ifndef API_INC_API_LCD_H_
#define API_INC_API_LCD_H_

#include <stdio.h>
#include "stm32f4xx_hal.h"  		/* <- HAL include */
//#include "stm32f4xx_nucleo_144.h" 	/* <- BSP include */
#include <stdbool.h>

#define DATOS 1
#define CONTROL 0

#define LCD_DIR 0x27

typedef enum
{
  LCD_OK       = 0x00U,
  LCD_ERROR    = 0x01U,

} LCD_Status_t;

//Bits de ENABLE y BACK LIGHT
#define EN (1<<2)
#define BL (1<<3)

//SEÑALES DE CONTROL

#define CLR_LCD 1
#define RETURN_HOME (1<<1)
#define ENTRY_MODE (1<<2)
#define DISPLAY_CONTROL (1<<3)
#define CURSOR_DISPLAY_SHIFT (1<<4)
#define FUNTION_SET (1<<5)
#define SET_CGRAM (1<<6)
#define SET_DDRAM (1<<7)
/************************************/
#define DISPLAY_ON (1<<2)
#define CURSOR_ON (1<<1)
#define CURSOR_BLINK 1
#define AUTOINCREMENT (1<<1)
#define _4BIT_MODE 0x28




//DEMORAS NECESARIAS

#define DELAY20ms 20
#define DELAY10ms 10
#define DELAY2ms  2
#define DELAY1ms  1
#define DelayTime 1

//MANEJO DE NIBBLES
#define HIGH_NIBBLE 0xf0
#define LOW_NIBBLE_MASK 0x0f
#define LOW_NIBBLE 4

//LINEAS DEL LCD

#define LINEA1 0x80
#define LINEA2 0xc0


//COMANDOS DE INICIALIZACIÓN
#define COMANDO_INI1 0x30
#define COMANDO_INI2 0x20


//PROTOTIPOS DE FUNCIONES
_Bool Init_Lcd(void);
void SacaTextoLcd (uint8_t *texto);
void DatoLcd (uint8_t dato);
void DatoAsciiLcd (uint8_t dato);
void DatoBCD (uint8_t dato);
void ClrLcd(void);
void PosCaracHLcd(uint8_t posH);
void PosCaracLLcd(uint8_t posL);
void CursorOffLcd(void);
void CursorOnLcd(void);




#endif /* API_INC_API_LCD_H_ */
