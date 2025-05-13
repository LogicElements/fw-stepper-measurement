/**
 * @file       oscillogram.h
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
#ifndef MOTOR_OSCILLOGRAM_H_
#define MOTOR_OSCILLOGRAM_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Definitions----------------------------------------------------------------*/

#define OSC_BUFFER_LENGTH     4000

#define OSC_BUFFER_MIN        100

/* Typedefs-------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

Status_t Oscil_Init(void);

Status_t Oscil_Handle(void);

Status_t Oscil_NewData(uint16_t *data, uint16_t samples);

#endif /* MOTOR_OSCILLOGRAM_H_ */

/** @} */
