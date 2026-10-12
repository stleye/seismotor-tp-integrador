/*
 * API_debounce.h
 *
 *  Created on: Sep 17, 2026
 *      Author: sebastian
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#include <stdbool.h>

typedef bool bool_t;

/**
 * @brief  Inicializa la MEF anti-rebote en el estado BUTTON_UP.
 * @param  None
 * @retval None
 */
void debounceFSM_init();

/**
 * @brief  Lee la entrada del pulsador, resuelve la transición de estados de
 *         la MEF anti-rebote y dispara los eventos de flanco correspondientes.
 *         Debe llamarse periódicamente.
 * @param  None
 * @retval None
 */
void debounceFSM_update();

/**
 * @brief  Indica si se detectó una pulsación (flanco descendente) desde la
 *         última lectura. Al leerla en true, resetea la variable interna a false.
 * @param  None
 * @retval bool_t: true si hubo una pulsación pendiente de leer
 */
bool_t readKey();

#endif /* API_INC_API_DEBOUNCE_H_ */
