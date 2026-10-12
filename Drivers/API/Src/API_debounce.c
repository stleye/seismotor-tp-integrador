/*
 * API_debounce.c
 *
 *  Created on: Sep 17, 2026
 *      Author: sebastian
 */

#include "API_debounce.h"
#include "API_delay.h"
#include "stm32f4xx_hal.h"

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

static delay_t delay;

static bool_t fallingEdgeFlag;

static const uint8_t delayDuration = 40;

typedef enum {
	BUTTON_UP, BUTTON_FALLING, BUTTON_DOWN, BUTTON_RAISING,
} debounceState_t;

static debounceState_t currentState;

/**
 * @brief  Marca que se detectó un flanco descendente (tecla presionada).
 * @param  None
 * @retval None
 */
static void buttonPressed() {
	fallingEdgeFlag = true;
}

/**
 * @brief  Se ejecuta al detectarse un flanco ascendente (tecla liberada).
 * @param  None
 * @retval None
 */
static void buttonReleased() {
}

/**
 * @brief  Inicializa la MEF anti-rebote en el estado BUTTON_UP.
 * @param  None
 * @retval None
 */
void debounceFSM_init() {
	delayInit(&delay, delayDuration);
	currentState = BUTTON_UP;
}

/**
 * @brief  Lee la entrada del pulsador, resuelve la transición de estados de
 *         la MEF anti-rebote y dispara los eventos de flanco correspondientes.
 *         Debe llamarse periódicamente.
 * @param  None
 * @retval None
 */
void debounceFSM_update() {
	switch (currentState) {
	case BUTTON_UP:
		if (!HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
			currentState = BUTTON_FALLING;
		}
		break;
	case BUTTON_FALLING:
		if (delayRead(&delay)) {
			if (!HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
				currentState = BUTTON_DOWN;
				buttonPressed();
			} else {
				currentState = BUTTON_UP;
			}
		}
		break;
	case BUTTON_RAISING:
		if (delayRead(&delay)) {
			if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
				currentState = BUTTON_UP;
				buttonReleased();
			} else {
				currentState = BUTTON_DOWN;
			}
		}
		break;
	case BUTTON_DOWN:
		if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
			currentState = BUTTON_RAISING;
		}
		break;
	default:
		debounceFSM_init();
	}
}

/**
 * @brief  Indica si se detectó una pulsación (flanco descendente) desde la
 *         última lectura. Al leerla en true, resetea la variable interna a false.
 * @param  None
 * @retval bool_t: true si hubo una pulsación pendiente de leer
 */
bool_t readKey() {
	bool_t pressed = fallingEdgeFlag;
	fallingEdgeFlag = false;
	return pressed;
}
