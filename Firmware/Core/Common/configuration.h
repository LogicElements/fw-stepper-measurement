/**
  * @file       configuration.h
  * @version    $(APP_VERSION)
  * @date       $(RELEASE_DATE)
 * @brief      Configuration and tools for register map definition
  * @author     jan.bartovsky
  *
  * @copyright  Logic Elements Copyright
  *
 * @defgroup grGonfig Configuration
  * @{
 * @brief Tools for accessing configuration and register map
  *
  * This module contains
  *
  * @par Main features:
 * - Access macros to the registers according to its type
 * - Flash read and write API
 * - Callback for register change
 * - Synchronization methods
 *
  */
#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"

#include "reg_map.h"

/* Definitions----------------------------------------------------------------*/

/**
 * Target device ID
 */
#define CONF_TARGET_DEVICE            2051   //< Set proper target device ID

/* Macros --------------------------------------------------------------------*/

/**
 * Get block number of the register
 */
#define CONF_BLOCK_ID(id) (((id) & 0xFF000000) >> 24)

/**
 * Get address of the register within the block
 */
#define CONF_ADDR_ID(id)  (((id) & 0x00FFF000) >> 12)

/**
 * Get byte length of the register
 */
#define CONF_BYTE_LEN_ID(id)  (CONF_LENGTH[((id) & 0x0000000F)])

/**
 * Get variable type of the register
 */
#define CONF_TYPE_ID(id)    (((id) & 0x0F00) >> 8)

/**
 * Get uint8_t * pointer of the beginning of the register
 */
#define CONF_PTR(id) (CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id))

/**
 * Get/set uint32_t value of the register
 */
#define CONF_INT(id) (*((uint32_t *)(CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id))))

/**
 * Get/set uint16_t value of the register
 */
#define CONF_SHORT(id) (*((uint16_t *)(CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id))))

/**
 * Get/set uint8_t value of the register
 */
#define CONF_BYTE(id) (*((uint8_t *)(CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id))))

/**
 * Get/set floating point value of the register
 */
#define CONF_FLOAT(id) (*((float *)(CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id))))

/*
 * Circuit-dependent macros
 */

/**
 * Get uint8_t* pointer to variable of given circuit
 */
#define CONF_PTR_C(id, c) (CONF_REG[CONF_BLOCK_ID(id) + (c)] + CONF_ADDR_ID(id))

/**
 * Get/set uint32_t pointer to variable of given circuit
 */
#define CONF_INT_C(id, c) (*((uint32_t *)(CONF_REG[CONF_BLOCK_ID(id) + (c)] + CONF_ADDR_ID(id))))

/**
 * Get/set uint16_t pointer to variable of given circuit
 */
#define CONF_SHORT_C(id, c) (*((uint16_t *)(CONF_REG[CONF_BLOCK_ID(id) + (c)] + CONF_ADDR_ID(id))))

/**
 * Get/set uint8_t pointer to variable of given circuit
 */
#define CONF_BYTE_C(id, c) (*((uint8_t *)(CONF_REG[CONF_BLOCK_ID(id) + (c)] + CONF_ADDR_ID(id))))

/**
 * Get/set float pointer to variable of given circuit
 */
#define CONF_FLOAT_C(id, c) (*((float *)(CONF_REG[CONF_BLOCK_ID(id) + (c)] + CONF_ADDR_ID(id))))

/**
 * Get/set uint8_t* pointer to variable of given circuit
 */
#define CONF_ID_C(id, c)  ((id) + ((c) * 0x01000000))

/*
 * Array access macros
 */

/**
 * Get floating point value of the register within an packed array
 */
#define CONF_FLOAT_A(id, a) (*((float *)(CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id) + (a) * sizeof(float))))

/**
 * Get uint32_t value of the register within an packed array
 */
#define CONF_INT_A(id, a) (*((uint32_t *)(CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id) + (a) * sizeof(uint32_t))))

/**
 * Get uint16_t value of the register within an packed array
 */
#define CONF_SHORT_A(id, a) (*((uint16_t *)(CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id) + (a) * sizeof(uint16_t))))

/**
 * Get uint8_t value of the register within an packed array
 */
#define CONF_BYTE_A(id, a) (*((uint8_t *)(CONF_REG[CONF_BLOCK_ID(id)] + CONF_ADDR_ID(id) + (a) * sizeof(uint8_t))))

/* Constants -------------------------------------------------------------------*/

/*
 * External public constant variables, defined in reg_map.c
 */

/**
 * Structure of configuration
 */
extern conf_reg_t conf;

/**
 * Register map storage
 */
extern uint8_t* const CONF_REG[CONF_REG_BLOCK_NUMBER];

/**
 * Vector of registers to log
 */
extern const uint32_t CONF_REG_LOGGER[CONF_REG_LOGGER_NUMBER];

/**
 * Vector of registers to report (Send as statistics with extended period)
 */
extern const uint32_t CONF_REG_CALIB[CONF_REG_CALIB_NUMBER];

/**
 * Vector of registers to synchronize
 */
extern const uint32_t CONF_REG_SYNCED[CONF_REG_SYNCED_NUMBER];

/**
 * Firmware information block in Flash memory
 */
extern const uint32_t CONF_FIRMWARE_INFO[9];

/**
 * Default (empty) values of firmware information block
 */
extern const uint32_t CONF_FIRMWARE_INFO_DEFAULT[8];

/**
 * Vector of register lengths
 */
extern const uint32_t CONF_LENGTH[16];


/*
 * External public constant variables, defined in configuration.c or linker script.
 */
extern const uint8_t CONF_FW_INFO_OFFSET[]          asm("_LD_FW_INFO_OFFSET");
extern const uint8_t CONF_C_BOOTLOADER_OFFSET[]     asm("_LD_ADDRESS_BOOTLOADER");
extern const uint8_t CONF_C_CALIBRATION_OFFSET[]    asm("_LD_ADDRESS_CALIBRATION");
extern const uint8_t CONF_C_APPLICATION_OFFSET[]    asm("_LD_ADDRESS_APPLICATION");
extern const uint8_t CONF_C_APP_BUFFER_OFFSET[]     asm("_LD_ADDRESS_BUFFER_APP");
extern const uint8_t CONF_C_CONFIGURATION_OFFSET[]  asm("_LD_ADDRESS_CONFIG");
extern const uint8_t CONF_C_APPLICATION_MAX_SIZE[]  asm("_LD_SIZE_BUFFER_APP");
extern const uint8_t CONF_C_CONFIGURATION_SIZE[]    asm("_LD_SIZE_CONFIGURATION");
extern const uint8_t CONF_C_CALIBRATION_SIZE[]      asm("_LD_SIZE_CALIBRATION");

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize configuration and register map
 * @return Status
 */
Status_t Config_Init(void);

/**
 * Check limit address of register ID
 * @param id Register ID to check
 * @return Status - OK if ID is within limits
 */
Status_t Config_CheckLimits(uint32_t id);

/**
 * Notify about change of configuration register value
 * @param id ID of the modified register
 * @return Status
 */
Status_t Config_ApplyConfig(uint32_t id);

/**
 * Read configuration stream (previously read from flash) and copy all known register values to their respective location.
 * @param data Buffer of the configuration stream
 * @param length Length of the stream
 * @return Status
 */
Status_t Config_ReadStream(uint8_t *data, uint32_t length);

/**
 * Create configuration stream for further storage to Flash memory.
 *
 * This function takes all registers that need to be stored in flash memory and create a stream
 * @param data Pointer to the buffer to store stream
 * @param length Result length of the stream
 * @param maxLength Length of the buffer - maximal length of the stream
 * @return Status
 */
Status_t Config_FillStream(uint8_t *data, uint32_t *length, uint32_t maxLength);

/**
 * Getter of factory request
 * @return STATUS_OK if factory reset is requested.
 */
Status_t Config_IsFactoryRequest(void);

#endif /* CONFIGURATION_H_ */
/** @} */
