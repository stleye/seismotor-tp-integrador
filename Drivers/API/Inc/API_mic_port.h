/*
 * API_mic_port.h
 *
 * Acceso al hardware del driver del microfono: recepcion I2S continua por DMA a un
 * doble buffer. Es lo unico que hay que reescribir para usar API_mic con otro micro u
 * otro I2S. Solo lo incluye API_mic.c.
 */

#ifndef API_INC_API_MIC_PORT_H_
#define API_INC_API_MIC_PORT_H_

#include <stdint.h>
#include <stdbool.h>

typedef bool bool_t;

#define MIC_PORT_FIRST_HALF  0 /* valores de half en micPortHalfCallback_t */
#define MIC_PORT_SECOND_HALF 1

/**
 * @brief  Funcion que el port llama, desde la interrupcion, cada vez que se completa
 *         una mitad del buffer.
 * @param  half: MIC_PORT_FIRST_HALF o MIC_PORT_SECOND_HALF
 * @retval None
 */
typedef void (*micPortHalfCallback_t)(uint8_t half);

/**
 * @brief  Arranca la recepcion I2S en modo circular sobre buf: 32 bits por canal,
 *         estereo, cada frame ocupa 4 half-words (L alto, L bajo, R alto, R bajo).
 * @param  buf: buffer de recepcion (las dos mitades)
 * @param  frames: cantidad total de frames estereo de buf
 * @param  onHalf: se llama cada vez que se completa una mitad
 * @retval bool_t: true si el hardware arranco
 */
bool_t micPortStart(uint16_t *buf, uint32_t frames, micPortHalfCallback_t onHalf);

#endif /* API_INC_API_MIC_PORT_H_ */
