/**
 * @file       system_bsp.c
 * @brief      System board-specific package implementation
 * @addtogroup grSystemBsp
 * @{
 */


/* Includes ------------------------------------------------------------------*/
#include "system_msp.h"
#include "common.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Extern address of binary start */
extern uint32_t _LD_ADDRESS_APPLICATION;
uint32_t APPLICATION_ADDRESS = (uint32_t)&_LD_ADDRESS_APPLICATION;

/* Public variables  ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/


void System_CrcInit(void)
{
  /* Initialize LL CRC */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);

  LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_NONE);
  LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_NONE);
  LL_CRC_SetInitialData(CRC, LL_CRC_DEFAULT_CRC_INITVALUE);
  LL_CRC_ResetCRCCalculationUnit(CRC);
}

void System_CrcClear(void)
{
  /* Reset CRC Calculation Unit */
  LL_CRC_ResetCRCCalculationUnit(CRC);
}

uint32_t System_CrcAccumulate(uint32_t *data, uint32_t length)
{
  uint32_t ret;

  /* Feed data into CRC unit */
  for (int i = 0; i < length; i++)
  {
    LL_CRC_FeedData32(CRC, data[i]);
  }
  /* Read back result data */
  ret = LL_CRC_ReadData32(CRC);

  return ret;
}



uint8_t System_Delay(uint32_t milliseconds)
{
  uint8_t ret = 0;
  HAL_Delay(milliseconds);
  return ret;
}

Status_t System_FlashErase(uint32_t startAddres, uint32_t endAddress)
{
  Status_t ret = STATUS_OK;
  FLASH_EraseInitTypeDef erase;
  uint32_t pageError;

  /* Unlock Flash memory */
  ret = HAL_FLASH_Unlock();

  /* Erase the given number of pages */
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.PageAddress = (uint32_t)startAddres;
  erase.NbPages = (endAddress - startAddres + (FLASH_PAGE_SIZE - 1)) / FLASH_PAGE_SIZE;
  ret = HAL_FLASHEx_Erase(&erase, &pageError);

  /* Lock Flash memory */
  HAL_FLASH_Lock();

  return ret;
}


Status_t System_FlashProgram(uint32_t addressOffset, uint8_t *data, uint32_t length)
{
  Status_t ret = STATUS_OK;
  uint32_t bytesWritten = 0;
  uint32_t value;

  /* Unlock Flash memory */
  ret = HAL_FLASH_Unlock();

  /* Address and data length must be multiple of 4 */
  if (((uint32_t)addressOffset % 4) || (length % 4))
  {
    ret = STATUS_ERROR;
  }

  while(ret == STATUS_OK && bytesWritten < length)
  {
    /* Write the data into memory */
    value = data[0] + ((uint32_t)data[1]<<8) + ((uint32_t)data[2]<<16) + ((uint32_t)data[3]<<24);
    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)addressOffset, value);
    bytesWritten += 4;
    addressOffset += 4;
    data += 4;
  }

  /* Lock Flash memory */
  HAL_FLASH_Lock();

  return ret;
}

Status_t System_IsFlashNotEmpty(uint32_t *address, uint32_t size)
{
  Status_t ret = STATUS_ERROR;
  uint32_t i = 0;

  /* Go through the given flash */
  while (i < size && ret == STATUS_ERROR)
  {
    /* If there is not a reset value, exit  */
    if (address[i/4] != 0xFFFFFFFF)
    {
      ret = STATUS_OK;
    }
    /* Increment step */
    i += 0x100;
  }

  return ret;
}


void System_RemapApplicationVector(void)
{
  volatile uint32_t *VectorTable = (volatile uint32_t *)0x20000000;

  /* Copy reset vector from flash into RAM */
  for (int i = 0; i < 48; i++)
  {
    VectorTable[i] = *(volatile uint32_t*)((uint32_t)APPLICATION_ADDRESS + (i << 2));
  }

  /* Remap vector from flash into RAM */
  __HAL_RCC_AHB_FORCE_RESET();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_AHB_RELEASE_RESET();
  __HAL_SYSCFG_REMAPMEMORY_SRAM();
}

void System_UseFlashVector(void)
{
  /* Remap vector from into flash */
  __HAL_RCC_AHB_FORCE_RESET();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_AHB_RELEASE_RESET();
  __HAL_SYSCFG_REMAPMEMORY_FLASH();
}



void System_StartApplication(uint32_t address)
{
  System_Callback_t jumpAddress;

  /* Prepare Jump address */
  jumpAddress = (System_Callback_t) (*((uint32_t*)(address + 0x4)));

  /* Stop Systick */
  SysTick->CTRL = 0;

  /* Disable all interrupts */
  __disable_irq();
  // Clear enable interrupt
  NVIC->ICER[0] = (__IO uint32_t)NVIC->ICER[0];

  // Clear pending interrupts
  NVIC->ICPR[0] = (__IO uint32_t)NVIC->ICPR[0];

  /* Set the reset vector */
  System_RemapApplicationVector();

  /* Initialize application Stack Pointer */
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

  /* Jump to the application finally ;) */
  jumpAddress();
}



/** @} */

