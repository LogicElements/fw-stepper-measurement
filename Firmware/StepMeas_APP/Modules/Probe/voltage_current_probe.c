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
#define MOTOR 0   // 1 starý motor, 0 nový motor
/* ADC channel mapping (based on order in adc_values[]) */
#define ADC_CHANNEL_IA2   0
#define ADC_CHANNEL_IA1   1
#define ADC_CHANNEL_IB2   2
#define ADC_CHANNEL_IB1   3

#define ADC_CHANNEL_COUNT 4
#define MIDDLE_VALUE      2050

#define CAPTURE_SAMPLES   5000
#define TRIGGER_THRESHOLD 2045
#define MONITOR_CHANNEL   1   // hlavní monitorovaný kanál
#define EXTRA_CHANNEL     0
#define ANGLE_BUFFER_LEN  20
#define STEP_TOLERANCE    4
#define STEP_ANGLES_COUNT (sizeof(step_angles)/sizeof(step_angles[0]))
#define VOLTAGE_BUF_LEN 90
#define AVG_FIFO_LEN 7

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
    int32_t buf[AVG_FIFO_LEN];
    uint8_t count;
} AvgFifo_t;

typedef struct
{
    uint16_t bufU1[VOLTAGE_BUF_LEN];
    uint16_t bufU2[VOLTAGE_BUF_LEN];
    uint16_t countU1;
    uint16_t countU2;
    uint8_t activeU1;
    uint8_t activeU2;
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
    int16_t I1_last;
    int16_t I2_last;
    int16_t U1_last;
    int16_t U2_last;
    int16_t U1_last_filtered;
    int16_t U2_last_filtered;
    int32_t average_U1;
    int32_t average_U2;
    int16_t angle_deg;
    rotation_dir_t dir;
    int16_t step_count;
    int16_t last_step_angle;
    int32_t CurrentDirectionA;
    int32_t CurrentDirectionB;
} MotorProbe_t;

/* Private variables ---------------------------------------------------------*/
#if (BUFFERING == 1)
static volatile capture_state_t capture_state = CAPT_WAIT_TRIGGER;
static volatile uint32_t capture_count = 0;
static volatile uint16_t capture_buffer[CAPTURE_SAMPLES];
static volatile uint16_t capture_buffer_ch2[CAPTURE_SAMPLES];
static volatile uint16_t Ucapture_buffer[CAPTURE_SAMPLES];
static volatile uint16_t Ucapture_buffer_ch2[CAPTURE_SAMPLES];
//static  volatile int16_t capture_buffer_angle[CAPTURE_SAMPLES];
//static volatile int16_t capture_buffer_steps[CAPTURE_SAMPLES];
static volatile uint16_t capture_buffer_avg[CAPTURE_SAMPLES];
static volatile uint16_t capture_buffer_avg2[CAPTURE_SAMPLES];
static volatile uint8_t capture_ready = 0;
#endif

/* Dva motory (A, B) */
static MotorProbe_t motorA;
static MotorProbe_t motorB;
static VoltageBuffer_t voltageBuf;

/* FIFO na úhly (společný) */
static AngleBufferFIFO_t angle_buffer;
static AvgFifo_t avgU1_fifo =
{ .count = 0 };
static AvgFifo_t avgU2_fifo =
{ .count = 0 };

int stall = 0;
int stall1 = 0;
int stall2 = 0;
int pocty = 0;

static step_state_t step_state = STEP_IDLE;
extern ADC_HandleTypeDef hadc1;

static int32_t last_avgU1 = -1;
static int32_t last_avgU2 = -1;

uint16_t adc_values[ADC_CHANNEL_COUNT];
volatile uint8_t adc_ready = 1;
uint16_t adc_ready_counter = 0;
static uint16_t sample_divider = 0;
int16_t kroky = 0;

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
static void Motor_StepDetectionAndUpdate(MotorProbe_t *m, uint16_t *adc_sample);
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

static inline uint8_t Check_CurrentZero(uint16_t *adc_sample, uint8_t channel)
{
    const int16_t tolerance = 20;

    if (abs(adc_sample[channel] - MIDDLE_VALUE) < tolerance)
    {
        return 1;
    }

    if (channel == ADC_CHANNEL_IB1)
    {
        voltageBuf.activeU1 = 0;
    }
    else if (channel == ADC_CHANNEL_IB2)
    {
        voltageBuf.activeU2 = 0;
    }

    return 0;
}

static void VoltageBuffer_Reset(uint8_t channel)
{
    if (channel == ADC_CHANNEL_IB1)
    {
        voltageBuf.countU1 = 0;
        voltageBuf.activeU1 = 0;
    }
    else if (channel == ADC_CHANNEL_IB2)
    {
        voltageBuf.countU2 = 0;
        voltageBuf.activeU2 = 0;
    }
}

/* Nová funkce pro záznam při průchodu nulou -------------------------------*/
static void OnCurrentZeroCrossing(uint8_t channel, uint16_t voltageValue)
{
    if (channel == ADC_CHANNEL_IB1)
    {
        if (!voltageBuf.activeU1)
        {
            VoltageBuffer_Reset(channel);
            voltageBuf.activeU1 = 1;
        }
        if (voltageBuf.countU1 < VOLTAGE_BUF_LEN)
        {
            static float lastU1 = 0.0f;   // drží poslední vyhlazenou hodnotu

            // jednoduchý filtr: 50 % stará + 50 % nová
            if (voltageBuf.countU1 == 0)
            {
                lastU1 = (float) voltageValue; // první vzorek = rovnou
            }
            else
            {
                lastU1 = 0.5f * (float) voltageValue + 0.5f * lastU1;
            }

            uint16_t filteredU1 = voltageValue; //(uint16_t) lastU1;

            // --- omezení podle směru ---
            if (motorA.CurrentDirectionA == 1)
            {
                // kladný směr → rozsah 2050–3500
                if (filteredU1 < 2050)
                    filteredU1 = 2050;
                if (filteredU1 > 3500)
                    filteredU1 = 3500;
            }
            else
            {
                // záporný směr → rozsah 500–2050
                if (filteredU1 < 500)
                    filteredU1 = 500;
                if (filteredU1 > 2050)
                    filteredU1 = 2050;
            }

            voltageBuf.bufU1[voltageBuf.countU1++] = filteredU1;
            motorA.U1_last_filtered = filteredU1;
        }

    }
    else if (channel == ADC_CHANNEL_IB2)
    {
        if (!voltageBuf.activeU2)
        {
            VoltageBuffer_Reset(channel);
            voltageBuf.activeU2 = 1;
        }

        if (!voltageBuf.activeU2)
        {
            VoltageBuffer_Reset(channel);
            voltageBuf.activeU2 = 1;
        }

        if (voltageBuf.countU2 < VOLTAGE_BUF_LEN)
        {
            static float lastU2 = 0.0f;   // drží poslední vyhlazenou hodnotu

            // jednoduchý filtr: 50 % stará + 50 % nová
            if (voltageBuf.countU2 == 0)
            {
                lastU2 = (float) voltageValue; // první vzorek = rovnou
            }
            else
            {
                lastU2 = 0.5f * (float) voltageValue + 0.5f * lastU2;
            }

            uint16_t filteredU2 = voltageValue; //(uint16_t) lastU2;

            // --- omezení podle směru ---
            if (motorA.CurrentDirectionB == 1)
            {
                // kladný směr → rozsah 2050–3500
                if (filteredU2 < 2050)
                    filteredU2 = 2050;
                if (filteredU2 > 3500)
                    filteredU2 = 3500;
            }
            else
            {
                // záporný směr → rozsah 500–2050
                if (filteredU2 < 500)
                    filteredU2 = 500;
                if (filteredU2 > 2050)
                    filteredU2 = 2050;
            }

            voltageBuf.bufU2[voltageBuf.countU2++] = filteredU2;
            motorA.U2_last_filtered = filteredU2;
        }
    }
}

void ProcessVoltageBufferU1(void)
{
    uint16_t localCount = voltageBuf.countU1;
    if (localCount == 0)
        return;

    uint32_t sum = 0;
    for (uint16_t i = 0; i < localCount; i++)
        sum += voltageBuf.bufU1[i];

    uint16_t average = (uint16_t) (sum / localCount); // výsledek přetypován na uint16_t
    motorA.average_U1 = average;

    // bezpečný reset
    voltageBuf.countU1 = 0;
    voltageBuf.activeU1 = 0;
}

void ProcessVoltageBufferU2(void)
{
    static int count = 0;
    uint16_t localCount = voltageBuf.countU2;
    if (localCount == 0)
        return;

    uint32_t sum = 0;
    for (uint16_t i = 0; i < localCount; i++)
        sum += voltageBuf.bufU2[i];

    uint16_t average = (uint16_t) (sum / localCount); // výsledek přetypován na uint16_t
    motorA.average_U2 = average;

    if (count == 5)
    {
        count++;
    }

    // bezpečný reset
    voltageBuf.countU2 = 0;
    voltageBuf.activeU2 = 0;
    count++;
}

void VoltageBuffer_Task(void)
{
    static uint8_t lastActiveU1 = 0;
    static uint8_t lastActiveU2 = 0;

    if (lastActiveU1 == 1 && voltageBuf.activeU1 == 0 && voltageBuf.countU1 > 6)
        ProcessVoltageBufferU1();

    if (lastActiveU2 == 1 && voltageBuf.activeU2 == 0 && voltageBuf.countU2 > 6)
        ProcessVoltageBufferU2();

    lastActiveU1 = voltageBuf.activeU1;
    lastActiveU2 = voltageBuf.activeU2;
}
static int32_t AvgFifo_Add(AvgFifo_t *fifo, int32_t value)
{
    // přidávej jen hodnoty větší než 2050
    if (value <= 2050)
    {
        return -1; // nic se nepřidá, buffer se nemění
    }

    // pokud není buffer plný, přidáme hodnotu
    if (fifo->count < AVG_FIFO_LEN)
    {
        fifo->buf[fifo->count++] = value;
        return -1; // ještě není plno → nic nepočítáme
    }

    // FIFO plné → posunout hodnoty o jedno doleva
    for (uint8_t i = 0; i < AVG_FIFO_LEN - 1; i++)
    {
        fifo->buf[i] = fifo->buf[i + 1];
    }
    fifo->buf[AVG_FIFO_LEN - 1] = value;

    // spočítat průměr
    int64_t sum = 0;
    for (uint8_t i = 0; i < AVG_FIFO_LEN; i++)
    {
        sum += fifo->buf[i];
    }
    return (int32_t) (sum / AVG_FIFO_LEN);
}

static void Voltage_avg_process(void)
{
    int32_t avgU1 = motorA.average_U1;
    int32_t avgU2 = motorA.average_U2;

    // --- RESET pokud jsou oba proudy v nule ---
    const int16_t tol = 20; // tolerance kolem 2050
    if (abs(motorA.I1_last - MIDDLE_VALUE) < tol && abs(
            motorA.I2_last - MIDDLE_VALUE)
                                                    < tol)
    {
        avgU1_fifo.count = 0;
        avgU2_fifo.count = 0;
        return; // dál nepočítáme, dokud není motor aktivní
    }

    // --- zpracování U1 ---
    if (avgU1 != last_avgU1)
    {
        int32_t prumer = AvgFifo_Add(&avgU1_fifo, avgU1);
        if (prumer >= 0)
        {
            stall1 = (prumer > 2300) ? 0 : 1; // ladění rozmezí co už je doraz a co ne  2250 -3000 nejlepší výsledek
        }
        last_avgU1 = avgU1;
    }

    // --- zpracování U2 ---
    if (avgU2 != last_avgU2)
    {
        int32_t prumer = AvgFifo_Add(&avgU2_fifo, avgU2);
        if (prumer >= 0)
        {
            stall2 = (prumer > 2300) ? 0 : 1;
        }
        last_avgU2 = avgU2;
    }

    // --- kombinace ---
    stall = (stall1 == 1 && stall2 == 1) ? 1 : 0;
}

/* Detekce zastavení motoru na základě nízkého napětí -------------------------*/
static void Motor_StallVoltageCheck(MotorProbe_t *m, uint16_t *adc_sample)
{
    #define LOW_VOLTAGE_LIMIT 2000
    #define MAX_SAMPLES       500     // velikost FIFO
    #define AVG_THRESHOLD     50       // kolik vzorků potřebujeme pro výpočet

    static uint16_t lowVoltageBuf[MAX_SAMPLES];
    static uint16_t writeIndex = 0;
    static uint16_t sampleCount = 0;
    static uint64_t runningSum = 0;     // průběžný součet pro rychlý průměr

    uint16_t value = adc_sample[ADC_CHANNEL_IA1];

    // sbírej pouze pokud je napětí nízké
    if (value < LOW_VOLTAGE_LIMIT)
    {
        if (sampleCount < MAX_SAMPLES)
        {
            // buffer ještě není plný → jen přidej
            lowVoltageBuf[writeIndex] = value;
            runningSum += value;
            sampleCount++;
        }
        else
        {
            // buffer plný → odečti nejstarší a přepiš ji novou
            runningSum -= lowVoltageBuf[writeIndex];
            lowVoltageBuf[writeIndex] = value;
            runningSum += value;
        }

        // posuň index (kruhově)
        writeIndex = (writeIndex + 1) % MAX_SAMPLES;
    }

    // počítej průměr, jakmile máme dost vzorků
    if (sampleCount >= AVG_THRESHOLD)
    {
        uint16_t average = (uint16_t)(runningSum / sampleCount);

        pocty = average;                    // ladicí výstup
        stall = (average < 1800) ? 1 : 0;   // příklad vyhodnocení
    }
}
/* Motor step detection & update ---------------------------------------------*/
static void Motor_StepDetectionAndUpdate(MotorProbe_t *m, uint16_t *adc_sample)
{
    const int16_t hysteresis = 20;
    if (abs(m->I1_last - MIDDLE_VALUE) > hysteresis && abs(
            m->I2_last - MIDDLE_VALUE)
                                                       > hysteresis)
    {
        step_state = STEP_ACTIVE;

        AngleBufferFIFO_Add(&angle_buffer, m->angle_deg);
        m->dir = Detect_RotationDirection(&angle_buffer);

        if (m->dir == DIR_CW || m->dir == DIR_CCW)
            StepCounter_Update(m);

    }
    else
    {
        step_state = STEP_IDLE;

    }
#if (MOTOR == 1)

        if (Check_CurrentZero(adc_sample, ADC_CHANNEL_IB1))
        {
            OnCurrentZeroCrossing(ADC_CHANNEL_IB1, adc_sample[ADC_CHANNEL_IA2]);

        }

        if (Check_CurrentZero(adc_sample, ADC_CHANNEL_IB2))
        {
            OnCurrentZeroCrossing(ADC_CHANNEL_IB2, adc_sample[ADC_CHANNEL_IA1]);
        }
        VoltageBuffer_Task();
        Voltage_avg_process();

#endif
#if(MOTOR ==0)
    Motor_StallVoltageCheck(&motorA, adc_sample);
#endif

    kroky = motorA.step_count;

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

void DirCurrentDetection(void)
{
    static uint16_t lastValueA = 0;
    static uint16_t lastValueB = 0;
    static uint16_t counter = 0;
    counter++;
    if (counter > 50)
    {
        uint16_t rawA = adc_values[ADC_CHANNEL_IB1];
        uint16_t rawB = adc_values[ADC_CHANNEL_IB2];

        if (rawA > lastValueA)
            motorA.CurrentDirectionA = 1;
        else
            motorA.CurrentDirectionA = 0;
        lastValueA = rawA;

        if (rawB > lastValueB)
            motorA.CurrentDirectionB = 1;
        else
            motorA.CurrentDirectionB = 0;
        lastValueB = rawB;

        counter = 0;
    }
}

/* Capture handler -----------------------------------------------------------*/
static inline void Capture_HandleSample(uint16_t adc_sample[])
{
    /* Motor A = IA1 + IA2 */
    Motor_HandleSample(&motorA, adc_sample[ADC_CHANNEL_IB1],
                       adc_sample[ADC_CHANNEL_IB2], adc_sample[ADC_CHANNEL_IA1],
                       adc_sample[ADC_CHANNEL_IA2]);
    DirCurrentDetection();
    Motor_StepDetectionAndUpdate(&motorA, adc_sample);

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

                if (voltageBuf.activeU1 || voltageBuf.activeU2)
                {
                    Ucapture_buffer[capture_count] =
                            adc_sample[ADC_CHANNEL_IA1];
                    Ucapture_buffer_ch2[capture_count] =
                            adc_sample[ADC_CHANNEL_IA2];
                }
                else
                {
                    // Ucapture_buffer[capture_count] = 2050;
                    //  Ucapture_buffer_ch2[capture_count] = 2050;
                    Ucapture_buffer[capture_count] =
                            adc_sample[ADC_CHANNEL_IA1];
                    Ucapture_buffer_ch2[capture_count] =
                            adc_sample[ADC_CHANNEL_IA2];
                }
                //capture_buffer_angle[capture_count] = motorA.angle_deg;
                // capture_buffer_steps[capture_count] = motorA.step_count;

                capture_count++;
            }
            break;

        case CAPT_CAPTURING:
            if (capture_count < CAPTURE_SAMPLES)
            {
                sample_divider++;

                if (sample_divider >= 1000)
                {
                    if ((sample_divider % 1) == 0)   // jen každý pátý vzorek
                    {
                        capture_buffer[capture_count] =
                                adc_sample[ADC_CHANNEL_IB1];
                        capture_buffer_ch2[capture_count] =
                                adc_sample[ADC_CHANNEL_IB2];

                        if (voltageBuf.activeU1 || voltageBuf.activeU2)
                        {
                            Ucapture_buffer[capture_count] = motorA
                                    .U2_last_filtered;
                            Ucapture_buffer_ch2[capture_count] = motorA
                                    .U1_last_filtered;
                        }
                        else
                        {
                            // Ucapture_buffer[capture_count] = 2050;
                            //  Ucapture_buffer_ch2[capture_count] = 2050;
                            Ucapture_buffer[capture_count] =
                                    adc_sample[ADC_CHANNEL_IA1];
                            Ucapture_buffer_ch2[capture_count] =
                                    adc_sample[ADC_CHANNEL_IA2];
                        }

                        capture_buffer_avg[capture_count] = motorA.average_U2;
                        capture_buffer_avg2[capture_count] = motorA.average_U1;
                        //capture_buffer_angle[capture_count] = motorA.angle_deg;
                        //capture_buffer_steps[capture_count] = motorA.step_count;

                        capture_count++;

                        if (capture_count >= CAPTURE_SAMPLES)
                        {
                            capture_state = CAPT_DONE;
                            capture_ready = 1;
                        }
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
