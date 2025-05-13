/**
 * @file       control.c
 * @brief      Command register and state machine implementation
 * @addtogroup grControl
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "control.h"
#include "configuration.h"
#include "led.h"
#include "system_msp.h"
#include "flash_app.h"
#include "switch.h"

/* Private defines -----------------------------------------------------------*/

/*
 * Command keys
 */
#define CONTROL_RESET       9901
#define CONTROL_FACTORY     8801
#define CONTROL_TESTING     7701
#define CONTROL_ERROR       6600
#define CONTROL_WDG_LATCH   5501

/**
 * Modbus base address
 */
#define MODBUS_BASE_ADDRESS       32

/**
 * Led behavior
 */

#define LED_PERIOD      20
#define LED_BLINK       4

/* Private macros  -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/

/**
 * Declaration of all private variables
 */
typedef struct
{
  uint16_t prevCmd;
  uint16_t localError;
  uint32_t prevTick;
  uint32_t count;
}Control_Private_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables (except HAL handles)
 */
static Control_Private_t cntrl;

/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/

Status_t Control_Init(void)
{
  Status_t ret = STATUS_OK;

  return ret;
}


Status_t Control_Handle(void)
{
  Status_t ret = STATUS_OK;

  /* Handle Reset commands */
  if (conf.sys.command == CONTROL_RESET)
  {
    Led_GreenOff();
    Led_RedOff();
    System_Delay(1000);
    System_Reset();
  }
  /* Handle factory reset command */
  else if (conf.sys.command == CONTROL_FACTORY)
  {
    FlashApp_RequestFactorySettings();
  }
  else if (conf.sys.command == CONTROL_WDG_LATCH)
  {
    /* The delay below should cause IWDG to restart device */
    System_Delay(4000);
  }
  /* Handle set error command */
  else if (conf.sys.command > CONTROL_ERROR && conf.sys.command < (CONTROL_ERROR + 100))
  {
    cntrl.localError = conf.sys.command - CONTROL_ERROR;
  }
  /* Handle clear error command */
  else if (conf.sys.command == CONTROL_ERROR)
  {
    cntrl.localError = 0;
  }
  /* Testing mode disables setting of modbus address from DIP */
  if (conf.sys.command == CONTROL_TESTING)
  {
    SET_BIT(conf.sys.status, STAT_BIT_TESTING);
  }
  else
  {
    CLEAR_BIT(conf.sys.status, STAT_BIT_TESTING);
    conf.com.mb_address = (conf.sys.io_input & 0x1F) + MODBUS_BASE_ADDRESS;
  }

  /* Error flag of status register */
  if (cntrl.localError != 0 || (conf.sys.status & (0x300F0)) != 0)
  {
    SET_BIT(conf.sys.status, STAT_BIT_ERROR);
  }
  else
  {
    CLEAR_BIT(conf.sys.status, STAT_BIT_ERROR);
  }

  /* Led blinking according to status flag */
  if (cntrl.count % LED_PERIOD == 0)
  {
    if (conf.sys.status & STAT_BIT_ERROR)
    {
      Led_RedOn();
    }
    else if (conf.sys.status & STAT_BIT_MB_TIMEOUT)
    {
      Led_RedOn();
      Led_GreenOn();
    }
    else
    {
      Led_GreenOn();
      Led_RedOff();
    }
  }
  else if (cntrl.count % LED_PERIOD == LED_BLINK)
  {
    Led_GreenOff();
    Led_RedOff();
  }

  if (HAL_GetTick() > cntrl.prevTick)
  {
    cntrl.prevTick += 1000;
    conf.sys.uptime += 1;
  }

  /* Store previous command */
  cntrl.prevCmd = conf.sys.command;
  cntrl.count++;

  return ret;
}

/* Private Functions ---------------------------------------------------------*/

/** @} */
