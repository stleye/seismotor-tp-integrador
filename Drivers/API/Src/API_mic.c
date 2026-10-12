/*
 * API_mic.c
 *
 * El port llena i2sBuf en modo circular y avisa desde la interrupcion cada vez que se
 * completa una mitad (onHalfReady). Mientras el DMA llena una mitad, la aplicacion
 * tiene 32 ms para leer la otra con micGetBlock().
 */

#include "API_mic.h"
#include <stddef.h>
#include "API_mic_port.h"

#define PCM_SHIFT 6 /* 24 bit -> 16 bit: >>8 es escala completa; >>6 da 4x de ganancia (satura) */

#define BUF_HALVES          2  /* doble buffer: el DMA llena una mitad mientras se lee la otra */
#define HALFWORDS_PER_FRAME 4  /* frame estereo = 2 muestras de 32 bits = 4 half-words (DMA de 16 bits) */
#define HALFWORD_BITS       16
#define SAMPLE_PAD_BITS     8  /* 24 bits utiles alineados a la izquierda en 32: sobran 8 abajo */

#define READY_NONE   0 /* valores de readyHalf */
#define READY_FIRST  1
#define READY_SECOND 2

static uint16_t i2sBuf[BUF_HALVES * MIC_BLOCK_SAMPLES * HALFWORDS_PER_FRAME];
static int16_t pcm[MIC_BLOCK_SAMPLES];

/* Mitad lista para leer (READY_NONE, READY_FIRST o READY_SECOND). La escribe la
 * interrupcion y la borra micGetBlock(). */
static volatile uint8_t readyHalf;
static volatile uint32_t dropped;

/**
 * @brief  Callback del port: se completo una mitad de i2sBuf. Corre en la interrupcion.
 * @param  half: MIC_PORT_FIRST_HALF o MIC_PORT_SECOND_HALF
 * @retval None
 */
static void onHalfReady(uint8_t half) {
	if (readyHalf != READY_NONE) {
		dropped++; /* la mitad anterior nunca se leyo */
	}
	readyHalf = (half == MIC_PORT_FIRST_HALF) ? READY_FIRST : READY_SECOND;
}

/**
 * @brief  Arma la muestra del canal izquierdo a partir de un frame I2S. Con L/R a GND el
 *         microfono sale por el canal izquierdo (half-words 0 y 1 del frame), con 24 bits
 *         MSB-first alineados a la izquierda en los 32 bits.
 * @param  frame: puntero al primer half-word del frame
 * @retval int32_t: muestra de 24 bits con signo
 */
static int32_t sampleLeft(const uint16_t *frame) {
	int32_t raw = (int32_t) (((uint32_t) frame[0] << HALFWORD_BITS) | frame[1]);
	return raw >> SAMPLE_PAD_BITS;
}

/**
 * @brief  Arranca la captura I2S por DMA. Queda corriendo siempre.
 * @param  None
 * @retval bool_t: true si el hardware arranco
 */
bool_t micInit(void) {
	readyHalf = READY_NONE;
	dropped = 0;
	return micPortStart(i2sBuf, BUF_HALVES * MIC_BLOCK_SAMPLES, onHalfReady);
}

/**
 * @brief  Indica si hay un bloque de audio nuevo listo para leer.
 * @param  None
 * @retval bool_t: true si hay un bloque sin leer
 */
bool_t micBlockReady(void) {
	return readyHalf != READY_NONE;
}

/**
 * @brief  Convierte el bloque listo a PCM de 16 bits con saturacion y lo marca como leido.
 * @param  None
 * @retval const int16_t*: MIC_BLOCK_SAMPLES muestras, o NULL si no habia bloque listo
 */
const int16_t* micGetBlock(void) {
	uint8_t h = readyHalf;
	if (h == READY_NONE) {
		return NULL;
	}
	readyHalf = READY_NONE;
	const uint16_t *half = &i2sBuf[(h == READY_FIRST) ? 0 : MIC_BLOCK_SAMPLES * HALFWORDS_PER_FRAME];
	for (uint32_t i = 0; i < MIC_BLOCK_SAMPLES; i++) {
		int32_t s = sampleLeft(&half[i * HALFWORDS_PER_FRAME]) >> PCM_SHIFT;
		if (s > INT16_MAX) {
			s = INT16_MAX;
		} else if (s < INT16_MIN) {
			s = INT16_MIN;
		}
		pcm[i] = (int16_t) s;
	}
	return pcm;
}

/**
 * @brief  Devuelve cuantos bloques se pisaron sin leer desde micInit().
 * @param  None
 * @retval uint32_t: bloques perdidos
 */
uint32_t micDropped(void) {
	return dropped;
}
