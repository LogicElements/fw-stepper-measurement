/**
  * @file       flash_conf.h
  * @version    $(APP_VERSION)
  * @date       $(RELEASE_DATE)
  * @brief      Flash memory configuration storage definition
  * @author     jan.bartovsky
  *
  * @copyright  Logic Elements Copyright
  *
  * @defgroup grFlashFs Flash memory configuration file system
  * @{
  * @brief Management of configuration files in external Flash memory
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
#ifndef BSP_FLASH_CONF_H_
#define BSP_FLASH_CONF_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"
#include "configuration.h"

/* Definitions----------------------------------------------------------------*/

/**
 * Maximal allowed length of configuration stream plus some reserve
 */
#define FLASH_CONF_SIZE      (1024)

/* Typedefs-------------------------------------------------------------------*/

typedef struct
{
  uint32_t addressNext;       ///< Address of the next configuration data (in flash memory space)
  uint32_t writes;            ///< Number of configuration writes. Incremented after every write operation
  uint32_t completeSize;      ///< Size of the entire configuration (including header)
  uint32_t reserved;           ///< reserved
}FlashConf_Header_t;


typedef struct
{
  FlashConf_Header_t header;
  uint8_t data[FLASH_CONF_SIZE];
}FlashConf_Entry_t;


/* Functions -----------------------------------------------------------------*/

/**
 * Initialize Flash Configuration storage
 * @return STATUS_OK on success
 * @return STATUS_ERROR on failure
 */
Status_t FlashConf_Init(void);

/**
 * Store configuration into Flash memory
 * @return STATUS_OK on success
 * @return STATUS_ERROR on failure
 */
Status_t FlashConf_Store(void);

/**
 * Load configuration from Flash memory
 * @return STATUS_OK on success
 * @return STATUS_ERROR on failure
 */
Status_t FlashConf_Load(void);

/**
 * Perform N + 5 write and find entry cycles to verify that memory is used properly
 * and that restart at any time does not drop any values
 * @return STATUS_OK on success
 * @return STATUS_ERROR on failure
 */
Status_t FlashConf_Test(void);


#endif /* BSP_FLASH_CONF_H_ */
/** @} */
