/*
 * API_uart.h
 *
 *  Created on: Sep 24, 2026
 *      Author: sebastian
 */

#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_

#include <stdbool.h>
#include <stdint.h>

typedef bool bool_t;

/**
 * @brief  Inicializa la USART2 (500000 baudios, 8N1, sin control de flujo).
 * @param  None
 * @retval bool_t: true si la inicialización fue exitosa, false en caso contrario
 */
bool_t uartInit();

/**
 * @brief  Envía por la UART un string completo, hasta el caracter '\0'.
 *         No hace nada si el puntero es NULL o si la longitud del string
 *         no está entre 1 y 1100.
 * @param  pstring: puntero al string terminado en '\0' a enviar
 * @retval None
 */
void uartSendString(uint8_t *pstring);

/**
 * @brief  Envía por la UART la cantidad de caracteres indicada del string.
 *         No hace nada si el puntero es NULL o si size no está entre 1 y
 *         1100.
 * @param  pstring: puntero al buffer a enviar
 * @param  size: cantidad de caracteres a enviar
 * @retval None
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size);

/**
 * @brief  Recibe por la UART, en modo polling, la cantidad de caracteres
 *         indicada y los guarda en el buffer. No hace nada si el puntero es
 *         NULL o si size no está entre 1 y 1100.
 * @param  pstring: puntero al buffer donde se guardan los datos recibidos
 * @param  size: cantidad de caracteres a recibir
 * @retval None
 */
bool_t uartReceiveStringSize(uint8_t *pstring, uint16_t size);

#endif /* API_INC_API_UART_H_ */
