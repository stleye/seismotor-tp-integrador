/*
 * API_uart.c
 *
 *  Created on: Sep 24, 2026
 *      Author: sebastian
 */

#include <stdio.h>
#include <string.h>
#include "API_uart.h"
#include "stm32f4xx_hal.h"

#define UART_TX_TIMEOUT 1000
#define UART_RX_TIMEOUT 0
#define UART_BAUD 500000   /* 16 MHz / 16 / 2: exacto. El audio son 32 KB/s y esto da 50 KB/s */
#define UART_MAX_SIZE 1100 /* un bloque de audio: 3 bytes de cabecera + 1024 de muestras */

static UART_HandleTypeDef huartHandle;

static void Error_Handler(void);

/**
 * @brief  Inicializa la USART2 (500000 baudios, 8N1, sin control de flujo).
 * @param  None
 * @retval bool_t: true si la inicialización fue exitosa, false en caso contrario
 */
bool_t uartInit() {
	huartHandle.Instance = USART2;
	huartHandle.Init.BaudRate = UART_BAUD;
	huartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	huartHandle.Init.StopBits = UART_STOPBITS_1;
	huartHandle.Init.Parity = UART_PARITY_NONE;
	huartHandle.Init.Mode = UART_MODE_TX_RX;
	huartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

	/* Sin mensaje de bienvenida (el de la practica 5): por esta UART sale el audio
	 * en binario y el texto se mezclaria con los bloques */
	return HAL_UART_Init(&huartHandle) == HAL_OK;
}

/**
 * @brief  Envía por la UART un string completo, hasta el caracter '\0'.
 *         No hace nada si el puntero es NULL o si la longitud del string
 *         no está entre 1 y UART_MAX_SIZE.
 * @param  pstring: puntero al string terminado en '\0' a enviar
 * @retval None
 */
void uartSendString(uint8_t *pstring) {
	if (pstring == NULL)
		return;

	size_t size = strlen((const char*) pstring);

	if (size == 0 || size > UART_MAX_SIZE)
		return;

	if (HAL_UART_Transmit(&huartHandle, pstring, size, UART_TX_TIMEOUT)
			!= HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief  Envía por la UART la cantidad de caracteres indicada del string.
 *         No hace nada si el puntero es NULL o si size no está entre 1 y
 *         UART_MAX_SIZE.
 * @param  pstring: puntero al buffer a enviar
 * @param  size: cantidad de caracteres a enviar
 * @retval None
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size) {
	if (pstring == NULL || size == 0 || size > UART_MAX_SIZE)
		return;

	if (HAL_UART_Transmit(&huartHandle, pstring, size, UART_TX_TIMEOUT)
			!= HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief  Recibe por la UART, en modo polling, la cantidad de caracteres
 *         indicada y los guarda en el buffer. No hace nada si el puntero es
 *         NULL o si size no está entre 1 y UART_MAX_SIZE.
 * @param  pstring: puntero al buffer donde se guardan los datos recibidos
 * @param  size: cantidad de caracteres a recibir
 * @retval None
 */
bool_t uartReceiveStringSize(uint8_t *pstring, uint16_t size) {
	if (pstring == NULL || size == 0 || size > UART_MAX_SIZE)
		return false;

	HAL_StatusTypeDef status = HAL_UART_Receive(&huartHandle, pstring, size,
	UART_RX_TIMEOUT);
	if (status != HAL_OK && status != HAL_TIMEOUT) {
		Error_Handler();
	}
	return status == HAL_OK;
}

/**
 * @brief  Se ejecuta ante un error de la HAL: deshabilita las interrupciones
 *         y detiene la ejecución en un bucle infinito.
 * @param  None
 * @retval None
 */
static void Error_Handler(void) {
	__disable_irq();
	while (1) {
	}
}
