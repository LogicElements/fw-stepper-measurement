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

#define ADC_CHANNEL_COUNT 8
#define STEP_THRESHOLD    40
#define MIDDLE_VALUE      2045  // Current change detection threshold

#define CAPTURE_SAMPLES      20000
#define TRIGGER_THRESHOLD    2000
#define MONITOR_CHANNEL      3

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
static uint16_t capture_buffer[CAPTURE_SAMPLES];
static volatile uint8_t capture_ready = 0;

static uint32_t filt50_y = 0;
static uint8_t filt50_init = 0;




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
static Status_t Probe_InitHAL(void);
static inline void Capture_HandleSample(uint16_t adc_sample[]);
static inline uint16_t filt50_step(uint16_t x);
static inline void filt50_reset(void);

/* Public functions ----------------------------------------------------------*/
Status_t Probe_Init(void)
{
    Status_t ret = STATUS_OK;
    ret |= Probe_InitHAL();
    return ret;
}

Status_t Probe_Handle(void)
{
    return STATUS_OK; // zatím prázdné
}

/* Callback functions --------------------------------------------------------*/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

}

/* Private functions ---------------------------------------------------------*/
static Status_t Probe_InitHAL(void)
{
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

static inline void filt50_reset(void)
{
    filt50_init = 0;
}

static inline uint16_t filt50_step(uint16_t x)
{
    if (!filt50_init)
    {
        filt50_y = x;
        filt50_init = 1;
        return (uint16_t) filt50_y;
    }
    filt50_y = ((filt50_y * 3u) + ((uint32_t) x * 7u) + 5u) / 10u;
    return (uint16_t) filt50_y;
}

/* ADC measurement start -----------------------------------------------------*/
void StartAdcMeasurement(void)
{
    if (adc_ready)
    {
        adc_ready = 0;


    }
    else
    {
        adc_ready_counter++;
    }
}

static inline void Capture_HandleSample(uint16_t adc_sample[])
{
    uint16_t raw = adc_sample[MONITOR_CHANNEL];
    uint16_t filt = filt50_step(raw);

    switch (capture_state)
    {
        case CAPT_WAIT_TRIGGER:
            if (raw < TRIGGER_THRESHOLD)
            {
                capture_state = CAPT_CAPTURING;
                capture_count = 0;
                filt50_reset();
                filt = filt50_step(raw);
                capture_buffer[capture_count++] = filt;
            }
            break;

        case CAPT_CAPTURING:
            if (capture_count < CAPTURE_SAMPLES)
            {
                filt = filt50_step(raw);
                capture_buffer[capture_count++] = filt;
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

/** @} */
