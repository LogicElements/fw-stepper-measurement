/**
 * @file       bootloader.c
 * @brief      Bootloader implementation
 * @addtogroup grBootloader
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "bootloader.h"
#include "system_msp.h"

/* Private defines -----------------------------------------------------------*/

/**
 * SIR1 project target device ID
 */
#define CONF_TARGET_DEVICE            9153

/* Private macros  -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/

/**
 * Empty structure of firmware information (used for CRC computation)
 */
const uint32_t CONF_FIRMWARE_INFO[8] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

/**
 * Structure of Factory information
 */
const uint32_t __attribute__((section (".sectionFwInfo"))) CONF_FACTORY_INFO[8] = {
    0xffffffff,     // Default serial number
    CONF_TARGET_DEVICE,   // Default device ID
    1001,   // Default Hardware revision
    1001,   // Default Bootloader revision
    CONF_TARGET_DEVICE,   // Default SN prefix
    0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF,  //
};

/**
 * Offset of Firmware information (from linker)
 */
extern const uint8_t CONF_FW_INFO_OFFSET[] asm("_LD_FW_INFO_OFFSET");

/**
 * Address of application image (from linker)
 */
extern const uint8_t CONF_C_APPLICATION_OFFSET[] asm("_LD_ADDRESS_APPLICATION");

/**
 * Address of bootloader image (from linker)
 */
extern const uint8_t CONF_C_BOOTLOADER_OFFSET[]     asm("_LD_ADDRESS_BOOTLOADER");

/**
 * Address of application buffer where new image is stored (from linker)
 */
extern const uint8_t CONF_C_APP_BUFFER_OFFSET[] asm("_LD_ADDRESS_BUFFER_APP");

/**
 * Maximal size of application image (from linker)
 */
extern const uint8_t CONF_C_APPLICATION_MAX_SIZE[] asm("_LD_SIZE_APPLICATION");

/**
 * Maximal size of application image (from linker)
 */
extern const uint8_t CONF_C_APP_BUFFER_MAX_SIZE[] asm("_LD_SIZE_BUFFER_APP");


/* Private variables ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
 * Copy whole image
 * @param source Pointer to source image
 * @param dest Pointer to the destination image
 * @return Status
 */
Status_t Btl_CopyImage(uint32_t *source, uint32_t *dest);

/**
 * Verify image consistency (CRC)
 * @param address Address of the image to check
 * @return Status (STATUS_OK if image is consistent)
 */
__attribute__((section (".sectionIsrConst"))) Status_t Btl_VerifyImage(uint32_t *address);

/* Functions -----------------------------------------------------------------*/

Status_t Btl_Init(void)
{
  Status_t ret = STATUS_OK;

  /* Initialize CRC peripheral */
  System_CrcInit();

  return ret;
}


Status_t Btl_Handle(void)
{
  Status_t ret = STATUS_OK;
  uint32_t size;

  /* Check if the buffer contains valid image */
  ret = Btl_VerifyImage((uint32_t *)CONF_C_APP_BUFFER_OFFSET);

  /* Buffer is valid */
  if (ret == STATUS_OK)
  {
    /* Get size of the buffered app */
    size = *((uint32_t*)CONF_C_APP_BUFFER_OFFSET + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 3);

    /* Erase application flash */
    ret += System_FlashErase((uint32_t)CONF_C_APPLICATION_OFFSET, (uint32_t)CONF_C_APPLICATION_OFFSET + size);

    /* Copy buffered app into application flash */
    ret += Btl_CopyImage((uint32_t*)CONF_C_APP_BUFFER_OFFSET, (uint32_t*)CONF_C_APPLICATION_OFFSET);

    /* Verify copied image one more time */
    if (Btl_VerifyImage((uint32_t*)CONF_C_APPLICATION_OFFSET) == STATUS_OK)
    {
      /* Erase buffer flash, so the next restart does not copy it again, and flasher has empty buffer */
      ret += System_FlashErase((uint32_t)CONF_C_APP_BUFFER_OFFSET, (uint32_t)CONF_C_APP_BUFFER_OFFSET + (uint32_t)CONF_C_APP_BUFFER_MAX_SIZE);
    }
  }
  else
  {
    ret = STATUS_OK;
  }

  /* Standard application verification */
  ret = Btl_VerifyImage((uint32_t*)CONF_C_APPLICATION_OFFSET);

  /* If application image is valid, jump to the application */
  if (ret == STATUS_OK)
  {
    System_Delay(500);
    System_StartApplication((uint32_t)CONF_C_APPLICATION_OFFSET);
  }

  return ret;
}

Status_t Btl_StartAppAnyway(void)
{
  Status_t ret = STATUS_OK;

  System_StartApplication((uint32_t)CONF_C_APPLICATION_OFFSET);

  return ret;
}

/* Private Functions ---------------------------------------------------------*/


Status_t Btl_VerifyImage(uint32_t *address)
{
  Status_t ret = STATUS_OK;
  uint32_t checksum = *(address + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 2);
  uint32_t calcChecksum;
  uint32_t size = *(address + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 3);
  uint32_t devId = *(uint32_t*)((uint32_t)CONF_C_BOOTLOADER_OFFSET + (uint32_t)CONF_FW_INFO_OFFSET + 4);
  uint32_t targetDev = *(address + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 8);

  /* If size is greater than MAX value */
  if (size > (uint32_t)CONF_C_APPLICATION_MAX_SIZE)
  {
    ret = STATUS_ERROR;
  }
  else
  {
    /* If flash is not empty */
    ret = System_IsFlashNotEmpty(address, (uint32_t)size);
  }

  if (ret == STATUS_OK)
  {
    /* Calculate CRC. CRC skips the Firmware information field, uses all 0xFF */
    System_CrcClear();
    System_CrcAccumulate(address, (uint32_t)CONF_FW_INFO_OFFSET / 4);
    System_CrcAccumulate((uint32_t*)CONF_FIRMWARE_INFO, 8);
    calcChecksum = System_CrcAccumulate(address + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 8, (size - (uint32_t)CONF_FW_INFO_OFFSET - 32) / 4);

    /* If CRC is different */
    if (calcChecksum != checksum)
    {
      ret = STATUS_ERROR;
    }
    /* Device Id not match */
    if (targetDev != devId)
    {
      ret = STATUS_ERROR;
    }
  }

  return ret;
}

Status_t Btl_CopyImage(uint32_t *source, uint32_t *dest)
{
  Status_t ret = STATUS_OK;
  /* Take size of source application */
  uint32_t size = *(source + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 3);

  /* Write source app into destination */
  ret = System_FlashProgram((uint32_t) dest, (uint8_t *)source, size);

  return ret;
}



/** @} */
