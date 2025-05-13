/**
 * @file       led.h
 * @version    $(APP_VERSION)
 * @date       $(RELEASE_DATE)
 * @brief      file_brief
 * @author     jan.bartovsky
 *
 * @copyright  Logic Elements Copyright
 *
 * @defgroup gr group_name
 * @{
 * @brief group_brief
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
#ifndef BSP_LED_H_
#define BSP_LED_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Definitions----------------------------------------------------------------*/

/* Typedefs-------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

Status_t Led_Init(void);

Status_t Led_GreenOn(void);

Status_t Led_GreenOff(void);

Status_t Led_GreenToggle(void);

Status_t Led_RedOn(void);

Status_t Led_RedOff(void);


#endif /* BSP_LED_H_ */
/** @} */
