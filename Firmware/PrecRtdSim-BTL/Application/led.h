/**
  * @file       led.h
  * @version    $(APP_VERSION)
  * @date       $(RELEASE_DATE)
  * @brief      LED driver definition
  * @author     jan.bartovsky
  *
  * @copyright  Logic Elements Copyright
  *
  * @defgroup grLed LED driver
  * @{
  * @brief API for driving LED diodes
  *
  * This module contains
  *
  * @par Main features:
  *
  * @par Example
  * @code
  *
  * @endcode
  */
#ifndef LED_H_
#define LED_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Definitions----------------------------------------------------------------*/


/*
 * LED ports definition
 */
#define LED_R_PORT        GPIOB
#define LED_G_PORT        GPIOB

#define LED_R_PIN         LL_GPIO_PIN_3
#define LED_G_PIN         LL_GPIO_PIN_4

/* Typedefs-------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize LED peripherals (GPIO, TIM) and starts blinking
 * @return Status
 */
Status_t Led_Init(void);



#endif /* LED_H_ */
/** @} */
