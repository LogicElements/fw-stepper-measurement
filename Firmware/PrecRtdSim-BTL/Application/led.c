/**
 * @file       led.c
 * @brief      LED driver implementation
 * @addtogroup grLed
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "system_msp.h"
#include "led.h"

/* Private defines -----------------------------------------------------------*/
/* Private macros  -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/

/**
 * Definition of all private variables
 */
typedef struct
{
  uint32_t index;
}LedPwm_Private_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables (except HAL handles)
 */
static LedPwm_Private_t led;

/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static Status_t Led_HalInit(void);

/* Functions -----------------------------------------------------------------*/

Status_t Led_Init(void)
{
  Status_t ret = STATUS_OK;

  /* Initialize HAL (LL) layer */
  ret = Led_HalInit();

  /* Start timer */
  LL_TIM_EnableIT_UPDATE(TIM14);
  LL_TIM_EnableCounter(TIM14);

  return ret;
}


/**
* @brief This function handles TIM6 global interrupt and DAC1/DAC2 underrun error interrupts.
*/
void TIM14_IRQHandler(void)
{
  /* TIM Update event */
  if (LL_TIM_IsActiveFlag_UPDATE(TIM14))
  {
    LL_TIM_ClearFlag_UPDATE(TIM14);

    /* Set new combination of colors */
    if (led.index & 1)
    {
      LL_GPIO_ResetOutputPin(LED_R_PORT, LED_R_PIN);
      LL_GPIO_SetOutputPin(LED_G_PORT, LED_G_PIN);
    }
    else
    {
      LL_GPIO_SetOutputPin(LED_R_PORT, LED_R_PIN);
      LL_GPIO_ResetOutputPin(LED_G_PORT, LED_G_PIN);
    }

    /* Increment counter */
    led.index += 1;
  }

}

/* Private Functions ---------------------------------------------------------*/


static Status_t Led_HalInit(void)
{
  Status_t ret = STATUS_OK;
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);

  /* TIM14 interrupt Init */
  NVIC_SetPriority(TIM14_IRQn, PRIO_IRQ_LED);
  NVIC_EnableIRQ(TIM14_IRQn);

  TIM_InitStruct.Prescaler = 47999;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 249;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;

  LL_TIM_Init(TIM14, &TIM_InitStruct);

  LL_TIM_DisableARRPreload(TIM14);

  /* Set output pin modes */
  LL_GPIO_SetPinMode(LED_R_PORT, LED_R_PIN, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LED_G_PORT, LED_G_PIN, LL_GPIO_MODE_OUTPUT);

  return ret;
}


/** @} */
