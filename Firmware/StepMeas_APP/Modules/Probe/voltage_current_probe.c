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

/* --- obecná struktura filtru --- */
#define MOVAVG_LEN 3
typedef struct {
    float exp_avg;
    float movavg_buf[MOVAVG_LEN];
    uint8_t movavg_index;
    uint8_t movavg_count;
    uint8_t initialized;
    float last_value;
} Filter_t;

/* Private variables ---------------------------------------------------------*/
static volatile capture_state_t capture_state = CAPT_WAIT_TRIGGER;
static volatile uint32_t capture_count = 0;
static uint16_t capture_buffer[CAPTURE_SAMPLES];         // hlavní kanál
static uint16_t capture_buffer_ch2[CAPTURE_SAMPLES];     // extra kanál
static float capture_buffer_angle[CAPTURE_SAMPLES];    // buffer pro úhel ve stupních
static volatile uint8_t capture_ready = 0;
int16_t angle_deg = 0;

/* Dva filtry – hlavní a extra */
static Filter_t filter_main;
static Filter_t filter_extra;

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
static inline void Filter_Reset(Filter_t *f);
static inline uint16_t Filter_Step(Filter_t *f, uint16_t x);

/* Public functions ----------------------------------------------------------*/
void Probe_Init(void)
{
    Probe_InitHAL();
    Filter_Reset(&filter_main);
    Filter_Reset(&filter_extra);
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

/* Obecné funkce pro filtr */
static inline void Filter_Reset(Filter_t *f)
{
    f->exp_avg = 0.0f;
    f->movavg_index = 0;
    f->movavg_count = 0;
    f->initialized = 0;
    f->last_value = 0.0f;
}

static inline uint16_t Filter_Step(Filter_t *f, uint16_t x)
{
    if (!f->initialized)
    {
        f->exp_avg = (float)x;
        f->last_value = f->exp_avg;
        f->initialized = 1;
        f->movavg_buf[0] = f->exp_avg;
        f->movavg_index = 1;
        f->movavg_count = 1;
        return (uint16_t)f->exp_avg;
    }
    else
    {
        f->exp_avg = (0.9f * f->exp_avg) + (0.1f * (float)x);
    }

    float diff = fabsf(f->exp_avg - f->last_value);

    if (diff <= 25.0f)
    {
        f->movavg_buf[f->movavg_index] = f->exp_avg;
        f->movavg_index = (f->movavg_index + 1) % MOVAVG_LEN;
        if (f->movavg_count < MOVAVG_LEN) f->movavg_count++;

        float sum = 0.0f;
        for (uint8_t i = 0; i < f->movavg_count; i++)
            sum += f->movavg_buf[i];

        f->last_value = sum / f->movavg_count;
    }
    else
    {
        f->last_value = f->exp_avg;
    }

    return (uint16_t)f->last_value;
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

/* Capture handler -----------------------------------------------------------*/
static inline void Capture_HandleSample(uint16_t adc_sample[])
{
    uint16_t raw_main  = adc_sample[MONITOR_CHANNEL];
    uint16_t raw_extra = adc_sample[EXTRA_CHANNEL];

    uint16_t filt_main  = Filter_Step(&filter_main, raw_main);
    uint16_t filt_extra = Filter_Step(&filter_extra, raw_extra);

    float angle_rad = atan2f((float)raw_main - MIDDLE_VALUE,
                             (float)raw_extra - MIDDLE_VALUE);
    angle_deg= (int16_t)(angle_rad * (180.0f / M_PI));

    switch (capture_state)
    {
        case CAPT_WAIT_TRIGGER:
            if (abs(raw_main - TRIGGER_THRESHOLD) > 50)
            {
                capture_state = CAPT_CAPTURING;
                capture_count = 0;
                Filter_Reset(&filter_main);
                Filter_Reset(&filter_extra);

                filt_main  = Filter_Step(&filter_main, raw_main);
                filt_extra = Filter_Step(&filter_extra, raw_extra);
                angle_rad  = atan2f((float)raw_main - MIDDLE_VALUE,
                                    (float)raw_extra - MIDDLE_VALUE);
                angle_deg  = (int16_t)(angle_rad * (180.0f / M_PI));

                capture_buffer[capture_count]       = filt_main;
                capture_buffer_ch2[capture_count]   = filt_extra;
                capture_buffer_angle[capture_count] = angle_deg;

                capture_count++;
            }
            break;

        case CAPT_CAPTURING:
            if (capture_count < CAPTURE_SAMPLES)
            {
                filt_main  = Filter_Step(&filter_main, raw_main);
                filt_extra = Filter_Step(&filter_extra, raw_extra);
                angle_rad  = atan2f((float)filt_main - MIDDLE_VALUE,
                                    (float)filt_extra - MIDDLE_VALUE);
                angle_deg  = (int16_t)(angle_rad * (180.0f / M_PI));

                capture_buffer[capture_count]       = filt_main;
                capture_buffer_ch2[capture_count]   = filt_extra;
                capture_buffer_angle[capture_count] = angle_deg;

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

}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {

        Capture_HandleSample(adc_values);
        adc_ready = 1;
    }
}

/** @} */
