/*
 * API_mic.h
 *
 * Captura del microfono INMP441 por I2S con DMA. Entrega el audio en bloques de 32 ms
 * de PCM de 16 bits, mono (canal izquierdo). El acceso al hardware esta en API_mic_port.c.
 */

#ifndef API_INC_API_MIC_H_
#define API_INC_API_MIC_H_

#include <stdint.h>
#include <stdbool.h>

typedef bool bool_t;

#define MIC_SAMPLE_RATE   16000 /* el real es 15.957 kHz (-0.26 %) */
#define MIC_BLOCK_SAMPLES 512   /* muestras por bloque: 512 / 16 kHz = 32 ms */

/**
 * @brief  Arranca la captura I2S por DMA. Queda corriendo siempre.
 * @param  None
 * @retval bool_t: true si el hardware arranco
 */
bool_t micInit(void);

/**
 * @brief  Indica si hay un bloque de audio nuevo listo para leer.
 * @param  None
 * @retval bool_t: true si hay un bloque sin leer
 */
bool_t micBlockReady(void);

/**
 * @brief  Convierte el bloque listo a PCM de 16 bits y lo marca como leido. Hay que
 *         usarlo antes de que llegue el proximo (32 ms).
 * @param  None
 * @retval const int16_t*: MIC_BLOCK_SAMPLES muestras, o NULL si no habia bloque listo
 */
const int16_t* micGetBlock(void);

/**
 * @brief  Devuelve cuantos bloques se pisaron sin leer desde micInit(), porque el lazo
 *         principal tardo mas de 32 ms en pedirlos.
 * @param  None
 * @retval uint32_t: bloques perdidos
 */
uint32_t micDropped(void);

#endif /* API_INC_API_MIC_H_ */
