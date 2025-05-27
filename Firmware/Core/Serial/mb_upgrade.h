/**
 * @file       mb_upgrade.h
 * @version    $(APP_VERSION)
 * @date       $(RELEASE_DATE)
 * @brief      Modbus RTU upgrade definition
 * @author     jan.bartovsky
 *
 * @copyright  Logic Elements Copyright
 *
 * @defgroup grMbUpgrade Modbus upgrade
 * @{
 * @brief Firmware upgrade for modbus RTU slave device
 *
 * This module contains implementation for reception of new firmware binary data
 * divided into several pages of given size. Writing is done using hand shake
 * principle. Written data from master are acknowledged by write-done flag. Slave
 * acknowledges the end of flash write procedure by status flag meaning the buffer is
 * free for new data.
 *
 * @par Main features:
 * - this features are to be integrated into modbus_slave module
 * - two callback must be implemented
 *  - Write page data (called after every page)
 *  - Write done (called at the end of transmission)
 *
 * @par Example
 * - see modbus_slave
 */
#ifndef MB_UPGRADE_H_
#define MB_UPGRADE_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Definitions----------------------------------------------------------------*/

/**
 * Base address of upgrade registers
 */
#define MB_UPGR_BASE_ADDRESS    1000

/**
 * Size of page in bytes
 */
#define MB_UPGR_PAGE_SIZE       64

/**
 * Last address of upgrade registers
 */
#define MB_UPGR_END_ADDRESS     (MB_UPGR_BASE_ADDRESS + MB_UPGR_PAGE_SIZE / 2 + 9)

/* Typedefs-------------------------------------------------------------------*/

typedef enum
{
  MB_UPGR_MODE_ERASE_ON_FLY = 0,
  MB_UPGR_MODE_ERASE_AT_START = 1,
  MB_UPGR_MODE_APPLY = 2,
}MbUpgr_Mode_t;


typedef enum
{
  MB_UPGR_STATUS_BUSY = 0,
  MB_UPGR_STATUS_READY = 1,
  MB_UPGR_STATUS_DONE_OK = 2,
  MB_UPGR_STATUS_DONE_ERROR = 3,
}MbUpgr_Status_t;

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize module, set status flag to ready.
 * @return STATUS_OK
 */
Status_t MbUpgr_Init(void);

/**
 * Periodic handle for upgrade module.
 *
 * This function checks the write done flag and if it is set, it calls application
 * callback that stores new data into flash. Then it sets status to ready.
 * If it is the last packet, proper callback is called.
 * @return Status of Write callback
 */
Status_t MbUpgr_Handle(void);

/**
 * Parse write register request for upgrade module
 * @param offset Modbus address
 * @param length Length of the request in registers
 * @param data Pointer to register data
 * @return STATUS_OK
 */
Status_t MbUpgr_WriteRegisters(uint16_t offset, uint16_t length, uint8_t* data);

/**
 * Parse read register request for upgrade module
 * @param offset Modbus address
 * @param length Length of read request in registers
 * @param data Pointer to register data
 * @return STATUS_OK
 */
Status_t MbUpgr_ReadRegisters(uint16_t offset, uint16_t length, uint8_t* data);

/**
 * Get type of binary file
 * @return Value of Type register
 */
uint16_t MbUpgr_GetType(void);

/**
 * Get Mode of upgrade
 * @return Value of the Mode
 */
uint16_t MbUpgr_GetMode(void);

/**
 * Callback prototype called after write into starting registers
 * @param type Received type if binary data
 * @param mode Mode of programming
 * @param size Size of binary
 * @return Status
 */
Status_t MbUpgr_HeaderClb(uint16_t type, uint16_t mode, uint32_t size);

/**
 * Callback prototype called after each received page
 * @param offset Total offset in bytes
 * @param data Pointer to data obtained
 * @param length Length of obtained data
 * @return Status
 */
Status_t MbUpgr_WritePageClb(uint32_t offset, uint8_t *data, uint16_t length);

/**
 * Callback prototype called after the complete binary is received
 * @param length Length of the binary
 * @return Status
 */
Status_t MbUpgr_WriteDoneClb(uint32_t length);

#endif /* MB_UPGRADE_H_ */
/** @} */
