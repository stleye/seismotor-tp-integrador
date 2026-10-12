/*
 * API_I2C.h
 *
 *  Created on: 8 abr. 2023
 *      Author: ipave
 */

#ifndef API_INC_API_I2C_H_

#include <stdio.h>
#include "stm32f4xx_hal.h"  		/* <- HAL include */
//#include "stm32f4xx_nucleo_144.h" 	/* <- BSP include */
#include <stdbool.h>

#define I2C_CLOCK_RATE 100000


_Bool I2C_HW_init(void);

#endif /* API_INC_API_I2C_H_ */
