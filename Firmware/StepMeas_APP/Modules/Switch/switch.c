/**
 * @file       switch.c
 * @brief      DIP switch driver
 * @addtogroup grSwitch
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "switch.h"
#include "main.h"

/* Private defines -----------------------------------------------------------*/

#define SW_DEBOUNCE_COUNT       15

/* Private macros  -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/

typedef struct
{
  uint16_t value;
  uint16_t valueToDebounce;
  uint16_t debounceCount;
  uint32_t tick;
  uint32_t btnPressed;
  uint32_t btnReleased;
}Switch_Private_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static Switch_Private_t sw;

/**
 * @brief Switch GPIO Port
 */
static GPIO_TypeDef* Switch_Port[SWITCH_NUMBER] =
{
    SWITCH_1_PORT, SWITCH_2_PORT, SWITCH_3_PORT, SWITCH_4_PORT, SWITCH_5_PORT,
    SWITCH_6_PORT, SWITCH_7_PORT, SWITCH_8_PORT, SWITCH_9_PORT, SWITCH_10_PORT
};

/**
 * @brief Switch GPIO Pin
 */
static const uint16_t Switch_Pin[SWITCH_NUMBER] =
{
    SWITCH_1_PIN, SWITCH_2_PIN, SWITCH_3_PIN, SWITCH_4_PIN, SWITCH_5_PIN,
    SWITCH_6_PIN, SWITCH_7_PIN, SWITCH_8_PIN, SWITCH_9_PIN, SWITCH_10_PIN
};


/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/


void Switch_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Configure all GPIO pins */
  for (int i = 0 ; i < SWITCH_NUMBER; i++)
  {
    GPIO_InitStruct.Pin = Switch_Pin[i];
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(Switch_Port[i], &GPIO_InitStruct);
  }

  return;
}


uint16_t Switch_GetAll(void)
{
  uint16_t val = 0;

  /* Read input states on all GPIO pins */
  for (int i = 0; i < SWITCH_NUMBER; i++)
  {
    if (HAL_GPIO_ReadPin(Switch_Port[i], Switch_Pin[i]) == 0)
    {
      val |= (1 << i);
    }
  }

  /* Did the value change */
  if (val != sw.value)
  {
    /* New value is read */
    if (val != sw.valueToDebounce)
    {
      /* New value is not debouncing, start it */
      sw.debounceCount = 0;
      sw.valueToDebounce = val;
    }

    /* Increment debounce counter */
    sw.debounceCount++;

    /* Debouncing ended, we have new value */
    if (sw.debounceCount > SW_DEBOUNCE_COUNT)
    {
      sw.value = val;
    }
  }
  else
  {
    sw.debounceCount = 0;
  }

  /* Is push button pressed? */
  if (sw.value & (1 << 9))
  {
    sw.btnPressed++;
  }
  else
  {
    sw.btnReleased = sw.btnPressed;
    sw.btnPressed = 0;
  }

  return sw.value;
}

uint16_t Switch_GetBootstrap(void)
{
  return (sw.value >> 5) & 0x0F;
}

uint16_t Switch_GetAddress(void)
{
  return sw.value & 0x1F;
}

uint32_t Switch_GetPressed(void)
{
  return sw.btnPressed;
}

uint32_t Switch_GetReleased(void)
{
  return sw.btnReleased;
}

/* Private Functions ---------------------------------------------------------*/

/** @} */
