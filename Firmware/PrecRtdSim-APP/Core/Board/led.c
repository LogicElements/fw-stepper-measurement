/**
 * @file       led.c
 * @brief      file_brief
 * @addtogroup gr
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "led.h"

/* Private defines -----------------------------------------------------------*/

#define LED_RED_PORT    GPIOB
#define LED_RED_PIN     GPIO_PIN_3

#define LED_GREEN_PORT  GPIOB
#define LED_GREEN_PIN   GPIO_PIN_4

/* Private macros  -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/

Status_t Led_Init(void)
{
  Status_t ret = STATUS_OK;
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);

  HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = LED_RED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED_GREEN_PIN;
  HAL_GPIO_Init(LED_GREEN_PORT, &GPIO_InitStruct);

  return ret;
}

Status_t Led_GreenOn(void)
{
  Status_t ret = STATUS_OK;

  HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);

  return ret;
}

Status_t Led_GreenOff(void)
{
  Status_t ret = STATUS_OK;

  HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);

  return ret;
}

Status_t Led_GreenToggle(void)
{
  Status_t ret = STATUS_OK;

  HAL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);

  return ret;
}

Status_t Led_RedOn(void)
{
  Status_t ret = STATUS_OK;

  HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);

  return ret;
}

Status_t Led_RedOff(void)
{
  Status_t ret = STATUS_OK;

  HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);

  return ret;
}

/* Private Functions ---------------------------------------------------------*/

/** @} */
