/**
 * @file       voltage_current_probe.h
 * @brief      Header file for ADC measurement and motor probe handling
 * @addtogroup grGroup
 * @{
 */

#ifndef VOLTAGE_CURRENT_PROBE_H
#define VOLTAGE_CURRENT_PROBE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>          // uint8_t, uint16_t, uint32_t
#include "stm32f4xx_hal.h"   // HAL handlers

/* Exported defines ----------------------------------------------------------*/
#define ADC_CHANNEL_COUNT   4

/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;



extern uint16_t adc_values[ADC_CHANNEL_COUNT];
extern volatile uint8_t adc_ready;
extern uint16_t adc_ready_counter;

extern uint16_t VA;
extern uint16_t VB;
extern uint32_t IA;
extern uint16_t IB;
extern uint16_t currentStatus;
extern uint16_t adc_value;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize probe module
 * @retval STATUS_OK or STATUS_ERROR
 */
void  Probe_Init(void);

/**
 * @brief Main probe handler
 * @retval STATUS_OK or STATUS_ERROR
 */
void  Probe_Handle(void);

/**
 * @brief Start ADC measurement using DMA
 */
void StartAdcMeasurement(void);

/**
 * @brief Process motor steps based on ADC values
 */
void ProcessMotorSteps(uint16_t adc_sample[]);

/**
 * @brief Perform ADC value conversion and store into structures
 */
void ADC_Conversion(uint16_t _adc_values[]);

/* Callback functions --------------------------------------------------------*/

/**
 * @brief Motor ADC conversion complete callback (called from HAL_ADC_ConvCpltCallback)
 */

#ifdef __cplusplus
}
#endif

#endif /* VOLTAGE_CURRENT_PROBE_H */

/** @} */
