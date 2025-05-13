/**
 * @file       system_bsp.c
 * @brief      System board-specific package implementation
 * @addtogroup grSystemBsp
 * @{
 */


/* Includes ------------------------------------------------------------------*/
#include "system_msp.h"
#include "common.h"
#include "configuration.h"
#include "main.h"

#include "stm32f0xx_ll_rcc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables  ---------------------------------------------------------*/

static CRC_HandleTypeDef hcrc;
static IWDG_HandleTypeDef hiwdg;

/* Public function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/**
  * @brief System Clock Configuration
  * @retval None
  */
//void SystemClock_Config(void)
//{
//
//  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);
//
//  if(__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_1)
//  {
//    Error_Handler();
//  }
//
//  /* Enable HSE */
//  LL_RCC_HSE_Enable();
//  while (LL_RCC_HSE_IsReady() != 1)
//  {
//  }
//
//  /* Enable HSI14 for ADC */
//  LL_RCC_HSI14_Enable();
//  while (LL_RCC_HSI14_IsReady() != 1)
//  {
//  }
//  LL_RCC_HSI14_SetCalibTrimming(16);
//
//  /* Enable LSI */
//  LL_RCC_LSI_Enable();
//  while (LL_RCC_LSI_IsReady() != 1)
//  {
//  }
//  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLL_MUL_3, LL_RCC_PREDIV_DIV_1);
//  LL_RCC_PLL_Enable();
//
//  /* Wait till PLL is ready */
//  while (LL_RCC_PLL_IsReady() != 1)
//  {
//  }
//
//  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
//  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
//  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
//
//  /* Wait till System clock is ready */
//  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
//  {
//  }
//
//  SystemCoreClock = 48000000;
//  LL_RCC_HSI14_EnableADCControl();
//  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
//  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
//
//  /* SysTick_IRQn interrupt configuration */
//  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
//  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
//  HAL_NVIC_SetPriority(SysTick_IRQn, PRIO_IRQ_SYSTICK, 0);
//
//  /* GPIO Ports Clock Enable */
//  __HAL_RCC_GPIOF_CLK_ENABLE();
//  __HAL_RCC_GPIOA_CLK_ENABLE();
//  __HAL_RCC_GPIOB_CLK_ENABLE();
//  __HAL_RCC_GPIOC_CLK_ENABLE();
//
//  /* Freeze IWDG in stop */
//  __HAL_RCC_DBGMCU_CLK_ENABLE();
//  __HAL_DBGMCU_FREEZE_IWDG();
//}


void System_CrcInit(void)
{
  /* Initialize CRC unit */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;

  __HAL_RCC_CRC_CLK_ENABLE();
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_CRC_DR_RESET(&hcrc);
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

Status_t System_InitWdg(void)
{
  Status_t ret = STATUS_OK;

  /* 40000 Hz / 32 = 1250 counts per second
   * 3750 = 3 s*/
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Window = 3750;
  hiwdg.Init.Reload = 3750;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  System_ReloadWdg();

  return ret;
}

Status_t System_ReloadWdg(void)
{
  Status_t ret = STATUS_OK;

  HAL_IWDG_Refresh(&hiwdg);

  return ret;
}


uint8_t System_Delay(uint32_t milliseconds)
{
  HAL_Delay(milliseconds);
  return 0;
}


void System_Reset(void)
{
  NVIC_SystemReset();
}

Status_t System_FlashErase(uint32_t startAddres, uint32_t endAddress)
{
  Status_t ret = STATUS_OK;
  FLASH_EraseInitTypeDef erase;
  uint32_t pageError;

  /* Unlock Flash memory */
  ret = HAL_FLASH_Unlock();

  /* Erase the given flash pages */
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
    /* Write a single word */
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


Status_t System_VerifyImage(uint32_t *address)
{
  Status_t ret = STATUS_OK;
  uint32_t checksum = *(address + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 2);
  uint32_t calcChecksum;
  uint32_t size = *(address + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 3);

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
    System_CrcAccumulate((uint32_t*)CONF_FIRMWARE_INFO_DEFAULT, 8);
    calcChecksum = System_CrcAccumulate(address + (uint32_t)CONF_FW_INFO_OFFSET / 4 + 8, (size - (uint32_t)CONF_FW_INFO_OFFSET - 32) / 4);

    /* If CRC is different */
    if (calcChecksum != checksum)
    {
      ret = STATUS_ERROR;
    }
  }

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
    VectorTable[i] = *(volatile uint32_t*)((uint32_t)CONF_C_APPLICATION_OFFSET + (i << 2));
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
  uint32_t stackPointer;

  /* Prepare Jump address */
  jumpAddress = (System_Callback_t) (*((uint32_t*)(address + 0x4)));
  stackPointer = *((uint32_t*)(address));

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
  __set_MSP(stackPointer);

  /* Jump to the application finally ;) */
  jumpAddress();
}



/** @} */

