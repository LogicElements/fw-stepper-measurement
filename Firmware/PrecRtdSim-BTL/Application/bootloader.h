/**
  * @file       bootloader.h
  * @version    $(APP_VERSION)
  * @date       $(RELEASE_DATE)
  * @brief      Bootloader definitions
  * @author     jan.bartovsky
  *
  * @copyright  Logic Elements Copyright
  *
  * @defgroup grBootloader Bootloader
  * @{
  * @brief Bootloader application image handling
  *
  * This module contains functions to check new image in incoming image buffer, verify its integrity
  * and switch it into default position. The old image is copied into fall back image.
  * If factory button is pressed for enough time, a valid fallback image is restored into
  * default position.
  *
  * @par Main features:
  * - check new image in buffer
  * - accept new image and store old one in fallback buffer
  * - restore image from fallback buffer to default
  * - check consistency of current image
  *
  */
#ifndef APPLICATION_BOOTLOADER_H_
#define APPLICATION_BOOTLOADER_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Definitions----------------------------------------------------------------*/

/* Typedefs-------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize bootloader module
 *
 * Initialize factory test button input pin, CRC.
 * @return Status
 */
Status_t Btl_Init(void);

/**
 * Bootloader handle to be called when MCU is ready.
 *
 * This function does all the steps of bootloader (accept new image, restore fallback)
 * @return Status
 */
__attribute__((section (".sectionIsrConst")))
Status_t Btl_Handle(void);

/**
 * Start the default application image even if the consistency is compromised.
 * @return Status
 */
Status_t Btl_StartAppAnyway(void);

#endif /* APPLICATION_BOOTLOADER_H_ */
/** @} */
