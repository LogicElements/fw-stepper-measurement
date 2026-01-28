/**
 * @file       system_msp.c
 * @brief      System MCU-specific package implementation
 * @addtogroup grSystemMsp
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include "system_msp.h"



/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static CRC_HandleTypeDef hcrc;

/* Private function prototypes -----------------------------------------------*/

static uint32_t System_GetSector(uint32_t address);

/* Public function prototypes -----------------------------------------------*/
/**
 * Weak prototype of user hook function on entering low-power Stop mode.
 */
__weak void System_EnterStopMode(void);

/**
 * Weak prototype of user hook function on exiting low-power Stop mode.
 */
__weak void System_ExitStopMode(void);


int16_t System_GetZoneFromTimestamp(uint32_t timestamp);

/* Public functions ----------------------------------------------------------*/




void System_CrcInit(void)
{
  hcrc.Instance = CRC;

  __HAL_RCC_CRC_CLK_ENABLE();
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
}

void System_CrcClear(void)
{
  /* Reset CRC Calculation Unit */
  __HAL_CRC_DR_RESET(&hcrc);
}

uint32_t System_CrcAccumulate(uint32_t *data, uint32_t length)
{
  return HAL_CRC_Accumulate(&hcrc, data, length);
}

//uint32_t System_GetRandomNumber(void)
//{
//  uint32_t countdown = 40;  // 40 cycles max (see RefMan)
//  RNG->CR = 0x04;
//
//  while(((RNG->SR & 0x01) == 0) && countdown != 0)
//    countdown--;
//
//  RNG->CR = 0x00;
//  RNG->SR = 0x00;
//  return RNG->DR;
//}


Status_t System_ReloadWdg(void)
{
  Status_t ret = STATUS_OK;

  IWDG->KR = IWDG_KEY_RELOAD;

  return ret;
}


void System_StartApplication(uint32_t address)
{
  uint32_t JumpAddress;
  System_Callback_t jump_to_application;

  /* Prepare Jump address */
  JumpAddress = *(uint32_t*) (address + 4);
  jump_to_application = (System_Callback_t) (JumpAddress);

  /* Disable all interrupts */
  /* + 0 is a workaround of "assignment to itself code check" */
  NVIC->ICER[0] = NVIC->ICER[0] + 0;
  NVIC->ICER[1] = NVIC->ICER[1] + 0;
  NVIC->ICER[2] = NVIC->ICER[2] + 0;

  /* Clear pending interrupts */
  /* + 0 is a workaround of "assignment to itself code check" */
  NVIC->ICPR[0] = NVIC->ICPR[0] + 0;
  NVIC->ICPR[1] = NVIC->ICPR[1] + 0;
  NVIC->ICPR[2] = NVIC->ICPR[2] + 0;

  /* Set the reset vector */
  SCB->VTOR = address;

  /* Initialize application Stack Pointer */
  __set_MSP(*(uint32_t*) address);

  /* Jump to the application finally ;) */
  jump_to_application();
}


void System_Reset(void)
{
  NVIC_SystemReset();
}

void System_FlashEnable(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
}

void System_FlashDisable(void)
{
  HAL_FLASH_Lock();
}

int16_t System_FlashProgramWord(uint32_t address, uint32_t data)
{
  HAL_StatusTypeDef ret;

  ret = HAL_FLASH_Program(TYPEPROGRAM_WORD, address, data);

  return (Status_t) ret;
}

int16_t System_FlashProgram(uint32_t address, uint8_t *data, uint32_t dataLength)
{
  Status_t ret = STATUS_OK;
  uint32_t bytesWritten = 0;
  uint32_t value;

  /* Unlock Flash memory */
  ret = HAL_FLASH_Unlock();

  /* Address and data length must be multiple of 4 */
  if (((uint32_t)address % 4) || (dataLength % 4))
  {
    ret = STATUS_ERROR;
  }

  while(ret == STATUS_OK && bytesWritten < dataLength)
  {
    /* Write a single word */
    value = data[0] + ((uint32_t)data[1]<<8) + ((uint32_t)data[2]<<16) + ((uint32_t)data[3]<<24);
    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)address, value);
    bytesWritten += 4;
    address += 4;
    data += 4;
  }

  /* Lock Flash memory */
  HAL_FLASH_Lock();

  return (int16_t) ret;
}

int16_t System_FlashErase(uint32_t startAddr, uint32_t endAddr)
{
  HAL_StatusTypeDef ret;
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;
  uint32_t FirstSector = 0;
  uint32_t NbOfSectors = 0;

  /* Get the First sector ID to erase */
  FirstSector = System_GetSector(startAddr);

  /* Get the number of sectors to erase */
  NbOfSectors = System_GetSector(endAddr) - FirstSector + 1;

  /* Configure the sector erase structure */
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = FirstSector;
  EraseInitStruct.NbSectors = NbOfSectors;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  /* Enable flash access, erase sector, disable flash access */
  System_FlashEnable();
  ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
  System_FlashDisable();

  return (int16_t) ret;
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
    i += 0x10;
  }

  return ret;
}


void System_Delay(uint32_t milliseconds)
{
  HAL_Delay(milliseconds);
}

uint32_t System_GetTick(void)
{
  return HAL_GetTick();
}



/* Private functions ----------------------------------------------------------*/

/**
 * @brief  Gets the sector of a given address
 * @param  None
 * @retval The sector of a given address
 */
static uint32_t System_GetSector(uint32_t address)
{
  if (address < 0x08004000)
    return FLASH_SECTOR_0;
  else if (address < 0x08008000)
    return FLASH_SECTOR_1;
  else if (address < 0x0800C000)
    return FLASH_SECTOR_2;
  else if (address < 0x08010000)
    return FLASH_SECTOR_3;
  else if (address < 0x08020000)
    return FLASH_SECTOR_4;
  else if (address < 0x08040000)
    return FLASH_SECTOR_5;
  else if (address < 0x08060000)
    return FLASH_SECTOR_6;
  else if (address < 0x08080000)
    return FLASH_SECTOR_7;
  else
    return UINT32_MAX;
}

/** @} */

