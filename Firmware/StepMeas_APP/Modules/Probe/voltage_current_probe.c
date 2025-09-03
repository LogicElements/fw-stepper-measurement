/**
 * @file       voltage_current_probe.c
 * @brief      ADC measurement and motor probe handling
 * @addtogroup grGroup
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include "voltage_current_probe.h"
#include "main.h"
#include <math.h>   // atan2, M_PI

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Private definitions -------------------------------------------------------*/

/* ADC channel mapping (based on order in adc_values[]) */
#define ADC_CHANNEL_VA2   0  // Voltage Phase A - Motor 2
#define ADC_CHANNEL_VB2   1  // Voltage Phase B - Motor 2
#define ADC_CHANNEL_VA1   2  // Voltage Phase A - Motor 1
#define ADC_CHANNEL_VB1   3  // Voltage Phase B - Motor 1
#define ADC_CHANNEL_IA2   4  // Current Phase A - Motor 2
#define ADC_CHANNEL_IB2   5  // Current Phase B - Motor 2
#define ADC_CHANNEL_IA1   6  // Current Phase A - Motor 1
#define ADC_CHANNEL_IB1   7  // Current Phase B - Motor 1

#define ADC_CHANNEL_COUNT 4
#define STEP_THRESHOLD    40
#define MIDDLE_VALUE      2045  // Current change detection threshold

#define CAPTURE_SAMPLES     10000
#define TRIGGER_THRESHOLD    2050
#define MONITOR_CHANNEL      1       // hlavní monitorovaný kanál
#define EXTRA_CHANNEL        0       // druhý kanál, který také ukládáme

/* Private typedefs ----------------------------------------------------------*/
typedef enum
{
    CAPT_WAIT_TRIGGER = 0,
    CAPT_CAPTURING,
    CAPT_DONE
} capture_state_t;

/* Private variables ---------------------------------------------------------*/
static volatile capture_state_t capture_state = CAPT_WAIT_TRIGGER;
static volatile uint32_t capture_count = 0;
static uint16_t capture_buffer[CAPTURE_SAMPLES];         // hlavní kanál
static uint16_t capture_buffer_ch2[CAPTURE_SAMPLES];     // extra kanál
static float capture_buffer_angle[CAPTURE_SAMPLES]; // nový buffer pro úhel (deg)
static volatile uint8_t capture_ready = 0;

/* --- filtr pro hlavní kanál --- */
#define MOVAVG_LEN 3
static float exp_avg = 0.0f;
static float movavg_buf[MOVAVG_LEN];
static uint8_t movavg_index = 0;
static uint8_t movavg_count = 0;
static uint8_t custom_filt_init = 0;

/* --- filtr pro extra kanál --- */
static float exp_avg_ch2 = 0.0f;
static float movavg_buf_ch2[MOVAVG_LEN];
static uint8_t movavg_index_ch2 = 0;
static uint8_t movavg_count_ch2 = 0;
static uint8_t custom_filt_init_ch2 = 0;

/* Structures ----------------------------------------------------------------*/
typedef struct
{
    uint16_t _A;
    uint16_t _B;
} Voltage_t;
typedef struct
{
    uint16_t _A;
    uint16_t _B;
} Current_t;
typedef struct
{
    Current_t current;
    Voltage_t voltage;
} Motor_t;
typedef struct
{
    Motor_t motor1;
    Motor_t motor2;
    int16_t value;
} Probe_private_t;

/* Module variables ----------------------------------------------------------*/
static Probe_private_t probe;
extern ADC_HandleTypeDef hadc1;

uint16_t probe_it = 0;
uint16_t TimeOK = 0;
int16_t value = 0;

uint16_t VA = 0;
uint16_t VB = 0;
uint32_t IA = 0;
uint16_t IB = 0;
uint16_t currentStatus = 1;
uint16_t adc_value = 0;

uint16_t adc_values[ADC_CHANNEL_COUNT];
volatile uint8_t adc_ready = 1;
uint16_t adc_ready_counter = 0;

/* Private prototypes --------------------------------------------------------*/
void Probe_InitHAL(void);
static inline void Capture_HandleSample(uint16_t adc_sample[]);
static inline uint16_t custom_filter_step(uint16_t x);
static inline void custom_filter_reset(void);
static inline uint16_t custom_filter_step_ch2(uint16_t x);
static inline void custom_filter_reset_ch2(void);

/* Public functions ----------------------------------------------------------*/
void Probe_Init(void)
{
    Probe_InitHAL();
}

void Probe_Handle(void)
{

}

/* Callback functions --------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void Probe_InitHAL(void)
{
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        // error handler
    }
}

static inline void custom_filter_reset(void)
{
    custom_filt_init = 0;
    movavg_index = 0;
    movavg_count = 0;
    exp_avg = 0.0f;
}

static inline uint16_t custom_filter_step(uint16_t x)
{
    static float last_value = 0.0f;

    if (!custom_filt_init)
    {
        exp_avg = (float) x;
        last_value = exp_avg;
        custom_filt_init = 1;
        movavg_buf[0] = exp_avg;
        movavg_index = 1;
        movavg_count = 1;
        return (uint16_t) exp_avg;
    }
    else
    {
        exp_avg = (0.7f * exp_avg) + (0.3f * (float) x);
    }

    float diff = fabsf(exp_avg - last_value);

    if (diff <= 25.0f)
    {
        movavg_buf[movavg_index] = exp_avg;
        movavg_index = (movavg_index + 1) % MOVAVG_LEN;
        if (movavg_count < MOVAVG_LEN)
            movavg_count++;

        float sum = 0.0f;
        for (uint8_t i = 0; i < movavg_count; i++)
            sum += movavg_buf[i];

        last_value = sum / movavg_count;
    }
    else
    {
        last_value = exp_avg;
    }

    return (uint16_t) last_value;
}

/* --- filtr pro extra kanál --- */
static inline void custom_filter_reset_ch2(void)
{
    custom_filt_init_ch2 = 0;
    movavg_index_ch2 = 0;
    movavg_count_ch2 = 0;
    exp_avg_ch2 = 0.0f;
}

static inline uint16_t custom_filter_step_ch2(uint16_t x)
{
    static float last_value_ch2 = 0.0f;

    if (!custom_filt_init_ch2)
    {
        exp_avg_ch2 = (float) x;
        last_value_ch2 = exp_avg_ch2;
        custom_filt_init_ch2 = 1;
        movavg_buf_ch2[0] = exp_avg_ch2;
        movavg_index_ch2 = 1;
        movavg_count_ch2 = 1;
        return (uint16_t) exp_avg_ch2;
    }
    else
    {
        exp_avg_ch2 = (0.7f * exp_avg_ch2) + (0.3f * (float) x);
    }

    float diff = fabsf(exp_avg_ch2 - last_value_ch2);

    if (diff <= 25.0f)
    {
        movavg_buf_ch2[movavg_index_ch2] = exp_avg_ch2;
        movavg_index_ch2 = (movavg_index_ch2 + 1) % MOVAVG_LEN;
        if (movavg_count_ch2 < MOVAVG_LEN)
            movavg_count_ch2++;

        float sum = 0.0f;
        for (uint8_t i = 0; i < movavg_count_ch2; i++)
            sum += movavg_buf_ch2[i];

        last_value_ch2 = sum / movavg_count_ch2;
    }
    else
    {
        last_value_ch2 = exp_avg_ch2;
    }

    return (uint16_t) last_value_ch2;
}

/* ADC measurement start -----------------------------------------------------*/
void StartAdcMeasurement(void)
{
    if (adc_ready)
    {
        adc_ready = 0;
        if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_values, ADC_CHANNEL_COUNT) != HAL_OK)
        {
            // chyba při startu – můžeš zavolat Error_Handler nebo nastavit flag
        }
    }
    else
    {
        adc_ready_counter++;
    }
}



static inline void Capture_HandleSample(uint16_t adc_sample[])
{
    uint16_t raw_main  = adc_sample[MONITOR_CHANNEL];
    uint16_t raw_extra = adc_sample[EXTRA_CHANNEL];

    // filtrované hodnoty
    uint16_t filt_main  = custom_filter_step(raw_main);
    uint16_t filt_extra = custom_filter_step_ch2(raw_extra);

    // úhel počítáme z RAW hodnot (odečtený střed), výstup ve STUPNÍCH
    float angle_rad = atan2f((float)raw_main - MIDDLE_VALUE,
                             (float)raw_extra - MIDDLE_VALUE);
    int16_t angle_deg = (int16_t)(angle_rad * (180.0f / M_PI));

    switch (capture_state)
    {
        case CAPT_WAIT_TRIGGER:
            if (abs(raw_main - TRIGGER_THRESHOLD) > 50)
            {
                capture_state = CAPT_CAPTURING;
                capture_count = 0;
                custom_filter_reset();
                custom_filter_reset_ch2();

                filt_main  = custom_filter_step(raw_main);
                filt_extra = custom_filter_step_ch2(raw_extra);
                angle_rad  = atan2f((float)raw_main - MIDDLE_VALUE,
                                    (float)raw_extra - MIDDLE_VALUE);
                angle_deg  = (int16_t)(angle_rad * (180.0f / M_PI));

                capture_buffer[capture_count]       = filt_main;
                capture_buffer_ch2[capture_count]   = filt_extra;
                capture_buffer_angle[capture_count] = angle_deg; // uložit °

                capture_count++;
            }
            break;

        case CAPT_CAPTURING:
            if (capture_count < CAPTURE_SAMPLES)
            {
                filt_main  = custom_filter_step(raw_main);
                filt_extra = custom_filter_step_ch2(raw_extra);
                angle_rad  = atan2f((float)raw_main - MIDDLE_VALUE,
                                    (float)raw_extra - MIDDLE_VALUE);
                angle_deg  = (int16_t)(angle_rad * (180.0f / M_PI));

                capture_buffer[capture_count]       = filt_main;
                capture_buffer_ch2[capture_count]   = filt_extra;
                capture_buffer_angle[capture_count] = angle_deg; // uložit °

                capture_count++;

                if (capture_count >= CAPTURE_SAMPLES)
                {
                    capture_state = CAPT_DONE;
                    capture_ready = 1;
                }
            }
            break;

        case CAPT_DONE:
        default:
            break;
    }
}

/* Conversion and processing -------------------------------------------------*/
void ProcessMotorSteps(uint16_t adc_sample[])
{
    int16_t I1 = probe.motor2.current._A - MIDDLE_VALUE;
    int16_t I2 = probe.motor2.current._B - MIDDLE_VALUE;

    float angle_rad = atan2f((float) I1, (float) I2);
    float angle_deg = angle_rad * (180.0f / M_PI);
    (void) angle_deg; // pokud nepoužíváš, aby nezůstal warning

    adc_ready = 1;
}

void ADC_Conversion(uint16_t _adc_values[])
{
    probe.motor1.current._A = _adc_values[ADC_CHANNEL_IA1];
    probe.motor1.current._B = _adc_values[ADC_CHANNEL_IB1];
    probe.motor1.voltage._A = _adc_values[ADC_CHANNEL_VA1];
    probe.motor1.voltage._B = _adc_values[ADC_CHANNEL_VB1];
    probe.motor2.current._A = _adc_values[ADC_CHANNEL_IA2];
    probe.motor2.current._B = _adc_values[ADC_CHANNEL_IB2];
    probe.motor2.voltage._A = _adc_values[ADC_CHANNEL_VA2];
    probe.motor2.voltage._B = _adc_values[ADC_CHANNEL_VB2];
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        ADC_Conversion(adc_values);

        Capture_HandleSample(adc_values);

        adc_ready = 1;
    }
}

/** @} */
