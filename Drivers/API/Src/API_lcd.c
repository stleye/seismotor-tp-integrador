/// @file API_lcd.c
/// @version 1.0
/// @date 23/4/2023
/// @author Ing. Pavelek Israel
/// @title API de manejo del LCD de alto nivel
/// @brief funciones lcd_port.c de alto nivel



#include "API_lcd.h"
#include "API_Lcd_port.h"
#include "API_delay.h"
#include "API_I2C.h"

static void DelayLcd(uint32_t demora);
static void ControlLcd(uint8_t valor);
static void Envia8bitsLcd (uint8_t valor,_Bool tipo);
static void Envia4bitsLcd (uint8_t valor,_Bool tipo);

static const uint8_t LCD_INIT_CMD[]={
		_4BIT_MODE,DISPLAY_CONTROL,RETURN_HOME,ENTRY_MODE+AUTOINCREMENT,DISPLAY_CONTROL+DISPLAY_ON,CLR_LCD
};

/**
  * @brief  Init_Lcd
  * @param  void
  * @return Bool HAL_ERROR o HAL_OK
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/


_Bool Init_Lcd(void){

   //if(I2C_HW_init()==LCD_ERROR)return LCD_ERROR;
   DelayLcd(DELAY20ms);
   Envia4bitsLcd(COMANDO_INI1,CONTROL);
   DelayLcd(DELAY10ms);
   Envia4bitsLcd(COMANDO_INI1,CONTROL);
   DelayLcd(DELAY1ms);
   Envia4bitsLcd(COMANDO_INI1,CONTROL);
   DelayLcd(DELAY1ms);
   Envia4bitsLcd(COMANDO_INI2,CONTROL);
   DelayLcd(DELAY1ms);
   /* La lista incluye RETURN_HOME y CLR_LCD, que tardan 1.52 ms en ejecutarse: como
      Envia4bitsLcd ya no espera, se deja tiempo despues de cada comando de la lista. */
   for(uint8_t i=0;i<sizeof(LCD_INIT_CMD);i++){
	   ControlLcd(LCD_INIT_CMD[i]);
	   DelayLcd(DELAY2ms);
   }
   return LCD_OK;

}

/**
  * @brief  DelayLcd demora bloqueante para el uso del LCD
  * @param  valor de la demora a realizar en ms
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/


static void DelayLcd(uint32_t demora){
	  LCD_Delay(demora);
}

/**
  * @brief  ControlLcd envia una palabra de CONTROL al lcd
  * @param  uint8_t valor de control a enviar
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/


static void ControlLcd(uint8_t valor){
	Envia8bitsLcd(valor,CONTROL);
}

/**
  * @brief  DatoLcd envia una palabra de DATOS al lcd
  * @param  uint8_t valor de datos a enviar
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/


void DatoLcd (uint8_t dato){
	Envia8bitsLcd(dato,DATOS);
}
/**
  * @brief  DatoAsciiLcd envia una palabra de DATOS al lcd
  * @param  uint8_t valor de datos a enviar previo a su conversion en ASCII
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/


void DatoAsciiLcd (uint8_t dato){
	Envia8bitsLcd(dato+ '0',DATOS);
}

/**
  * @brief  DatoAsciiLcd envia una palabra de DATOS al lcd
  * @param  uint8_t valor de datos a enviar previo a su conversion en BCD
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/

void DatoBCD (uint8_t dato){

	  DatoAsciiLcd((((dato)&HIGH_NIBBLE)>>LOW_NIBBLE));
	  DatoAsciiLcd(((dato)&LOW_NIBBLE_MASK));

}

/**
  * @brief  DatoAsciiLcd Función que parte el envio de 1 byte en nible alto y bajo para el trabajo en 4 bits
  * @param  uint8_t valor a enviar
  * @param uint8_t tipo CONTROL o DATOS
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/

void Envia8bitsLcd (uint8_t valor,_Bool tipo){
	Envia4bitsLcd(valor&HIGH_NIBBLE,tipo); 		//me con quedo y envio los 4 bits más significaticos.
	Envia4bitsLcd(valor<<LOW_NIBBLE,tipo);   	//me quedo y envio los 4 bits menos significativos.
}

/**
  * @brief  DatoAsciiLcd Función que parte el envio de 4 bits
  * @param  uint8_t valor a enviar
  * @param uint8_t tipo CONTROL o DATOS
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/

static void Envia4bitsLcd (uint8_t valor,_Bool tipo){

	/* Sin esperas: el pulso de EN tiene que durar 450 ns y los comandos comunes se
	   ejecutan en 37 us, pero cada escritura I2C a 100 kHz (direccion + dato) ya tarda
	   ~0.2 ms. Las esperas de 1 ms que habia aca hacian que cada caracter tardara ~8 ms
	   y frenaban el envio del audio; sin ellas tarda ~0.8 ms.
	   Los comandos lentos (clear, home, inicializacion) esperan donde se los llama. */
	LCD_Write_Byte(valor+tipo+EN+BL);
	LCD_Write_Byte(valor+tipo+BL);
}

/**
  * @brief  SacaTextoLcd Funcion que envia un texto al LCD
  * @param  uint8_t * puntero a un string a enviar
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/


void SacaTextoLcd (uint8_t *texto){
	while(*texto)DatoLcd(*texto++);
}

/**
  * @brief  ClrLcd Funcion que borra el LCD
  * @param  void
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/

void ClrLcd(void){
   ControlLcd(CLR_LCD);				//	Display Clear.
   DelayLcd(DELAY2ms);

}


/**
  * @brief  PosCaracHLcd Función que posiciona el cursor del LCD en la linea superior.
  * @param  uint8_t un valor de 0 a la longitud de la pantalla
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/


void PosCaracHLcd(uint8_t posH){
	ControlLcd(posH | LINEA1);
}

/**
  * @brief  PosCaracLLcd Función que posiciona el cursor del LCD en la linea inferior.
  * @param  uint8_t un valor de 0 a la longitud de la pantalla
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/


void PosCaracLLcd(uint8_t posL){
	ControlLcd(posL | LINEA2);
}

/**
  * @brief  CursorOffLcd Funcion que apaga el cursor del LCD
  * @param  void
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/

void CursorOffLcd(void){
	ControlLcd(DISPLAY_CONTROL+DISPLAY_ON);
}

/**
  * @brief  CursorOnLcd Funcion que prende el cursor del LCD
  * @param  void
  * @return void
  * @author Ing. Pavelek Israel
  * @version 1.0
  * @date 16/4/2023
*/

void CursorOnLcd(void){
	ControlLcd(DISPLAY_CONTROL+CURSOR_ON+DISPLAY_ON+CURSOR_BLINK);
}

