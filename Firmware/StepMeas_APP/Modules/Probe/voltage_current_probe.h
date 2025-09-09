/**
 * @file       voltage_current_probe.h
 * @brief      Header for ADC measurement and motor probe handling
 * @addtogroup grGroup
 * @{
 */

#ifndef VOLTAGE_CURRENT_PROBE_H
#define VOLTAGE_CURRENT_PROBE_H

#include <stdint.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"

/* --- Public API --- */
void Probe_Init(void);
void StartAdcMeasurement(void);

/* --- Externí proměnné --- */
extern uint16_t adc_values[];
extern volatile uint8_t adc_ready;
extern volatile int16_t angle_deg;

/* --- Externí periferie --- */
extern TIM_HandleTypeDef htim2;

#endif /* VOLTAGE_CURRENT_PROBE_H */

/** @} */
