/*
 * API_mic_port.c
 *
 * Port del driver del microfono para la NUCLEO-F446RE con la HAL de ST: I2S2
 * (configurado por CubeMX como master RX, Philips, 32 bits, 16 kHz) con DMA1 Stream3
 * circular de half-words.
 */

#include "API_mic_port.h"
#include "stm32f4xx_hal.h"

#define MIC_I2S hi2s2
#define I2S_WORDS_PER_FRAME 2 /* datos de 32 bits por frame estereo (L y R) */

extern I2S_HandleTypeDef MIC_I2S;

static micPortHalfCallback_t halfCallback;

/**
 * @brief  Arranca la recepcion I2S en modo circular sobre buf.
 * @param  buf: buffer de recepcion (las dos mitades)
 * @param  frames: cantidad total de frames estereo de buf
 * @param  onHalf: se llama cada vez que se completa una mitad
 * @retval bool_t: true si la HAL no informo error
 */
bool_t micPortStart(uint16_t *buf, uint32_t frames, micPortHalfCallback_t onHalf) {
	halfCallback = onHalf;
	/* Size en datos de 32 bits (2 por frame): la HAL lo duplica a half-words para el DMA */
	return HAL_I2S_Receive_DMA(&MIC_I2S, buf, frames * I2S_WORDS_PER_FRAME) == HAL_OK;
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	if (hi2s == &MIC_I2S && halfCallback != NULL) {
		halfCallback(MIC_PORT_FIRST_HALF);
	}
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
	if (hi2s == &MIC_I2S && halfCallback != NULL) {
		halfCallback(MIC_PORT_SECOND_HALF);
	}
}
