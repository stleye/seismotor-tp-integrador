/*
 * app.c
 *
 * MEF principal: INIT -> IDLE <-> STREAMING.
 */

#include "app.h"
#include <stdio.h>
#include "main.h"
#include "API_debounce.h"
#include "API_delay.h"
#include "API_lcd.h"
#include "API_mic.h"
#include "API_uart.h"

#define LCD_COLS       16
#define LCD_TOP        0    /* lineas del LCD para lcdLine() */
#define LCD_BOTTOM     1
#define LCD_REFRESH_MS 1000 /* el contador de segundos del LCD */

#define BLOCK_SYNC_1   0xA5 /* marca de principio de bloque: record_wav.py la busca para */
#define BLOCK_SYNC_2   0x5A /* engancharse al stream; son patrones de bits alternados y */
                            /* complementarios, poco probables en el audio */
#define BLOCK_HDR_SIZE 3    /* BLOCK_SYNC_1, BLOCK_SYNC_2 y el numero de secuencia */

static appState_t state;
static delay_t lcdDelay;
static uint32_t seconds;

/**
 * @brief  Escribe una linea completa del LCD, rellenando con espacios hasta el final
 *         para tapar lo que habia antes sin tener que borrar el display.
 * @param  line: LCD_TOP o LCD_BOTTOM
 * @param  text: texto a mostrar (se corta en LCD_COLS caracteres)
 * @retval None
 */
static void lcdLine(uint8_t line, const char *text) {
	char buf[LCD_COLS + 1];
	snprintf(buf, sizeof buf, "%-*s", LCD_COLS, text);
	if (line == LCD_TOP) {
		PosCaracHLcd(0);
	} else {
		PosCaracLLcd(0);
	}
	SacaTextoLcd((uint8_t*) buf);
}

/**
 * @brief  Muestra en la linea de abajo del LCD los segundos que lleva la grabacion.
 * @param  None
 * @retval None
 */
static void lcdShowSeconds(void) {
	char buf[LCD_COLS + 1];
	snprintf(buf, sizeof buf, "%lu s", (unsigned long) seconds);
	lcdLine(LCD_BOTTOM, buf);
}

/**
 * @brief  Manda un bloque de audio a la PC. Formato: [0xA5 0x5A seq] + las muestras
 *         int16 little-endian. La cabecera le permite a la PC encontrar el principio de
 *         cada bloque, y seq detectar si se perdio alguno.
 * @param  pcm: MIC_BLOCK_SAMPLES muestras
 * @retval None
 */
static void sendBlock(const int16_t *pcm) {
	static uint8_t seq;
	uint8_t hdr[BLOCK_HDR_SIZE] = { BLOCK_SYNC_1, BLOCK_SYNC_2, seq++ };
	uartSendStringSize(hdr, sizeof hdr);
	uartSendStringSize((uint8_t*) pcm, MIC_BLOCK_SAMPLES * sizeof(int16_t));
}

/**
 * @brief  Deja la MEF en APP_INIT, su punto de entrada.
 * @param  None
 * @retval None
 */
void appFSM_init(void) {
	debounceFSM_init();
	state = APP_INIT;
}

/**
 * @brief  Evalua las entradas del estado actual (pulsador, audio disponible, tiempo),
 *         ejecuta sus acciones y cambia de estado si corresponde. Se llama en cada
 *         vuelta del while(1).
 * @param  None
 * @retval None
 */
void appFSM_update(void) {
	bool_t key = readKey();

	switch (state) {
	case APP_INIT:
		Init_Lcd();
		if (!uartInit() || !micInit()) {
			lcdLine(LCD_TOP, "Error de inicio");
			Error_Handler();
		}
		lcdLine(LCD_TOP, "Seismotor");
		lcdLine(LCD_BOTTOM, "B1 para grabar");
		state = APP_IDLE;
		break;

	case APP_IDLE:
		if (key) {
			seconds = 0;
			delayInit(&lcdDelay, LCD_REFRESH_MS);
			lcdLine(LCD_TOP, "Grabando");
			lcdShowSeconds();
			state = APP_STREAMING;
		}
		break;

	case APP_STREAMING:
		if (micBlockReady()) {
			sendBlock(micGetBlock());
		}
		if (delayRead(&lcdDelay)) {
			seconds++;
			lcdShowSeconds();
		}
		if (key) {
			lcdLine(LCD_TOP, "Seismotor");
			lcdLine(LCD_BOTTOM, "B1 para grabar");
			state = APP_IDLE;
		}
		break;

	default:
		state = APP_INIT;
	}

	debounceFSM_update();
}
