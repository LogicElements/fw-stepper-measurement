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
#define EXTRA_CHANNEL        0
#define ANGLE_BUFFER_LEN    20
#define STEP_TOLERANCE 4
#define STEP_ANGLES_COUNT (sizeof(step_angles)/sizeof(step_angles[0]))

/* Private typedefs ----------------------------------------------------------*/
typedef enum
{
    CAPT_WAIT_TRIGGER = 0,
    CAPT_CAPTURING,
    CAPT_DONE
} capture_state_t;

/* --- obecná struktura filtru --- */
#define MOVAVG_LEN 3
typedef struct
{
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
static volatile uint16_t capture_buffer[CAPTURE_SAMPLES];        // hlavní kanál
static volatile uint16_t capture_buffer_ch2[CAPTURE_SAMPLES];     // extra kanál
static float volatile capture_buffer_angle[CAPTURE_SAMPLES];
//static volatile int32_t capture_buffer_steps[CAPTURE_SAMPLES]; // buffer pro úhel ve stupních
static volatile uint8_t capture_ready = 0;
volatile int16_t angle_deg = 0;

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

typedef enum
{
    STEP_IDLE = 0,
    STEP_ACTIVE
} step_state_t;

typedef enum
{
    DIR_NONE = 0,
    DIR_CW,     // Clockwise
    DIR_CCW     // Counter-clockwise
} rotation_dir_t;

typedef struct
{
    int16_t buf[ANGLE_BUFFER_LEN];
    uint8_t count;
} AngleBufferFIFO_t;

static AngleBufferFIFO_t angle_buffer;

static step_state_t step_state = STEP_IDLE;
/* Module variables ----------------------------------------------------------*/
//static Probe_private_t probe;
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
static uint8_t sample_divider = 0;

uint16_t adc_values[ADC_CHANNEL_COUNT];
volatile uint8_t adc_ready = 1;
uint16_t adc_ready_counter = 0;

static int16_t step_count = 0;          // celkový počet kroků
static volatile int16_t last_step_angle = 9999; // poslední úhel, který způsobil krok

/* Kontrolní úhly */
static const int16_t step_angles[] =
{ 178, 90, 0, -90, -178 };

volatile rotation_dir_t dir = DIR_NONE;

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

static inline void AngleBufferFIFO_Reset(AngleBufferFIFO_t *ab)
{
    ab->count = 0;
}

/* Přidání nové hodnoty */
static inline void AngleBufferFIFO_Add(AngleBufferFIFO_t *ab, int16_t angle)
{
    // pokud se úhel dostane na hranici, resetujeme
    if (angle > 178 || angle < -178)
    {
        AngleBufferFIFO_Reset(ab);
        ab->buf[0] = angle;
        ab->count = 1;
        return;
    }

    if (ab->count < ANGLE_BUFFER_LEN)
    {
        ab->buf[ab->count++] = angle;
    }
    else
    {
        for (uint8_t i = 0; i < ANGLE_BUFFER_LEN - 1; i++)
            ab->buf[i] = ab->buf[i + 1];

        ab->buf[ANGLE_BUFFER_LEN - 1] = angle;
    }
}

/* Detekce směru – porovnání první a poslední hodnoty */
static rotation_dir_t Detect_RotationDirection(AngleBufferFIFO_t *ab)
{
    if (ab->count < ANGLE_BUFFER_LEN)
        return DIR_NONE;

    int16_t first = ab->buf[0];
    int16_t last = ab->buf[ab->count - 1];

    int16_t diff = last - first;

    if (diff > 0)
        return DIR_CW;   // rostoucí
    if (diff < 0)
        return DIR_CCW;  // klesající
    return DIR_NONE;
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
        f->exp_avg = (float) x;
        f->last_value = f->exp_avg;
        f->initialized = 1;
        f->movavg_buf[0] = f->exp_avg;
        f->movavg_index = 1;
        f->movavg_count = 1;
        return (uint16_t) f->exp_avg;
    }
    else
    {
        f->exp_avg = (0.9f * f->exp_avg) + (0.1f * (float) x);
    }

    float diff = fabsf(f->exp_avg - f->last_value);

    if (diff <= 25.0f)
    {
        f->movavg_buf[f->movavg_index] = f->exp_avg;
        f->movavg_index = (f->movavg_index + 1) % MOVAVG_LEN;
        if (f->movavg_count < MOVAVG_LEN)
            f->movavg_count++;

        float sum = 0.0f;
        for (uint8_t i = 0; i < f->movavg_count; i++)
            sum += f->movavg_buf[i];

        f->last_value = sum / f->movavg_count;
    }
    else
    {
        f->last_value = f->exp_avg;
    }

    return (uint16_t) f->last_value;
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

static inline uint8_t Detect_Step(uint16_t main_val, uint16_t extra_val,
                                  int16_t angle)
{
    const int16_t hysteresis = 30;

    /* podmínka: oba kanály kolem středu */
    if (abs((int16_t) main_val - MIDDLE_VALUE) > hysteresis && abs(
            (int16_t) extra_val - MIDDLE_VALUE)
                                                               > hysteresis)
    {

        step_state = STEP_ACTIVE;
        return 1;

    }
    else
    {

        step_state = STEP_IDLE;
    }

    return 0;
}
static void StepCounter_Update(rotation_dir_t dir, int16_t angle)
{
    for (uint8_t i = 0; i < STEP_ANGLES_COUNT; i++)
    {
        if (abs(angle - step_angles[i]) <= STEP_TOLERANCE)
        {
            // už jsme krok na tomhle úhlu počítali → přeskoč
            if (abs(last_step_angle) == abs(step_angles[i]))
                return;

            // nový krok → přičti/odečti
            if (dir == DIR_CW)
                step_count++;
            else if (dir == DIR_CCW)
                step_count--;

            last_step_angle = step_angles[i];  // zapamatuj si úhel
            return;
        }
    }

}

/* Capture handler -----------------------------------------------------------*/
static inline void Capture_HandleSample(uint16_t adc_sample[])
{
    uint16_t raw_main = adc_sample[MONITOR_CHANNEL];
    uint16_t raw_extra = adc_sample[EXTRA_CHANNEL];

    uint16_t filt_main = Filter_Step(&filter_main, raw_main);
    uint16_t filt_extra = Filter_Step(&filter_extra, raw_extra);

    float angle_rad = atan2f((float) raw_main - MIDDLE_VALUE,
                             (float) raw_extra - MIDDLE_VALUE);
    angle_deg = (int16_t) (angle_rad * (180.0f / M_PI));

    switch (capture_state)
    {
        case CAPT_WAIT_TRIGGER:
            if (abs(raw_main - TRIGGER_THRESHOLD) > 50)
            {
                capture_state = CAPT_CAPTURING;
                capture_count = 0;
                Filter_Reset(&filter_main);
                Filter_Reset(&filter_extra);

                capture_buffer[capture_count] = filt_main;
                capture_buffer_ch2[capture_count] = filt_extra;
                capture_buffer_angle[capture_count] = angle_deg;
                //capture_buffer_steps[capture_count] = step_count;
                capture_count++;
            }
            break;

        case CAPT_CAPTURING:
            if (capture_count < CAPTURE_SAMPLES)
            {

                sample_divider++;
                if (sample_divider >= 15)
                {
                    sample_divider = 0;

                    capture_buffer[capture_count] = filt_main;
                    capture_buffer_ch2[capture_count] = filt_extra;
                    capture_buffer_angle[capture_count] = angle_deg;
                    // capture_buffer_steps[capture_count] = step_count;

                    capture_count++;

                    if (capture_count >= CAPTURE_SAMPLES)
                    {
                        capture_state = CAPT_DONE;
                        capture_ready = 1;
                    }
                }
            }
            break;

        case CAPT_DONE:
        default:
            break;
    }
    if (Detect_Step(filt_main, filt_extra, angle_deg))
    {
        AngleBufferFIFO_Add(&angle_buffer, angle_deg);

        dir = Detect_RotationDirection(&angle_buffer);

        if (dir == DIR_CW || dir == DIR_CCW)
        {
            StepCounter_Update(dir, angle_deg);
        }
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == button_Pin)  // kontrola, jestli to bylo opravdu od tlačítka
    {
        capture_state = CAPT_WAIT_TRIGGER;

    }
}

/** @} */
