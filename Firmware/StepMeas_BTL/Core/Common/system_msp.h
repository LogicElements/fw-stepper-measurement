/**
  * @file       system_msp.h
  * @version    1.0
  * @date       07-12-2015
  * @brief      System MCU-specific package definitions
  *
  * @copyright  Logic Elements Copyright
  *
  * @defgroup grSystemMsp System MSP (platform dependent)
  * @{
  * @brief System MCU-specific package
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

/**
 * General system pointer to function type
 */
typedef  void (*System_Callback_t)(void);


/* Functions -----------------------------------------------------------------*/

void System_RemapVector(void);

Status_t System_ReloadWdg(void);

/**
 * Configure the system clocks.
 */
void SystemClock_Config(void);

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
 * Software reset.
 */
void System_Reset(void);

/**
 * Unlock Flash interface for programming.
 */
void System_FlashEnable(void);

/**
 * Lock Flash interface for programming
 */
void System_FlashDisable(void);

/**
 * Program single 32-bit word to Flash
 * @param address Target address in Flash
 * @param data Data to write
 * @return HAL status
 */
int16_t System_FlashProgramWord(uint32_t address, uint32_t data);

/**
 * Program a bunch of data into Flash memory
 * @param address Address in Flash memory where the data should be stored
 * @param data Pointer to the data to write to Flash
 * @param dataLength Length of the data to write
 * @return Status
 */
int16_t System_FlashProgram(uint32_t address, uint8_t *data, uint32_t dataLength);

/**
 * Erase the selected Flash sector
 * @param[in] startAddr Start address of the memory to erase
 * @param[in] endAddr End address of the memory to erase
 * @return HAL status
 */
int16_t System_FlashErase(uint32_t startAddr, uint32_t endAddr);


/**
 * Check if the flash is empty
 * @param address Start address to check
 * @param size Size of the flash to check
 * @return Result (OK - empty, ERROR - not empty)
 */
Status_t System_IsFlashNotEmpty(uint32_t *address, uint32_t size);


/**
 * System delay based on system tick.
 * @param miliseconds Delay in msec.
 */
void System_Delay(uint32_t miliseconds);

/**
 * Get the current system tick count
 * @return System tick count
 */
uint32_t System_GetTick(void);

/**
 * Initialize CRC unit. Stm32F4 has no CRC settings
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
 * @return
 */
uint32_t System_CrcAccumulate(uint32_t *data, uint32_t length);

/**
 * Get random number using RNG
 * @return Random number
 */
uint32_t System_GetRandomNumber(void);

#endif /* SYSTEM_MSP_H_ */

/** @} */

