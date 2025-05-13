/**
 * @file       motor.c
 * @brief      This file contains code for motor module
 * @addtogroup grMotor
 * @{
 */

/* TODO 1: Check if these edges are necessary to catch (alternative 1 to standard edges) */
/* TODO 2: Check if these edges are necessary to catch (alternative 2 to standard edges) */

/* TODO: Lze ladit pomoci zmeny hystereze - MOTOR_IN_0_LIM_DEF a MOTOR_OUT_0_LIM_DEF
 *                                        - MOTOR_IN_0_LIM_PERC a MOTOR_OUT_0_LIM_PERC
 *
 *                        zmeny delky filtru - MOTOR_FILT_SIZE
 *
 *                        zmeny poctu vzorku pro detekci hrany - MOTOR_CHANGE_SIZE
 *
 *                        detekce alternativnich hran (TODO_1 a TODO_2)
 *
 *       Problem je ze DHW nekonci generovani kroku v cele periode a pak je nahodne jestli se
 *       na zacatku/konci detekuje hrana nebo ne (podle toho jestli kanal prejel hysterezi nebo
 *       ne, ovsem hystereze se neda odstranit protoze pote do detekuje sum).
 */

/* Includes ------------------------------------------------------------------*/

#include "Motor/motor.h"

/* Common includes */
#include "control.h"
#include "system_msp.h"
#include "Motor/oscillogram.h"

/* Driver includes */

/* Application includes */

/* Private definitions -------------------------------------------------------*/

#if (MOTOR_SHOW_STATS == 1)
#define MOTOR_EDGES_BUFFER 800
#endif

/* Private macros  -----------------------------------------------------------*/

/* Private typedefs ----------------------------------------------------------*/

/**
 * Motor channels enum
 */
typedef enum
{
  MOTOR_CHANNEL_A,                            ///< Motor channel A
  MOTOR_CHANNEL_B,                            ///< Motor channel B

}Motor_Channel_e;

/**
 * Motor channel states enum
 */
typedef enum
{
  MOTOR_STATE_IN_ZERO_FROM_START,             ///< Channel value is in zero level from start (from before the current movement started)
  MOTOR_STATE_IN_ZERO_FROM_POSITIVE,          ///< Channel value is in zero level (returned from positive level before)
  MOTOR_STATE_ZERO_TO_POSITIVE,               ///< Channel value goes to positive level from zero level
  MOTOR_STATE_IN_POSITIVE,                    ///< Channel value is in positive level
  MOTOR_STATE_POSITIVE_TO_ZERO,               ///< Channel value goes to zero level from positive level
  MOTOR_STATE_IN_ZERO_FROM_NEGATIVE,          ///< Channel value is in zero level (returned from negative level before)
  MOTOR_STATE_ZERO_TO_NEGATIVE,               ///< Channel value goes to negative level from zero level
  MOTOR_STATE_IN_NEGATIVE,                    ///< Channel value is in negative level
  MOTOR_STATE_NEGATIVE_TO_ZERO,               ///< Channel value goes to zero level from negative level

}Motor_Channel_State_e;

/**
 * Motor ADC values buffer
 */
typedef struct
{
  uint16_t _1_A;                              ///< ADC value for motor 1 channel A
  uint16_t _1_B;                              ///< ADC Value for motor 1 channel B
  uint16_t _2_A;                              ///< ADC Value for motor 2 channel A
  uint16_t _2_B;                              ///< ADC Value for motor 2 channel B

}Motor_Buf_t;

/**
 * Motor accumulation filter
 */
typedef struct
{
  uint32_t value;                             ///< Accumulation filter value
  uint16_t it;                                ///< Accumulation filter iterator

}Motor_Acc_Filt_t;

/**
 * Motor channel structure
 */
typedef struct
{
  Motor_Acc_Filt_t ADC_filter;                ///< Accumulation filter for ADC values

  Motor_Channel_State_e state;                ///< Current motor channel state
  Motor_Channel_State_e last_edge;            ///< Last edge state
  Motor_Channel_State_e last_edge_to_check;   ///< Last edge state that needs to be processed

  Motor_Acc_Filt_t zero_acc_filter;           ///< Zero value accumulation filter
  uint16_t zero_value;                        ///< Zero filtered value

  uint16_t local_peak_value;                  ///< Value of the current sine course local peak
  Motor_Acc_Filt_t positive_acc_filter;       ///< Positive peak value accumulation filter
  Motor_Acc_Filt_t negative_acc_filter;       ///< Negative peak value accumulation filter
  uint16_t positive_value;                    ///< Positive peak filtered value
  uint16_t negative_value;                    ///< Negative peak filtered value
  int32_t in_0_lim;                           ///< Threshold for detection positive/negative to zero edges
  int32_t out_0_lim;                          ///< Threshold for detection zero to positive/negative edges

  uint8_t to_zero_cnt;                        ///< Counter for positive/negative to zero edges detection
  uint8_t to_positive_cnt;                    ///< Counter for zero to positive edges detection
  uint8_t to_negative_cnt;                    ///< Counter for zero to negative edges detection

  int32_t last_edge_time;                     ///< Time of last edge
  int32_t last_zero_to_positive_time;         ///< Time of the last edge from zero to positive peak
  int32_t last_zero_to_negative_time;         ///< Time of the last edge from zero to negative peak
  int32_t last_period;                        ///< Period of last detected sine course
  int32_t time_from_last_edge;                ///< Time from last edge to now

}Motor_Channel_t;

#if (MOTOR_SHOW_STATS == 1)
/**
 * Motor detected edges buffer
 */
typedef struct
{
  Motor_Channel_e ch;                         ///< Channel the detected edges comes from
  Motor_Channel_State_e detected_edge;        ///< Detected edge type from current motor channel
  Motor_Channel_State_e compared_edge;        ///< Edge type that is used from opposite motor channel to compare the detected edge
  _Bool correct;                              ///< If edge was counted as correct or not

}Motor_Edges_Buf_t;

/**
 * Motor stats structure
 */
typedef struct
{
  uint32_t canceled_edges_A;                  ///< Cancelled edges
  uint32_t canceled_edges_B;

  uint8_t edges_buffer_it;                    ///< Buffer of detected edges
  Motor_Edges_Buf_t edges_buffer[MOTOR_EDGES_BUFFER];

  uint32_t edges_A_up;                        ///< Total edges detected for motor channel A (for each type of edge)
  uint32_t edges_A_up_to_zero;
  uint32_t edges_A_down;
  uint32_t edges_A_down_to_zero;

  uint32_t edges_B_up;                        ///< Total edges detected for motor channel B (for each type of edge)
  uint32_t edges_B_up_to_zero;
  uint32_t edges_B_down;
  uint32_t edges_B_down_to_zero;

}Motor_Stats_t;
#endif

/**
 * Declaration of all private variables
 */
typedef struct
{
  _Bool meas_proc;                            ///< Flag that the previous measurement has been processed
  _Bool meas_started;                         ///< Flag that the measurement has started
  uint32_t ADC_int_last_time;                 ///< Last tick time of the ADC interrupt
  uint16_t sample_period;                     ///< Sampling period of the ADC [us]
  uint32_t total_proc_time;
  Motor_Buf_t adc_buf[MOTOR_BUF_CNT]
                     [MOTOR_BUF_SIZE];        ///< Buffer for ADC measurement for each motor channel
  uint8_t buf_it;                             ///< Buffer iterator
  uint8_t buf_it_last;                        ///< Buffer last iterator
  uint8_t proc_range;                         ///< Range of buffers to process

  Motor_Channel_t _1_A;                       ///< Structure for motor 1 channel A
  Motor_Channel_t _1_B;                       ///< Structure for motor 1 channel B
  Motor_Channel_t _2_A;                       ///< Structure for motor 2 channel A
  Motor_Channel_t _2_B;                       ///< Structure for motor 2 channel B

  int8_t steps_1_last_dir;                    ///< Last detected direction of motor 1
  int8_t steps_2_last_dir;                    ///< Last detected direction of motor 2

  int16_t steps_1_cnt;                        ///< Counter of steps of motor 1
  int16_t steps_2_cnt;                        ///< Counter of steps of motor 2

  uint16_t steps_1_total;                     ///< Total set steps of motor 1
  uint16_t steps_2_total;                     ///< Total set steps of motor 2

  float position_1;                           ///< Position of motor 1 (form 0 to MOTOR_POS_RESOLUTION)
  float position_2;                           ///< Position of motor 2 (form 0 to MOTOR_POS_RESOLUTION)

  #if (MOTOR_SHOW_STATS == 1)
  Motor_Stats_t stats_1;                      ///< Variables for motor measurement statistics */
  Motor_Stats_t stats_2;
  #endif

  #if (MOTOR_SWV_PRINT == 1)
  int32_t print_it;                           ///< Variables for ADC data printing to SWV */
  int32_t print_trigger;
  #endif

}Motor_Private_t;

/* Constants -----------------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables (except HAL handles)
 */
static Motor_Private_t motor;

/* Private callback prototypes -----------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

#if (MOTOR_SHOW_STATS == 1)
static Status_t Motor_ChannelStateMachine(Motor_Channel_t *ch,
                                          int8_t dir, int8_t *last_dir, int16_t *steps,
                                          int16_t min_steps, int16_t max_steps,
                                          int32_t timestamp, uint16_t new_val,
                                          Motor_Stats_t *stats);

static Status_t Motor_StepsStateMachine(Motor_Channel_t *ch_A, Motor_Channel_t *ch_B,
                                        int8_t *last_dir, int16_t *steps,
                                        int16_t min_steps, int16_t max_steps,
                                        Motor_Stats_t *stats);
#else
static Status_t Motor_ChannelStateMachine(Motor_Channel_t *ch,
                                          int8_t dir, int8_t *last_dir, int16_t *steps,
                                          int16_t min_steps, int16_t max_steps,
                                          int32_t timestamp, uint16_t new_val);

static Status_t Motor_StepsStateMachine(Motor_Channel_t *ch_A, Motor_Channel_t *ch_B,
                                        int8_t *last_dir, int16_t *steps,
                                        int16_t min_steps, int16_t max_steps);
#endif

static uint16_t Motor_AccFilter(Motor_Acc_Filt_t *acc_filter, uint16_t add_value, uint16_t size);

static Status_t Motor_StateCounter(uint8_t *cnt_to_inc, uint8_t *cnt_to_dec_1, uint8_t *cnt_to_dec_2);

#if (MOTOR_SHOW_STATS == 1)
static Status_t Motor_ClearEdgeTimes(Motor_Channel_t *ch,
                                     int8_t dir, int8_t *last_dir, int16_t *steps,
                                     int16_t min_steps, int16_t max_steps,
                                     Motor_Stats_t *motor_stats);
#else
static Status_t Motor_ClearEdgeTimes(Motor_Channel_t *ch,
                                     int8_t dir, int8_t *last_dir, int16_t *steps,
                                     int16_t min_steps, int16_t max_steps);
#endif

#if (MOTOR_SWV_PRINT == 1)
static Status_t Motor_CheckTrigger(uint8_t i, uint8_t j);
static Status_t Motor_PrintData();
#endif

static Status_t Motor_InitHAL(void);

/* Functions -----------------------------------------------------------------*/

/**
 * @brief  This function initializes motor module.
 *
 * @param  None
 *
 * @retval Status_t
 */
Status_t Motor_Init(void)
{
  Status_t ret = STATUS_OK;

  /* Clear private variables */
  memset(&motor, 0, sizeof(motor));

  /* Set default motor variables */
  motor.meas_proc = true;
  motor.buf_it = MOTOR_BUF_CNT;
  motor.steps_1_total = MOTOR_TOTAL_DEF_STEPS_1;
  motor.steps_2_total = MOTOR_TOTAL_DEF_STEPS_2;
  #if (MOTOR_SWV_PRINT == 1)
  motor.print_trigger = -1;
  #endif

  /* HAL related initialization */
  ret |= Motor_InitHAL();

  return ret;
}

/**
 * @brief  This function handles motor module.
 *
 * @note   Must be called periodically.
 *
 * @param  period: Period of the handle call
 *
 * @retval Status_t
 */
Status_t Motor_Handle(uint32_t period)
{
  Status_t ret = STATUS_OK;
  uint8_t buf_it = 0;

  /* Get tick */
  int64_t total_time_tick = System_GetTick();

  /* Check if total steps parameters changed and set them if needed */
  if (motor.steps_1_total != conf.motor.steps_1 ||
      motor.steps_2_total != conf.motor.steps_2)
  {
    if (conf.motor.steps_1 >= MOTOR_TOTAL_MIN_STEPS && conf.motor.steps_1 <= MOTOR_TOTAL_MAX_STEPS)
    {
      motor.steps_1_total = conf.motor.steps_1;
    }
    else
    {
      conf.motor.steps_1 = motor.steps_1_total;
      ret |= STATUS_ERROR;
    }
    if (conf.motor.steps_2 >= MOTOR_TOTAL_MIN_STEPS && conf.motor.steps_2 <= MOTOR_TOTAL_MAX_STEPS)
    {
      motor.steps_2_total = conf.motor.steps_2;
    }
    else
    {
      conf.motor.steps_2 = motor.steps_2_total;
      ret |= STATUS_ERROR;
    }
  }

  /* Check if position changed from outside and set it if needed
   * NOTE: Stepper motor driver in DHW-21x as well as measurement in the test board use steps count
   *       two times bigger than the application itself, therefore *2 is needed for total steps range
   *       which is defined according to the application firmware */
  if (motor.position_1 != conf.motor.position_1 ||
      motor.position_2 != conf.motor.position_2)
  {
    if (conf.motor.position_1 >= MOTOR_MIN_POS && conf.motor.position_1 <= MOTOR_MAX_POS)
    {
      motor.position_1 = conf.motor.position_1;
      motor.steps_1_cnt = (motor.position_1 * (motor.steps_1_total * 2)) / 100;
    }
    else
    {
      conf.motor.position_1 = motor.position_1;
      ret |= STATUS_ERROR;
    }
    if (conf.motor.position_2 >= MOTOR_MIN_POS && conf.motor.position_2 <= MOTOR_MAX_POS)
    {
      motor.position_2 = conf.motor.position_2;
      motor.steps_2_cnt = (motor.position_2 * (motor.steps_2_total * 2)) / 100;
    }
    else
    {
      conf.motor.position_2 = motor.position_2;
      ret |= STATUS_ERROR;
    }
  }

  /* Compute new motors currents if measurement is completed */
  if (motor.meas_proc == false)
  {
    /* Compute number of buffers to process */
    buf_it = motor.buf_it;
    motor.proc_range = (((int16_t)buf_it - (int16_t)motor.buf_it_last) + MOTOR_BUF_CNT) % MOTOR_BUF_CNT;
    if (motor.proc_range <= 0)
    {
      /* If buffer iterators are equal -> processing is too slow -> error */
      ret |= STATUS_ERROR;
    }

    /* Process values - from (last_buf_it) to (buf_it - 1) */
    for (uint8_t i = motor.buf_it_last; i < motor.buf_it_last + motor.proc_range; i++)
    {

      /* Pass it to oscillogram */
      Oscil_NewData((uint16_t *)motor.adc_buf[i % MOTOR_BUF_CNT], MOTOR_BUF_SIZE);

      /* Process all values in the buffer */
      for (uint8_t j = 0; j < MOTOR_BUF_SIZE; j++)
      {
        /* For each ADC measurement, process the state machine of each motor channel */
        #if (MOTOR_SHOW_STATS == 1)
        ret |= Motor_ChannelStateMachine(&motor._1_A,
                                         MOTOR_DIR_A_TO_B, &motor.steps_1_last_dir, &motor.steps_1_cnt,
                                         MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                         ((i % MOTOR_BUF_CNT) * MOTOR_BUF_SIZE + j) * MOTOR_ADC_SAMPLE_PER,
                                         Motor_AccFilter(&motor._1_A.ADC_filter,
                                                         motor.adc_buf[i % MOTOR_BUF_CNT][j]._1_A,
                                                         MOTOR_FILT_SIZE),
                                         &motor.stats_1);

        ret |= Motor_ChannelStateMachine(&motor._1_B,
                                         MOTOR_DIR_B_TO_A, &motor.steps_1_last_dir, &motor.steps_1_cnt,
                                         MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                         ((i % MOTOR_BUF_CNT) * MOTOR_BUF_SIZE + j) * MOTOR_ADC_SAMPLE_PER,
                                         Motor_AccFilter(&motor._1_B.ADC_filter,
                                                         motor.adc_buf[i % MOTOR_BUF_CNT][j]._1_B,
                                                         MOTOR_FILT_SIZE),
                                         &motor.stats_1);

        ret |= Motor_ChannelStateMachine(&motor._2_A,
                                         MOTOR_DIR_A_TO_B, &motor.steps_1_last_dir, &motor.steps_1_cnt,
                                         MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                         ((i % MOTOR_BUF_CNT) * MOTOR_BUF_SIZE + j) * MOTOR_ADC_SAMPLE_PER,
                                         Motor_AccFilter(&motor._2_A.ADC_filter,
                                                         motor.adc_buf[i % MOTOR_BUF_CNT][j]._2_A,
                                                         MOTOR_FILT_SIZE),
                                         &motor.stats_2);

        ret |= Motor_ChannelStateMachine(&motor._2_B,
                                         MOTOR_DIR_B_TO_A, &motor.steps_1_last_dir, &motor.steps_1_cnt,
                                         MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                         ((i % MOTOR_BUF_CNT) * MOTOR_BUF_SIZE + j) * MOTOR_ADC_SAMPLE_PER,
                                         Motor_AccFilter(&motor._2_B.ADC_filter,
                                                         motor.adc_buf[i % MOTOR_BUF_CNT][j]._2_B,
                                                         MOTOR_FILT_SIZE),
                                         &motor.stats_2);
        #else
        ret |= Motor_ChannelStateMachine(&motor._1_A,
                                         MOTOR_DIR_A_TO_B, &motor.steps_1_last_dir, &motor.steps_1_cnt,
                                         MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                         ((i % MOTOR_BUF_CNT) * MOTOR_BUF_SIZE + j) * MOTOR_ADC_SAMPLE_PER,
                                         Motor_AccFilter(&motor._1_A.ADC_filter,
                                                         motor.adc_buf[i % MOTOR_BUF_CNT][j]._1_A,
                                                         MOTOR_FILT_SIZE));

        ret |= Motor_ChannelStateMachine(&motor._1_B,
                                         MOTOR_DIR_B_TO_A, &motor.steps_1_last_dir, &motor.steps_1_cnt,
                                         MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                         ((i % MOTOR_BUF_CNT) * MOTOR_BUF_SIZE + j) * MOTOR_ADC_SAMPLE_PER,
                                         Motor_AccFilter(&motor._1_B.ADC_filter,
                                                         motor.adc_buf[i % MOTOR_BUF_CNT][j]._1_B,
                                                         MOTOR_FILT_SIZE));

        ret |= Motor_ChannelStateMachine(&motor._2_A,
                                         MOTOR_DIR_A_TO_B, &motor.steps_2_last_dir, &motor.steps_2_cnt,
                                         MOTOR_CNT_MIN_STEPS, (motor.steps_2_total * 2) - MOTOR_CNT_MIN_STEPS,
                                         ((i % MOTOR_BUF_CNT) * MOTOR_BUF_SIZE + j) * MOTOR_ADC_SAMPLE_PER,
                                         Motor_AccFilter(&motor._2_A.ADC_filter,
                                                         motor.adc_buf[i % MOTOR_BUF_CNT][j]._2_A,
                                                         MOTOR_FILT_SIZE));

        ret |= Motor_ChannelStateMachine(&motor._2_B,
                                         MOTOR_DIR_B_TO_A, &motor.steps_2_last_dir, &motor.steps_2_cnt,
                                         MOTOR_CNT_MIN_STEPS, (motor.steps_2_total * 2) - MOTOR_CNT_MIN_STEPS,
                                         ((i % MOTOR_BUF_CNT) * MOTOR_BUF_SIZE + j) * MOTOR_ADC_SAMPLE_PER,
                                         Motor_AccFilter(&motor._2_B.ADC_filter,
                                                         motor.adc_buf[i % MOTOR_BUF_CNT][j]._2_B,
                                                         MOTOR_FILT_SIZE));
        #endif

        /* For each motor, process the steps (position) state machine
         * NOTE: Stepper motor driver in DHW-21x as well as measurement in the test board use steps count
         *       two times bigger than the application itself, therefore *2 is needed for total steps range
         *       which is defined according to the application firmware */
        #if (MOTOR_SHOW_STATS == 1)
        ret |= Motor_StepsStateMachine(&motor._1_A, &motor._1_B,
                                       &motor.steps_1_cnt,
                                       MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                       &motor.stats_1);

        ret |= Motor_StepsStateMachine(&motor._2_A, &motor._2_B,
                                       &motor.steps_2_cnt,
                                       MOTOR_CNT_MIN_STEPS, (motor.steps_2_total * 2) - MOTOR_CNT_MIN_STEPS,
                                       &motor.stats_2);
        #else
        ret |= Motor_StepsStateMachine(&motor._1_A, &motor._1_B,
                                       &motor.steps_1_last_dir, &motor.steps_1_cnt,
                                       MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS);

        ret |= Motor_StepsStateMachine(&motor._2_A, &motor._2_B,
                                       &motor.steps_2_last_dir, &motor.steps_2_cnt,
                                       MOTOR_CNT_MIN_STEPS, (motor.steps_2_total * 2) - MOTOR_CNT_MIN_STEPS);
        #endif

        /* Check for trigger for SWV data printing if enabled */
        #if (MOTOR_SWV_PRINT == 1)
        if (Motor_CheckTrigger(i, j) != STATUS_OK)
        {
          i = motor.buf_it_last + motor.proc_range;
          break;
        }
        #endif
      }
    }

    /* Print ADC data to SWV if enabled */
    #if (MOTOR_SWV_PRINT == 1)
    ret |= Motor_PrintData();
    #endif

    /* Save last processed buffer */
    motor.buf_it_last = buf_it;
    motor.meas_proc = true;

    /* Save measurement to register map
     * NOTE: Stepper motor driver in DHW-21x as well as measurement in the test board use steps count
     *       two times bigger than the application itself, therefore *2 is needed for total steps range
     *       which is defined according to the application firmware */
    motor.position_1 = ((float)(motor.steps_1_cnt * 100)) / (motor.steps_1_total * 2);
    motor.position_2 = ((float)(motor.steps_2_cnt * 100)) / (motor.steps_2_total * 2);
    SAT_DOWN(motor.position_1, MOTOR_MIN_POS);
    SAT_DOWN(motor.position_2, MOTOR_MIN_POS);
    SAT_UP(motor.position_1, MOTOR_MAX_POS);
    SAT_UP(motor.position_2, MOTOR_MAX_POS);
    conf.motor.position_1 = motor.position_1;
    conf.motor.position_2 = motor.position_2;

    /* Check the correctness of the ADC sample period -> if different than set -> error */
    if (motor.sample_period > MOTOR_ADC_SAMPLE_PER + 1 ||
        motor.sample_period < MOTOR_ADC_SAMPLE_PER - 1)
    {
      ret |= STATUS_ERROR;
    }
  }

  /* If ADC did not give any data -> error */
  else if (motor.meas_started == true)
  {
    motor.position_1 = MOTOR_MIN_POS;
    motor.position_1 = MOTOR_MIN_POS;
    conf.motor.position_1 = MOTOR_MIN_POS;
    conf.motor.position_1 = MOTOR_MIN_POS;
    ret |= STATUS_ERROR;
  }

  /* Compute total handle processing time */
  if (((int64_t)System_GetTick()) - total_time_tick > motor.total_proc_time)
  {
    motor.total_proc_time = ((int64_t)System_GetTick()) - total_time_tick;
  }
  if (motor.total_proc_time > period / 2)
  {
    ret |= STATUS_ERROR;
  }

  CHECK_ERROR(ret, ERROR_CODE_motor);
  return ret;
}

/* Callback functions --------------------------------------------------------*/

/**
 * @brief  This function is a callback for the ADC conversion complete interrupt.
 *
 * @param  hadc: ADC handle
 *
 * @retval None
 */
void Motor_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  Status_t ret = STATUS_OK;

  /* If correct ADC */
  if (hadc->Instance == hadc_motor.Instance)
  {
    /* Compute ADC sampling period */
    motor.sample_period = ((System_GetTick() - motor.ADC_int_last_time) * 1000) / MOTOR_BUF_SIZE;
    motor.ADC_int_last_time = System_GetTick();

    /* If previous measurement has finished -> flag to process the data */
    motor.meas_proc = false;
    motor.meas_started = true;

    /* Restart DMA */
    motor.buf_it++;
    if (motor.buf_it >= MOTOR_BUF_CNT)
    {
      motor.buf_it = 0;
    }
    if (HAL_ADC_Start_DMA(&hadc_motor, (uint32_t *)motor.adc_buf[motor.buf_it], MOTOR_CH_CNT * MOTOR_BUF_SIZE) != HAL_OK)
    {
      ret |= STATUS_ERROR;
    }
  }

  CHECK_ERROR(ret, ERROR_CODE_motor);
}

/**
 * @brief  This function is a callback for the ADC error interrupt.
 *
 * @param  hadc: ADC handle
 *
 * @retval None
 */
void Motor_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
  Status_t ret = STATUS_OK;

  /* If correct ADC */
  if (hadc->Instance == hadc_motor.Instance)
  {
    /* Set error */
    ret |= STATUS_ERROR;
  }

  CHECK_ERROR(ret, ERROR_CODE_motor);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  This function processes the state machine for the motor channel
 *
 * @param  ch:        Pointer to motor channel to process (ex.: Motor 1 or 2 channel A or B)
 *
 * @param  dir:       Which motor channel is selected in the current function call
 *                    (MOTOR_DIR_A_TO_B or MOTOR_DIR_B_TO_A)
 * @param  last_dir:  Pointer to the last detected step direction of motor connected to selected motor channel
 * @param  steps:     Pointer to the steps value of motor connected to selected motor channel
 *
 * @param  min_steps: Minimum possible steps value
 * @param  max_steps: Maximum possible steps value
 *
 * @param  timestamp: Time of the new value in ADC buffer
 * @param  new_val:   New ADC measured value for motor channel
 *
 * @param  stats:     Pointer to statistics structure
 *
 * @retval Status_t
 */
#if (MOTOR_SHOW_STATS == 1)
static Status_t Motor_ChannelStateMachine(Motor_Channel_t *ch,
                                          int8_t dir, int8_t *last_dir, int16_t *steps,
                                          int16_t min_steps, int16_t max_steps,
                                          int32_t timestamp, uint16_t new_val,
                                          Motor_Stats_t *stats)
#else
static Status_t Motor_ChannelStateMachine(Motor_Channel_t *ch,
                                          int8_t dir, int8_t *last_dir, int16_t *steps,
                                          int16_t min_steps, int16_t max_steps,
                                          int32_t timestamp, uint16_t new_val)
#endif
{
  Status_t ret = STATUS_OK;

  /* If positive and negative peaks were evaluated -> use these values as threshold, else -> default is used */
  if (ch->positive_acc_filter.it >= MOTOR_PEAK_FILT_SIZE &&
      ch->negative_acc_filter.it >= MOTOR_PEAK_FILT_SIZE)
  {
    ch->in_0_lim = (((ch->positive_value - ch->negative_value) / 2) * MOTOR_IN_0_LIM_PERC) / 100;
    ch->out_0_lim = (((ch->positive_value - ch->negative_value) / 2) * MOTOR_OUT_0_LIM_PERC) / 100;
  }
  else
  {
    ch->in_0_lim = MOTOR_IN_0_LIM_DEF;
    ch->out_0_lim = MOTOR_OUT_0_LIM_DEF;
  }

  /* Check whether the new value is moving to one of the state ranges -> set proper state change counter */
  if (new_val > ch->zero_value + ch->out_0_lim)
  {
    ret |= Motor_StateCounter(&ch->to_positive_cnt, &ch->to_zero_cnt, &ch->to_negative_cnt);
  }
  else if (new_val < ch->zero_value - ch->out_0_lim)
  {
    ret |= Motor_StateCounter(&ch->to_negative_cnt, &ch->to_zero_cnt, &ch->to_positive_cnt);
  }
  else if (new_val > ch->zero_value - ch->in_0_lim &&
           new_val < ch->zero_value + ch->in_0_lim)
  {
    ret |= Motor_StateCounter(&ch->to_zero_cnt, &ch->to_positive_cnt, &ch->to_negative_cnt);
  }

  /* Check if last transition is not too long ago */
  if (ch->last_edge != MOTOR_STATE_IN_ZERO_FROM_START)
  {
    /* If last transition occurred -> compute time from last transition */
    ch->time_from_last_edge = timestamp - ch->last_edge_time;
    if (ch->time_from_last_edge < 0)
    {
      ch->time_from_last_edge += (MOTOR_BUF_CNT * MOTOR_BUF_SIZE * MOTOR_ADC_SAMPLE_PER);
    }

    /* If maximum period threshold reached -> clear last transition state to prevent catching the false steps */
    if (ch->time_from_last_edge > MOTOR_CHANGE_MAX_PER)
    {
      #if (MOTOR_SHOW_STATS)
      ret |= Motor_ClearEdgeTimes(ch,
                                  dir, last_dir, steps,
                                  MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                  stats);
      #else
      ret |= Motor_ClearEdgeTimes(ch,
                                  dir, last_dir, steps,
                                  MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS);
      #endif
    }

    /* Else -> if we have valid period (last two from zero transitions occurred) -> use this period as threshold */
    else if (ch->last_period >= 0)
    {
      /* If threshold reached -> clear last transition state to prevent catching the false steps */
      if (ch->time_from_last_edge > ch->last_period)
      {
        #if (MOTOR_SHOW_STATS)
        ret |= Motor_ClearEdgeTimes(ch,
                                    dir, last_dir, steps,
                                    MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                    stats);
        #else
        ret |= Motor_ClearEdgeTimes(ch,
                                    dir, last_dir, steps,
                                    MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS);
        #endif
      }
    }
  }
  else
  {
    #if (MOTOR_SHOW_STATS)
    ret |= Motor_ClearEdgeTimes(ch,
                                0, last_dir, steps,
                                MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS,
                                stats);
    #else
    ret |= Motor_ClearEdgeTimes(ch,
                                0, last_dir, steps,
                                MOTOR_CNT_MIN_STEPS, (motor.steps_1_total * 2) - MOTOR_CNT_MIN_STEPS);
    #endif
  }

  /* Decide action according to the motor channel transition state */
  switch (ch->state)
  {
    /* Zero to positive sine part --------------------------------------------- */
    /* ADC value moved from zero to positive sine part */
    case MOTOR_STATE_ZERO_TO_POSITIVE:

      /* Compute transition period */
      if (ch->last_zero_to_negative_time >= 0)
      {
        ch->last_period = timestamp - ch->last_zero_to_negative_time;
        if (ch->last_period < 0)
        {
          ch->last_period += (MOTOR_BUF_CNT * MOTOR_BUF_SIZE * MOTOR_ADC_SAMPLE_PER);
        }
        ch->last_period = ch->last_period * 2;
      }

      /* Set new last transition timestamp and state */
      ch->last_edge_time = timestamp;
      ch->last_zero_to_positive_time = timestamp;
      ch->last_edge = MOTOR_STATE_ZERO_TO_POSITIVE;
      ch->last_edge_to_check = MOTOR_STATE_ZERO_TO_POSITIVE;
      ch->state = MOTOR_STATE_IN_POSITIVE;

    /* No break; ADC value is in positive sine part */
    case MOTOR_STATE_IN_POSITIVE:

      /* Filter positive peak level */
      ch->local_peak_value = MAX(ch->local_peak_value, new_val);
      if (new_val < ch->local_peak_value)
      {
        ch->positive_value = Motor_AccFilter(&ch->positive_acc_filter, ch->local_peak_value, MOTOR_PEAK_FILT_SIZE);
      }

      /* Check change of state and set the new one if needed */
      if (ch->to_zero_cnt >= MOTOR_CHANGE_SIZE)
      {
        ch->state = MOTOR_STATE_POSITIVE_TO_ZERO;
      }
      break;

    /* Positive to zero sine part --------------------------------------------- */
    /* ADC value moved from positive sine part to zero */
    case MOTOR_STATE_POSITIVE_TO_ZERO:

      /* Set new last transition timestamp and state */
      ch->last_edge_time = timestamp;
      ch->last_edge = MOTOR_STATE_POSITIVE_TO_ZERO;
      ch->last_edge_to_check = MOTOR_STATE_POSITIVE_TO_ZERO;
      ch->state = MOTOR_STATE_IN_ZERO_FROM_POSITIVE;

    /* No break; ADC value is in zero sine part (got here from positive part or after start) */
    case MOTOR_STATE_IN_ZERO_FROM_START:
    case MOTOR_STATE_IN_ZERO_FROM_POSITIVE:

      /* Filter zero level */
      ch->zero_value = Motor_AccFilter(&ch->zero_acc_filter, new_val, MOTOR_ZERO_FILT_SIZE);
      ch->local_peak_value = ch->zero_value;

      /* Check change of state and set the new one if needed */
      if (ch->to_positive_cnt >= MOTOR_CHANGE_SIZE)
      {
        ch->state = MOTOR_STATE_ZERO_TO_POSITIVE;
      }
      else if (ch->to_negative_cnt >= MOTOR_CHANGE_SIZE)
      {
        ch->state = MOTOR_STATE_ZERO_TO_NEGATIVE;
      }
      break;

    /* Zero to negative sine part --------------------------------------------- */
    /* ADC value moved from zero to negative sine part */
    case MOTOR_STATE_ZERO_TO_NEGATIVE:

      /* Compute transition period */
      if (ch->last_zero_to_positive_time >= 0)
      {
        ch->last_period = timestamp - ch->last_zero_to_positive_time;
        if (ch->last_period < 0)
        {
          ch->last_period += (MOTOR_BUF_CNT * MOTOR_BUF_SIZE * MOTOR_ADC_SAMPLE_PER);
        }
        ch->last_period = ch->last_period * 2;
      }

      /* Set new last transition timestamp and state */
      ch->last_edge_time = timestamp;
      ch->last_zero_to_negative_time = timestamp;
      ch->last_edge = MOTOR_STATE_ZERO_TO_NEGATIVE;
      ch->last_edge_to_check = MOTOR_STATE_ZERO_TO_NEGATIVE;
      ch->state = MOTOR_STATE_IN_NEGATIVE;

    /* No break; ADC value is in negative sine part */
    case MOTOR_STATE_IN_NEGATIVE:

      /* Filter negative peak level */
      ch->local_peak_value = MIN(ch->local_peak_value, new_val);
      if (new_val > ch->local_peak_value)
      {
        ch->negative_value = Motor_AccFilter(&ch->negative_acc_filter, ch->local_peak_value, MOTOR_PEAK_FILT_SIZE);
      }

      /* Check change of state and set the new one if needed */
      if (ch->to_zero_cnt >= MOTOR_CHANGE_SIZE)
      {
        ch->state = MOTOR_STATE_NEGATIVE_TO_ZERO;
      }
      break;

    /* Negative to zero sine part --------------------------------------------- */
    /* ADC value moved from negative sine part to zero */
    case MOTOR_STATE_NEGATIVE_TO_ZERO:

      /* Set new last transition timestamp and state */
      ch->last_edge_time = timestamp;
      ch->last_edge = MOTOR_STATE_NEGATIVE_TO_ZERO;
      ch->last_edge_to_check = MOTOR_STATE_NEGATIVE_TO_ZERO;
      ch->state = MOTOR_STATE_IN_ZERO_FROM_NEGATIVE;

    /* No break; ADC value is in zero sine part (got here from negative part) */
    case MOTOR_STATE_IN_ZERO_FROM_NEGATIVE:

      /* Filter zero level */
      ch->zero_value = Motor_AccFilter(&ch->zero_acc_filter, new_val, MOTOR_ZERO_FILT_SIZE);
      ch->local_peak_value = ch->zero_value;

      /* Check change of state and set the new one if needed */
      if (ch->to_positive_cnt >= MOTOR_CHANGE_SIZE)
      {
        ch->state = MOTOR_STATE_ZERO_TO_POSITIVE;
      }
      else if (ch->to_negative_cnt >= MOTOR_CHANGE_SIZE)
      {
        ch->state = MOTOR_STATE_ZERO_TO_NEGATIVE;
      }
      break;

    /* Unknown */
    default:
      ret |= STATUS_ERROR;
      break;
  }

  return ret;
}

/**
 * @brief  This function processes the state machine for the whole motor (both motor channels combined)
 *
 * @param  ch_A:      Pointer to motor channel A
 * @param  ch_B:      Pointer to motor channel B
 *
 * @param  last_dir:  Pointer where the last detected motor step direction is returned
 * @param  steps:     Pointer where the steps value is returned
 *
 * @param  min_steps: Minimum possible steps value
 * @param  max_steps: Maximum possible steps value
 *
 * @param  stats:     Pointer to statistics structure
 *
 * @retval Status_t
 */
#if (MOTOR_SHOW_STATS == 1)
static Status_t Motor_StepsStateMachine(Motor_Channel_t *ch_A, Motor_Channel_t *ch_B,
                                        int8_t *last_dir, int16_t *steps,
                                        int16_t min_steps, int16_t max_steps,
                                        Motor_Stats_t *stats)
#else
static Status_t Motor_StepsStateMachine(Motor_Channel_t *ch_A, Motor_Channel_t *ch_B,
                                        int8_t *last_dir, int16_t *steps,
                                        int16_t min_steps, int16_t max_steps)
#endif
{
  Status_t ret = STATUS_OK;
  int8_t detected_A = 0;
  int8_t detected_B = 0;

  /* Check new edges for motor channel A */
  if (ch_A->last_edge_to_check != MOTOR_STATE_IN_ZERO_FROM_START)
  {
    /* Decide action according to the edge type */
    switch (ch_A->last_edge_to_check)
    {
      case MOTOR_STATE_ZERO_TO_POSITIVE:

        /* Save stats */
        #if (MOTOR_SHOW_STATS == 1)
        stats->edges_A_up++;
        #endif

        switch (ch_B->last_edge)
        {
          case MOTOR_STATE_ZERO_TO_POSITIVE: detected_A = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_POSITIVE_TO_ZERO: /* TODO 2 */ detected_A = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_ZERO_TO_NEGATIVE: detected_A = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_NEGATIVE_TO_ZERO: /* TODO 1 */ detected_A = MOTOR_DIR_B_TO_A;
            break;
          default:
            break;
        }
        break;

      case MOTOR_STATE_POSITIVE_TO_ZERO:

        /* Save stats */
        #if (MOTOR_SHOW_STATS == 1)
        stats->edges_A_up_to_zero++;
        #endif

        switch (ch_B->last_edge)
        {
          case MOTOR_STATE_ZERO_TO_POSITIVE: detected_A = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_POSITIVE_TO_ZERO: /* TODO 1 */ detected_A = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_ZERO_TO_NEGATIVE: detected_A = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_NEGATIVE_TO_ZERO: /* TODO 2 */ detected_A = MOTOR_DIR_B_TO_A;
            break;
          default:
            break;
        }
        break;

      case MOTOR_STATE_ZERO_TO_NEGATIVE:

        /* Save stats */
        #if (MOTOR_SHOW_STATS == 1)
        stats->edges_A_down++;
        #endif

        switch (ch_B->last_edge)
        {
          case MOTOR_STATE_ZERO_TO_POSITIVE: detected_A = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_POSITIVE_TO_ZERO: /* TODO 1 */ detected_A = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_ZERO_TO_NEGATIVE: detected_A = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_NEGATIVE_TO_ZERO: /* TODO 2 */ detected_A = MOTOR_DIR_A_TO_B;
            break;
          default:
            break;
        }
        break;

      case MOTOR_STATE_NEGATIVE_TO_ZERO:

        /* Save stats */
        #if (MOTOR_SHOW_STATS == 1)
        stats->edges_A_down_to_zero++;
        #endif

        switch (ch_B->last_edge)
        {
          case MOTOR_STATE_ZERO_TO_POSITIVE: detected_A = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_POSITIVE_TO_ZERO: /* TODO 2 */ detected_A = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_ZERO_TO_NEGATIVE: detected_A = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_NEGATIVE_TO_ZERO: /* TODO 1 */ detected_A = MOTOR_DIR_A_TO_B;
            break;
          default:
            break;
        }
        break;

      default:
        break;
    }

    /* Save stats */
    #if (MOTOR_SHOW_STATS == 1)
    stats->edges_buffer[stats->edges_buffer_it].ch = MOTOR_CHANNEL_A;
    stats->edges_buffer[stats->edges_buffer_it].detected_edge = ch_A->last_edge_to_check;
    stats->edges_buffer[stats->edges_buffer_it].compared_edge = ch_B->last_edge;
    stats->edges_buffer[stats->edges_buffer_it].correct = (detected_A != 0) ? true : false;
    if (stats->edges_buffer_it < MOTOR_EDGES_BUFFER)
    {
      stats->edges_buffer_it++;
    }
    #endif

    /* Clear edge that has been checked */
    ch_A->last_edge_to_check = MOTOR_STATE_IN_ZERO_FROM_START;
  }

  /* Check new edges for motor channel B */
  if (ch_B->last_edge_to_check != MOTOR_STATE_IN_ZERO_FROM_START)
  {
    /* Decide action according to the edge type */
    switch (ch_B->last_edge_to_check)
    {
      case MOTOR_STATE_ZERO_TO_POSITIVE:

        /* Save stats */
        #if (MOTOR_SHOW_STATS == 1)
        stats->edges_B_up++;
        #endif

        switch (ch_A->last_edge)
        {
          case MOTOR_STATE_ZERO_TO_POSITIVE: detected_B = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_POSITIVE_TO_ZERO: /* TODO 2 */ detected_B = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_ZERO_TO_NEGATIVE: detected_B = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_NEGATIVE_TO_ZERO: /* TODO 1 */ detected_B = MOTOR_DIR_A_TO_B;
            break;
          default:
            break;
        }
        break;

      case MOTOR_STATE_POSITIVE_TO_ZERO:

        /* Save stats */
        #if (MOTOR_SHOW_STATS == 1)
        stats->edges_B_up_to_zero++;
        #endif

        switch (ch_A->last_edge)
        {
          case MOTOR_STATE_ZERO_TO_POSITIVE: detected_B = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_POSITIVE_TO_ZERO: /* TODO 1 */ detected_B = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_ZERO_TO_NEGATIVE: detected_B = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_NEGATIVE_TO_ZERO: /* TODO 2 */ detected_B = MOTOR_DIR_A_TO_B;
            break;
          default:
            break;
        }
        break;

      case MOTOR_STATE_ZERO_TO_NEGATIVE:

        /* Save stats */
        #if (MOTOR_SHOW_STATS == 1)
        stats->edges_B_down++;
        #endif

        switch (ch_A->last_edge)
        {
          case MOTOR_STATE_ZERO_TO_POSITIVE: detected_B = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_POSITIVE_TO_ZERO: /* TODO 1 */ detected_B = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_ZERO_TO_NEGATIVE: detected_B = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_NEGATIVE_TO_ZERO: /* TODO 2 */ detected_B = MOTOR_DIR_B_TO_A;
            break;
          default:
            break;
        }
        break;

      case MOTOR_STATE_NEGATIVE_TO_ZERO:

        /* Save stats */
        #if (MOTOR_SHOW_STATS == 1)
        stats->edges_B_down_to_zero++;
        #endif

        switch (ch_A->last_edge)
        {
          case MOTOR_STATE_ZERO_TO_POSITIVE: detected_B = MOTOR_DIR_B_TO_A;
            break;
          case MOTOR_STATE_POSITIVE_TO_ZERO: /* TODO 2 */ detected_B = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_ZERO_TO_NEGATIVE: detected_B = MOTOR_DIR_A_TO_B;
            break;
          case MOTOR_STATE_NEGATIVE_TO_ZERO: /* TODO 1 */ detected_B = MOTOR_DIR_B_TO_A;
            break;
          default:
            break;
        }
        break;

      default:
        break;
    }

    /* Save stats */
    #if (MOTOR_SHOW_STATS == 1)
    stats->edges_buffer[stats->edges_buffer_it].ch = MOTOR_CHANNEL_B;
    stats->edges_buffer[stats->edges_buffer_it].detected_edge = ch_B->last_edge_to_check;
    stats->edges_buffer[stats->edges_buffer_it].compared_edge = ch_A->last_edge;
    stats->edges_buffer[stats->edges_buffer_it].correct = (detected_B != 0) ? true : false;
    if (stats->edges_buffer_it < MOTOR_EDGES_BUFFER)
    {
      stats->edges_buffer_it++;
    }
    #endif

    /* Clear edge that has been checked */
    ch_B->last_edge_to_check = MOTOR_STATE_IN_ZERO_FROM_START;
  }

  /* If state that means steps change was detected -> increment steps */
  if (detected_A != 0 || detected_B != 0)
  {
    /* Increment steps value */
    *steps = *steps + detected_A;
    *steps = *steps + detected_B;
    *last_dir = detected_A + detected_B;

    /* Saturate steps value */
    SAT_DOWN(*steps, min_steps);
    SAT_UP(*steps, max_steps);
  }

  return ret;
}

/**
 * @brief  This function computes the accumulation filter.
 *
 * @param  acc_filter: Pointer to the accumulation filter
 * @param  add_value:  Value to add to the filter
 * @param  size:       Size of the filter
 *
 * @retval uint16_t: Filtered value (add_value if error)
 */
static uint16_t Motor_AccFilter(Motor_Acc_Filt_t *acc_filter, uint16_t add_value, uint16_t size)
{
  /* Compute filter and return result */
  if (acc_filter->it < size)
  {
    acc_filter->it++;
  }
  else
  {
    acc_filter->value -= (acc_filter->value / size);
  }
  acc_filter->value += add_value;
  return (uint16_t)(acc_filter->value / acc_filter->it);
}

/**
 * @brief  This function computes motor channel state counters.
 *
 * @param  cnt_to_inc:   Counter to increment
 * @param  cnt_to_dec_1: Counter to decrement
 * @param  cnt_to_dec_2: Counter to decrement
 *
 * @retval None
 */
static Status_t Motor_StateCounter(uint8_t *cnt_to_inc, uint8_t *cnt_to_dec_1, uint8_t *cnt_to_dec_2)
{
  /* Process counters */
  if (*cnt_to_inc < MOTOR_CHANGE_SIZE)
  {
    *cnt_to_inc = *cnt_to_inc + 1;
  }
  if (*cnt_to_dec_1 > 0)
  {
    *cnt_to_dec_1 = *cnt_to_dec_1 - 1;
  }
  if (*cnt_to_dec_2 > 0)
  {
    *cnt_to_dec_2 = *cnt_to_dec_2 - 1;
  }

  return STATUS_OK;
}

/**
 * @brief  Clear captured times of edges.
 *
 * @param  ch:        Pointer to motor channel

 * @param  dir:       Which channel is selected in the current function call
 *                    (MOTOR_DIR_A_TO_B or MOTOR_DIR_B_TO_A)
 * @param  last_dir:  Pointer to the last detected step direction of motor connected to selected motor channel
 * @param  steps:     Pointer to the steps value of motor connected to selected motor channel
 *
 * @param  min_steps: Minimum possible steps value
 * @param  max_steps: Maximum possible steps value
 *
 * @param  stats:     Pointer to statistics structure
 *
 * @retval Status_t
 */
#if (MOTOR_SHOW_STATS == 1)
static Status_t Motor_ClearEdgeTimes(Motor_Channel_t *ch,
                                     int8_t dir, int8_t *last_dir, int16_t *steps,
                                     int16_t min_steps, int16_t max_steps,
                                     Motor_Stats_t *motor_stats)
#else
static Status_t Motor_ClearEdgeTimes(Motor_Channel_t *ch,
                                     int8_t dir, int8_t *last_dir, int16_t *steps,
                                     int16_t min_steps, int16_t max_steps)
#endif
{
  /* Clear edges times */
  ch->last_zero_to_positive_time = INT32_MIN;
  ch->last_zero_to_negative_time = INT32_MIN;
  ch->last_edge_time = INT32_MIN;
  ch->last_period = INT32_MIN;

  /* Clear motor channel and edge states if requested */
  if (dir == MOTOR_DIR_A_TO_B)
  {
    ch->state = MOTOR_STATE_IN_ZERO_FROM_START;
    ch->last_edge = MOTOR_STATE_IN_ZERO_FROM_START;
    ch->last_edge_to_check = MOTOR_STATE_IN_ZERO_FROM_START;
    #if (MOTOR_SHOW_STATS == 1)
    stats->canceled_edges_A++;
    #endif
  }
  else if (dir == MOTOR_DIR_B_TO_A)
  {
    ch->state = MOTOR_STATE_IN_ZERO_FROM_START;
    ch->last_edge = MOTOR_STATE_IN_ZERO_FROM_START;
    ch->last_edge_to_check = MOTOR_STATE_IN_ZERO_FROM_START;
    #if (MOTOR_SHOW_STATS == 1)
    stats->canceled_edges_B++;
    #endif
  }

  /* If motor movement has ended */
  if (*last_dir != 0)
  {
    /* Decrement steps value by 2 according to the last detected steps direction
     * NOTE: This is a workaround, since testboard detects from one to two extra edges at the end of each movement
     * If steps count is then odd, increment back by 1 according to the direction once more
     * NOTE: This rounding to odd numbers is possible since DHW-21x counts motor movement in steps
     *       that are divided by 2 in comparison to what the testboard detects. */
    *steps = (*steps) - (2 * (*last_dir));
    if (((*steps) % 2) != 0)
    {
      *steps = (*steps) + (*last_dir);
    }
    SAT_DOWN(*steps, min_steps);
    SAT_UP(*steps, max_steps);
    *last_dir = 0;
  }

  return STATUS_OK;
}

#if (MOTOR_SWV_PRINT == 1)
/**
 * @brief  This function checks the trigger to start ADC data printing to SWV.
 *
 * @param  i: Buffer iterator
 * @param  j: Iterator of value in the buffer
 *
 * @retval Status_t: STATUS_BUSY - if trigger detected
 *                   STATUS_OK   - otherwise
 */
static Status_t Motor_CheckTrigger(uint8_t i, uint8_t j)
{
  Status_t ret = STATUS_OK;

  /* If trigger not set yet */
  if (motor.print_trigger < 0)
  {
    /* If one of the motors moved */
    if (motor._1_A.last_edge != MOTOR_STATE_IN_ZERO_FROM_START ||
        motor._1_B.last_edge != MOTOR_STATE_IN_ZERO_FROM_START)
    {
      /* Set trigger to buffer before current sample position */
      if (i == 0)
      {
        motor.print_trigger = MOTOR_BUF_CNT - 1;
      }
      else
      {
        motor.print_trigger = i - 1;
      }
      motor.print_it = 0;
    }
  }

  /* If trigger set */
  else
  {
    /* Count samples to print */
    motor.print_it++;
    if (motor.print_it >= MOTOR_BUF_CNT * MOTOR_BUF_SIZE)
    {
      /* If limit reached -> indicate to returned status */
      ret |= STATUS_BUSY;
    }
  }

  return ret;
}
#endif

#if (MOTOR_SWV_PRINT == 1)
/**
 * @brief  This function prints ADC data to SWV.
 *
 * @param  None
 *
 * @retval Status_t
 */
static Status_t Motor_PrintData()
{
  Status_t ret = STATUS_OK;

  /* If printing iterator reached the limit */
  if (motor.print_it >= MOTOR_BUF_CNT * MOTOR_BUF_SIZE)
  {
    /* Stop ADC measurement */
    motor.print_it = 0;
    if (HAL_ADC_Stop_DMA(&hadc_motor) != HAL_OK)
    {
      ret |= STATUS_ERROR;
    }
    if (HAL_TIM_OC_Stop(&htim_motor, MOTOR_TIM_CH) != HAL_OK)
    {
      ret |= STATUS_ERROR;
    }

    /* Go through buffer from trigger position all around, blink with green LEDs */
    (void)Control_Led(LED_G_1, false, GPIO_PIN_RESET);
    (void)Control_Led(LED_G_2, false, GPIO_PIN_RESET);
    (void)Control_Led(LED_R_1, false, GPIO_PIN_RESET);
    (void)Control_Led(LED_R_2, false, GPIO_PIN_RESET);
    PRINTF("Time[us],ADC_1A,ADC_1B,ADC_2A,ADC_2B\n");
    for (int i = motor.print_trigger; i < motor.print_trigger + MOTOR_BUF_CNT; i++)
    {
      for (uint8_t j = 0; j < MOTOR_BUF_SIZE; j++)
      {
        /* Print ADC values to SWV */
        ret |= System_ReloadWdg();
        PRINTF("%d,%d,%d,\n", (int)motor.print_it * MOTOR_ADC_SAMPLE_PER,
               (int)Motor_AccFilter(&motor._1_A.ADC_filter, motor.adc_buf[i % MOTOR_BUF_CNT][j]._1_A, MOTOR_FILT_SIZE),
               (int)Motor_AccFilter(&motor._1_B.ADC_filter, motor.adc_buf[i % MOTOR_BUF_CNT][j]._1_B, MOTOR_FILT_SIZE));
        ret |= System_ReloadWdg();
        HAL_Delay(10);
        if (j % 10 == 0)
        {
          (void)Control_Led(LED_G_1, true, GPIO_PIN_SET);
          (void)Control_Led(LED_G_2, true, GPIO_PIN_SET);
          (void)Control_Led(LED_R_1, false, GPIO_PIN_RESET);
          (void)Control_Led(LED_R_2, false, GPIO_PIN_RESET);
        }
        motor.print_it++;
      }
      ret |= System_ReloadWdg();
      HAL_Delay(10);
    }

    /* Stuck here forever, turn green LEDs on */
    while (1)
    {
      (void)Control_Led(LED_G_1, false, GPIO_PIN_SET);
      (void)Control_Led(LED_G_2, false, GPIO_PIN_SET);
      (void)Control_Led(LED_R_1, false, GPIO_PIN_RESET);
      (void)Control_Led(LED_R_2, false, GPIO_PIN_RESET);
      (void)System_ReloadWdg();
      __disable_irq();
    }
  }

  return ret;
}
#endif

/**
 * @brief  This function initializes HAL for motor module.
 *
 * @param  None
 *
 * @retval Status_t
 */
static Status_t Motor_InitHAL(void)
{
  Status_t ret = STATUS_OK;

  /* Start the motor current measurement trigger timer */
  motor.buf_it++;
  if (motor.buf_it >= MOTOR_BUF_CNT)
  {
    motor.buf_it = 0;
  }
  if (HAL_ADC_Start_DMA(&hadc_motor, (uint32_t *)motor.adc_buf[motor.buf_it], MOTOR_CH_CNT * MOTOR_BUF_SIZE) != HAL_OK)
  {
    ret |= STATUS_ERROR;
  }
  if (HAL_TIM_OC_Start(&htim_motor, MOTOR_TIM_CH) != HAL_OK)
  {
    ret |= STATUS_ERROR;
  }

  return ret;
}

/** @} */
