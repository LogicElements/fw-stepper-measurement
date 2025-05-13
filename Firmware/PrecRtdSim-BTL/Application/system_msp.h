/**
 * @file       system_msp.h
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
#ifndef SYSTEM_MSP_H_
#define SYSTEM_MSP_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Definitions----------------------------------------------------------------*/

/* Typedefs-------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize CRC unit with default settings and word computation.
 */
void System_CrcInit(void);

/**
 * Clear CRC computation.
 */
void System_CrcClear(void);

/**
 * Calculate CRC computation.
 * @param data Pointer to data to calculate
 * @param length Length of the data
 * @return Current CRC value
 */
uint32_t System_CrcAccumulate(uint32_t *data, uint32_t length);

/**
 * System delay based on system tick.
 * @param miliseconds Delay in msec.
 */
uint8_t System_Delay(uint32_t milliseconds);

/**
 * Program a bunch of data into Flash memory
 * @param address Address in Flash memory where the data should be stored
 * @param data Pointer to the data to write to Flash
 * @param dataLength Length of the data to write
 * @return Status
 */
Status_t System_FlashProgram(uint32_t addressOffset, uint8_t *data, uint32_t length);

/**
 * Erase the selected Flash sector
 * @param[in] startAddr Start address of the memory to erase
 * @param[in] endAddr End address of the memory to erase
 * @return HAL status
 */
Status_t System_FlashErase(uint32_t startAddres, uint32_t endAddress);

/**
 * Check if the flash is empty
 * @param address Start address to check
 * @param size Size of the flash to check
 * @return Result (OK - empty, ERROR - not empty)
 */
Status_t System_IsFlashNotEmpty(uint32_t *address, uint32_t size);

/**
 * Start application at specified address
 *
 * This function is used by bootloaders and reset managers to jump to
 * an application image located at the specified address.
 * Before jumping this function disables all interrupts, sets Stack Pointer,
 * sets the Reset Vector, and jumps to the application address + 4.
 * @param address Beginning address of the application to run
 */
void System_StartApplication(uint32_t address);

/**
 * Remap reset vector to flash
 */
void System_UseFlashVector(void);

#endif /* SYSTEM_MSP_H_ */
/** @} */
