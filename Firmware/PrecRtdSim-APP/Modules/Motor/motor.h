/**
 * @file       motor.h
 * @version    $(APP_VERSION)
 * @date       $(RELEASE_DATE)
 * @brief      This file contains header for motor module
 * @author     ondrej.lufinka
 *
 * @copyright  Logic Elements Copyright
 *
 * @defgroup grMotor Motor group
 * @{
 * @brief This group contains code for motor module
 *
 * This module contains
 *
 * @par Main features:
 * Motor module initialization
 * Motor module handle
 *  - measurement of the motor 1 and 2 currents
 *  - diagnostics of the currents to give output of steps and positions for both motors
 *    (value returned to the configuration registers)
 *
 * @endcode
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MOTOR_H_
#define MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Common includes */
#include "configuration.h"

/* Driver includes */
#include "adc.h"
#include "tim.h"

/* Application includes */

/* Definitions ---------------------------------------------------------------*/

/**
 * Input ADC defines
 */
#define hadc_motor              hadc3         ///< HAL ADC handle for motor current measurement
#define htim_motor              htim4         ///< HAL TIM handle for motor current measurement (timer triggers the ADC)
#define MOTOR_TIM_CH            TIM_CHANNEL_4 ///< TIM channel for trigger event

#define MOTOR_SHOW_STATS        (0)           ///< Show variables of the motor measurement stats

#define MOTOR_SWV_PRINT         (0)           ///< Enable SWV data printing for ADC measurement (1), otherwise (0)
                                              ///  (logging works for motor 1)

#define MOTOR_ADC_RES           (4096)        /// Resolution of ADC

#define MOTOR_ADC_SAMPLE_PER    (250)         ///< Sample period of ADC measurement [us]

#define MOTOR_CH_CNT            (4)           ///< Count of the measurement channels
#define MOTOR_BUF_SIZE          (100)         ///< Size of the buffer for each measured channel (there are 4 channels)
                                              ///  (DMA_int_period = MOTOR_BUF_SIZE * MOTOR_ADC_SAMPLE_PER = 25 ms)
#define MOTOR_BUF_CNT           (8)           ///< Count of the buffers (count must be >= (handle_proc_period / DMA_int_period) * 4)
                                              ///  (8 >= (50 ms / 25 ms) * 4)

#define MOTOR_FILT_SIZE         (4)           ///< ADC accumulation filter size

#define MOTOR_ZERO_FILT_SIZE    (16000)       ///< Accumulation filter for zero measurement size
#define MOTOR_PEAK_FILT_SIZE    (2000)        ///< Accumulation filter for peaks measurement size
                                              ///  (zero filtering period = MOTOR_ZERO_FILT_SIZE * MOTOR_ADC_SAMPLE_PER = 0.5 s)

#define MOTOR_IN_0_LIM_DEF      (471)         ///< Default threshold of ADC values to switch state from positive/negative to zero
                                              ///  (range of zero state is from zero_level - MOTOR_IN_0_LIMIT to zero_level + MOTOR_IN_0_LIMIT)
#define MOTOR_OUT_0_LIM_DEF     (565)         ///< Default threshold of ADC values to switch state from zero to positive/negative
                                              ///  (range of positive state is above zero_level + MOTOR_OUT_0_LIMIT,
                                              ///   range of negative state is below zero_level - MOTOR_OUT_0_LIMIT)
#define MOTOR_IN_0_LIM_PERC     (45)          ///< Percentage of ADC values to switch state from positive/negative to zero
                                              ///< (used when sine course range is evaluated)
#define MOTOR_OUT_0_LIM_PERC    (55)          ///< Percentage of ADC values to switch state from zero to positive/negative
                                              ///< (used when sine course range is evaluated)

#define MOTOR_CHANGE_SIZE       (4)           ///< Number of samples needed to be in range of new sine course state to change the state
                                              ///  (e.g. sine is moving from zero to positive, state is changed to positive after
                                              ///  (this number of samples is in positive state range)

#define MOTOR_CHANGE_MAX_PER    (50 * 1000)   ///< Time to wait for subsequent sine course state transition [us]
                                              ///  (if elapsed, last transition is cleared and steps counters
                                              ///   will not change with the next transition,
                                              ///   also MOTOR_CHANGE_MAX_PER must be <= MOTOR_BUF_CNT * MOTOR_BUF_SIZE * MOTOR_ADC_SAMPLE_PER / 4)

#define MOTOR_DIR_B_TO_A        ( 1)          ///< Direction of steps when channel B is preceding channel A
#define MOTOR_DIR_A_TO_B        (-1)          ///< Direction of steps when channel A is preceding channel B
                                              ///  (1 means the motor is moving from 0 to 100,
                                              ///   0 means the motor is moving from 100 to 0)

#define MOTOR_CNT_MIN_STEPS     (-64 - 832)   ///< Minimum value that can be set to motor steps counter

#define MOTOR_TOTAL_MIN_STEPS   (1)           ///< Minimum value that can be set to total motor steps count
#define MOTOR_TOTAL_DEF_STEPS_1 (314)         ///< Default value of total motor 1 steps count
#define MOTOR_TOTAL_DEF_STEPS_2 (392)         ///< Default value of total motor 2 steps count
#define MOTOR_TOTAL_MAX_STEPS   (8000)        ///< Maximum value that can be set to total motor steps count

#define MOTOR_MIN_POS           (-150)        ///< Value of motor position, when min steps reached [%]
#define MOTOR_MAX_POS           (150)         ///< Value of motor position, when max steps reached [%]

/* Macros --------------------------------------------------------------------*/

/* Typedefs ------------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

Status_t Motor_Init(void);

Status_t Motor_Handle(uint32_t period);

/* Callback prototypes -------------------------------------------------------*/

void Motor_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

void Motor_ADC_ErrorCallback(ADC_HandleTypeDef* hadc);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_H_ */

/** @} */
