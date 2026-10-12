/*
 * app.h
 *
 * MEF principal: B1 arranca y corta el envio del audio del microfono a la PC por UART,
 * y el LCD muestra el estado y el tiempo de grabacion.
 */

#ifndef INC_APP_H_
#define INC_APP_H_

typedef enum {
	APP_INIT, APP_IDLE, APP_STREAMING,
} appState_t;

/**
 * @brief  Deja la MEF en APP_INIT, su punto de entrada.
 * @param  None
 * @retval None
 */
void appFSM_init(void);

/**
 * @brief  Evalua las entradas del estado actual (pulsador, audio disponible, tiempo),
 *         ejecuta sus acciones y cambia de estado si corresponde. Se llama en cada
 *         vuelta del while(1).
 * @param  None
 * @retval None
 */
void appFSM_update(void);

#endif /* INC_APP_H_ */
