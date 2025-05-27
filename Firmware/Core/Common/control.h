/**
 * @file       control.h
 * @version    $(APP_VERSION)
 * @date       $(RELEASE_DATE)
 * @brief      Command register and state machine definition
 * @author     jan.bartovsky
 *
 * @copyright  Logic Elements Copyright
 *
 * @defgroup grControl Control
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
#ifndef APPLICATION_BSP_CONTROL_H_
#define APPLICATION_BSP_CONTROL_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Definitions----------------------------------------------------------------*/

/* Typedefs-------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize control module
 * @return Status OK
 */
Status_t Control_Init(void);

/**
 * Periodic control handle
 *
 * This function handles Command register and sets LED color from Status register
 * @return Status
 */
Status_t Control_Handle(void);


#endif /* APPLICATION_BSP_CONTROL_H_ */
/** @} */
