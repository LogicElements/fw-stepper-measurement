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
#define BUFFERING 1   // 1 = ukládání do bufferů aktivní, 0 = vypnuto
/* ADC channel mapping (based on order in adc_values[]) */
#define ADC_CHANNEL_IA2   0
#define ADC_CHANNEL_IA1   1
#define ADC_CHANNEL_IB2   2
#define ADC_CHANNEL_IB1   3

#define ADC_CHANNEL_COUNT 4
#define MIDDLE_VALUE      2045

#define CAPTURE_SAMPLES   5000
#define TRIGGER_THRESHOLD 2050
#define MONITOR_CHANNEL   1   // hlavní monitorovaný kanál
#define EXTRA_CHANNEL     0
#define ANGLE_BUFFER_LEN  20
#define STEP_TOLERANCE    4
#define STEP_ANGLES_COUNT (sizeof(step_angles)/sizeof(step_angles[0]))
#define VOLTAGE_BUF_LEN 100

/* Private typedefs ----------------------------------------------------------*/
typedef enum
{
    CAPT_WAIT_TRIGGER = 0,
    CAPT_CAPTURING,
    CAPT_DONE
} capture_state_t;

/* --- struktura filtru --- */
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
typedef struct
{
    uint16_t bufU1[VOLTAGE_BUF_LEN];
    uint16_t bufU2[VOLTAGE_BUF_LEN];
    uint16_t count;
    uint8_t active;  // flag jestli právě sbíráme
} VoltageBuffer_t;

typedef enum
{
    STEP_IDLE = 0,
    STEP_ACTIVE
} step_state_t;

typedef enum
{
    DIR_NONE = 0,
    DIR_CW,
    DIR_CCW
} rotation_dir_t;

typedef struct
{
    int16_t buf[ANGLE_BUFFER_LEN];
    uint8_t count;
} AngleBufferFIFO_t;

/* --- struktura motoru --- */
typedef struct
{
    Filter_t I1_filter;       // proud fáze 1
    Filter_t I2_filter;       // proud fáze 2
    int16_t I1_last;         // poslední vyfiltrovaný proud fáze 1
    int16_t I2_last;
    int16_t U1_last;   // napětí fáze 1 (raw)
    int16_t U2_last;   // poslední vyfiltrovaný proud fáze 2
    int16_t angle_deg;       // aktuální úhel motoru
    rotation_dir_t dir;       // směr otáčení
    int16_t step_count;      // počet kroků
    int16_t last_step_angle; // poslední krokový úhel
} MotorProbe_t;

/* Private variables ---------------------------------------------------------*/
#if (BUFFERING == 1)
static volatile capture_state_t capture_state = CAPT_WAIT_TRIGGER;
static volatile uint32_t capture_count = 0;
static volatile uint16_t capture_buffer[CAPTURE_SAMPLES];
static volatile uint16_t capture_buffer_ch2[CAPTURE_SAMPLES];
static volatile uint16_t Ucapture_buffer[CAPTURE_SAMPLES];
static volatile uint16_t Ucapture_buffer_ch2[CAPTURE_SAMPLES];
static float volatile capture_buffer_angle[CAPTURE_SAMPLES];
static volatile int32_t capture_buffer_steps[CAPTURE_SAMPLES];
static volatile uint8_t capture_ready = 0;
#endif

/* Dva motory (A, B) */
static MotorProbe_t motorA;
static MotorProbe_t motorB;
static VoltageBuffer_t voltageBuf;

/* FIFO na úhly (společný) */
static AngleBufferFIFO_t angle_buffer;

static step_state_t step_state = STEP_IDLE;
extern ADC_HandleTypeDef hadc1;

uint16_t adc_values[ADC_CHANNEL_COUNT];
volatile uint8_t adc_ready = 1;
uint16_t adc_ready_counter = 0;
static uint16_t sample_divider = 0;

static const int16_t step_angles[] =
{ 178, 90, 0, -90, -178 };

/* Private prototypes --------------------------------------------------------*/
void Probe_InitHAL(void);
static inline void Capture_HandleSample(uint16_t adc_sample[]);
static inline void Filter_Reset(Filter_t *f);
static inline uint16_t Filter_Step(Filter_t *f, uint16_t x);
static inline void Motor_HandleSample(MotorProbe_t *m, uint16_t raw_I1,
                                      uint16_t raw_I2, uint16_t raw_U1,
                                      uint16_t raw_U2);
static void Motor_StepDetectionAndUpdate(MotorProbe_t *m,uint16_t *adc_sample);
static void StepCounter_Update(MotorProbe_t *m);

/* Public functions ----------------------------------------------------------*/
void Probe_Init(void)
{
    Probe_InitHAL();

    Filter_Reset(&motorA.I1_filter);
    Filter_Reset(&motorA.I2_filter);
    motorA.I1_last = 0;
    motorA.I2_last = 0;
    motorA.angle_deg = 0;
    motorA.dir = DIR_NONE;
    motorA.step_count = 0;
    motorA.last_step_angle = 9999;

    Filter_Reset(&motorB.I1_filter);
    Filter_Reset(&motorB.I2_filter);
    motorB.I1_last = 0;
    motorB.I2_last = 0;
    motorB.angle_deg = 0;
    motorB.dir = DIR_NONE;
    motorB.step_count = 0;
    motorB.last_step_angle = 9999;
}

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

static inline void AngleBufferFIFO_Add(AngleBufferFIFO_t *ab, int16_t angle)
{
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

static rotation_dir_t Detect_RotationDirection(AngleBufferFIFO_t *ab)
{
    if (ab->count < ANGLE_BUFFER_LEN)
        return DIR_NONE;

    int16_t first = ab->buf[0];
    int16_t last = ab->buf[ab->count - 1];
    int16_t diff = last - first;

    if (diff > 0)
        return DIR_CW;
    if (diff < 0)
        return DIR_CCW;
    return DIR_NONE;
}

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

/* Motor sample handler ------------------------------------------------------*/
static inline void Motor_HandleSample(MotorProbe_t *m, uint16_t raw_I1,
                                      uint16_t raw_I2, uint16_t raw_U1,
                                      uint16_t raw_U2)
{
    m->I1_last = Filter_Step(&m->I1_filter, raw_I1);
    m->I2_last = Filter_Step(&m->I2_filter, raw_I2);
    m->U1_last = raw_U1;
    m->U2_last = raw_U2;

    float angle_rad = atan2f((float) m->I1_last - MIDDLE_VALUE,
                             (float) m->I2_last - MIDDLE_VALUE);
    m->angle_deg = (int16_t) (angle_rad * (180.0f / M_PI));
}

/* Step counter --------------------------------------------------------------*/
static void StepCounter_Update(MotorProbe_t *m)
{
    for (uint8_t i = 0; i < STEP_ANGLES_COUNT; i++)
    {
        if (abs(m->angle_deg - step_angles[i]) <= STEP_TOLERANCE)
        {
            if (abs(m->last_step_angle) == abs(step_angles[i]))
                return;

            if (m->dir == DIR_CW)
                m->step_count++;
            else if (m->dir == DIR_CCW)
                m->step_count--;

            m->last_step_angle = step_angles[i];
            return;
        }
    }
}
static inline uint8_t Check_CurrentZero(uint16_t *adc_sample)
{
    const int16_t tolerance = 20;   // okno kolem "nuly"
    if (abs(adc_sample[ADC_CHANNEL_IB1] - MIDDLE_VALUE) < tolerance || abs(
            adc_sample[ADC_CHANNEL_IB2] - MIDDLE_VALUE)
                                                                       < tolerance)
    {
        return 1; // jeden proud v nule
    }
    return 0;
}
static inline void VoltageBuffer_Reset(VoltageBuffer_t *vb)
{
    vb->count = 0;
    vb->active = 0;
}

static inline void VoltageBuffer_Add(VoltageBuffer_t *vb, uint16_t u1,
                                     uint16_t u2)
{
    if (vb->count < VOLTAGE_BUF_LEN)
    {
        vb->bufU1[vb->count] = u1;
        vb->bufU2[vb->count] = u2;
        vb->count++;
    }
}

static void VoltageBuffer_Average(VoltageBuffer_t *vb, float *avgU1,
                                  float *avgU2)
{
    if (vb->count == 0)
    {
        *avgU1 = 0;
        *avgU2 = 0;
        return;
    }

    uint32_t sumU1 = 0;
    uint32_t sumU2 = 0;

    for (uint16_t i = 0; i < vb->count; i++)
    {
        sumU1 += vb->bufU1[i];
        sumU2 += vb->bufU2[i];
    }

    *avgU1 = (float) sumU1 / vb->count;
    *avgU2 = (float) sumU2 / vb->count;
}

/* Motor step detection & update ---------------------------------------------*/
static void Motor_StepDetectionAndUpdate(MotorProbe_t *m,uint16_t *adc_sample)
{
    const int16_t hysteresis = 30;
    // říká jestli nejsou proudy nulový
    if (abs(m->I1_last - MIDDLE_VALUE) > hysteresis && abs(
            m->I2_last - MIDDLE_VALUE)
                                                       > hysteresis)
    {
        step_state = STEP_ACTIVE;

        AngleBufferFIFO_Add(&angle_buffer, m->angle_deg);
        m->dir = Detect_RotationDirection(&angle_buffer);

        if (m->dir == DIR_CW || m->dir == DIR_CCW)
        {
            StepCounter_Update(m);
        }

    }
    else
    {
        step_state = STEP_IDLE;
    }
    if (Check_CurrentZero(adc_sample))
    {
        if (!voltageBuf.active)
        {
            // začínáme sbírat
            VoltageBuffer_Reset(&voltageBuf);
            voltageBuf.active = 1;
        }

        // přidáme aktuální hodnoty do bufferu
        VoltageBuffer_Add(&voltageBuf, m->U1_last, m->U2_last);
    }
    else
    {
        if (voltageBuf.active)
        {

            float avgU1, avgU2;
            VoltageBuffer_Average(&voltageBuf, &avgU1, &avgU2);

            voltageBuf.active = 0;
        }
    }
}

/* ADC measurement start -----------------------------------------------------*/
void StartAdcMeasurement(void)
{
    if (adc_ready)
    {
        adc_ready = 0;
        if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_values, ADC_CHANNEL_COUNT) != HAL_OK)
        {
            // chyba při startu
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
    /* Motor A = IA1 + IA2 */
    Motor_HandleSample(&motorA, adc_sample[ADC_CHANNEL_IB1],
                       adc_sample[ADC_CHANNEL_IB2], adc_sample[ADC_CHANNEL_IA1],
                       adc_sample[ADC_CHANNEL_IA2]);
    /* Motor B = IB1 + IB2 */
    // Motor_HandleSample(&motorB,
    // adc_sample[ADC_CHANNEL_IB1],
    // adc_sample[ADC_CHANNEL_IB2]);
#if (BUFFERING == 1)
    switch (capture_state)
    {
        case CAPT_WAIT_TRIGGER:
            if ((abs(adc_sample[ADC_CHANNEL_IA1] - TRIGGER_THRESHOLD) > 50) || (abs(
                    adc_sample[ADC_CHANNEL_IB2] - TRIGGER_THRESHOLD)
                                                                                > 50))
            {
                capture_state = CAPT_CAPTURING;
                capture_count = 0;
                Filter_Reset(&motorA.I1_filter);
                Filter_Reset(&motorA.I2_filter);

                capture_buffer[capture_count] = adc_sample[ADC_CHANNEL_IB1];
                capture_buffer_ch2[capture_count] = adc_sample[ADC_CHANNEL_IB2];

                if (voltageBuf.active)
                {
                    Ucapture_buffer[capture_count] =
                            adc_sample[ADC_CHANNEL_IA1];
                    Ucapture_buffer_ch2[capture_count] =
                            adc_sample[ADC_CHANNEL_IA2];
                }
                else
                {
                    Ucapture_buffer[capture_count] = 2050;
                    Ucapture_buffer_ch2[capture_count] = 2050;
                }
                capture_buffer_angle[capture_count] = motorA.angle_deg;
                //capture_buffer_steps[capture_count] = motorA.step_count;
                capture_count++;
            }
            break;

        case CAPT_CAPTURING:
            if (capture_count < CAPTURE_SAMPLES)
            {
                sample_divider++;
                if (sample_divider >= 0)
                {
                    //sample_divider = 0;

                    capture_buffer[capture_count] = adc_sample[ADC_CHANNEL_IB1];//motorA.I1_last;
                    capture_buffer_ch2[capture_count] = adc_sample[ADC_CHANNEL_IB2];//motorA.I2_last;
                    if (voltageBuf.active)
                    {
                        Ucapture_buffer[capture_count] =
                                adc_sample[ADC_CHANNEL_IA1];
                        Ucapture_buffer_ch2[capture_count] =
                                adc_sample[ADC_CHANNEL_IA2];
                    }
                    else
                    {
                        Ucapture_buffer[capture_count] = 2050;
                        Ucapture_buffer_ch2[capture_count] = 2050;
                    }
                    capture_buffer_angle[capture_count] = motorA.angle_deg;
                    capture_buffer_steps[capture_count] = voltageBuf.active;

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
#endif

    /* Step detection (oba motory) */
    Motor_StepDetectionAndUpdate(&motorA,adc_sample);
    // Motor_StepDetectionAndUpdate(&motorB);
}
/* Callback functions --------------------------------------------------------*/
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
#if (BUFFERING == 1)
    if (GPIO_Pin == button_Pin)
    {
        capture_state = CAPT_WAIT_TRIGGER;
    }
#endif
}

/** @} */
