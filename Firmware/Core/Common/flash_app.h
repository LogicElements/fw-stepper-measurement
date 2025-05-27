/**
  * @file       flash_app.h
  * @version    $(APP_VERSION)
  * @date       $(RELEASE_DATE)
  * @brief      External flash application task implementation
  * @author     jan.bartovsky
  *
  * @copyright  Logic Elements Copyright
  *
  * @defgroup grFlashApp Flash memory task
  * @{
  * @brief Application task for handling external flash memory
  *
  *
  * @endcode
  */
#ifndef BSP_FLASH_APP_H_
#define BSP_FLASH_APP_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Definitions----------------------------------------------------------------*/

/* Typedefs-------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize flash application module
 *
 * This function initializes configuration module and loads the most recent configuration
 * @return Status
 */
Status_t FlashApp_Init(void);

/**
 * Periodic handle of flash application module
 *
 * This function handles configuration requests and factory reset requests
 * @return Status
 */
Status_t FlashApp_Handle(void);

/**
 * Perform immediate factory values revert and return without restarting
 * @return Status of new configuration storage
 */
Status_t FlashApp_StartupFactoryReset(void);

/**
 * Request flash task to write down the configuration because it has been changed.
 *
 * @return Status
 */
Status_t FlashApp_RequestConfStorage(void);


/** Request flash task to erase complete Flash memory and restore factory settings.
 *
 * @return Status
 */
Status_t FlashApp_RequestFactorySettings(void);

#endif /* BSP_FLASH_APP_H_ */
/** @} */
