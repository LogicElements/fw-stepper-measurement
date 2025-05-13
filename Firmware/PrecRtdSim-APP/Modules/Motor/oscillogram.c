/**
 * @file       oscillogram.c
 * @brief      file_brief
 * @addtogroup gr
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <Motor/oscillogram.h>
#include "usbd_cdc_if.h"
#include "configuration.h"
#include "com_proto.h"

/* Private defines -----------------------------------------------------------*/

#define BUFFER_OFFSET     10

/* Private macros  -----------------------------------------------------------*/

/* Private typedefs ----------------------------------------------------------*/

/**
 * Declaration of all private variables
 */
typedef struct
{
  uint8_t saving;
  uint8_t done;
  uint16_t buffer[2][OSC_BUFFER_LENGTH + BUFFER_OFFSET + 100];
  uint16_t ptr;
}Oscillogram_Private_t;

/* Private constants ---------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables (except HAL handles)
 */
static Oscillogram_Private_t osc;

/* Private variables ---------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

Status_t Oscil_Init(void)
{
  Status_t ret = STATUS_OK;

  return ret;
}

Status_t Oscil_Handle(void)
{
  Status_t ret = STATUS_OK;

  /* Done */
  if (osc.done != 0)
  {
    /* Is USB ready to send data? */
    if (CDC_TransmitReady() == STATUS_OK)
    {
      osc.done--;
      /* Send data */
      uint8_t * buf = (uint8_t*)osc.buffer[osc.done];
      ComProto_FillSeries(buf, 1, osc.done + 1, conf.sys.uptime, 1, osc.ptr);
      CDC_Transmit_FS(buf, ComProto_GetLength(buf));

      if (osc.done == 0)
      {
        /* Clear pointer */
        osc.ptr = 0;
      }
    }

    /* End of data transmit means end of capture for one-shot and auto trigger */
    if (osc.done == 0 &&
        (conf.osc.mode == OSC_ONE_SHOT || conf.osc.mode == OSC_AUTO_TRIGGER))
    {
      conf.osc.start = 0;
    }
  }

  return ret;
}

Status_t Oscil_NewData(uint16_t *data, uint16_t samples)
{
  Status_t ret = STATUS_OK;

  /* Search for auto trigger */
  if (conf.osc.mode == OSC_AUTO_TRIGGER && conf.osc.start == 0)
  {
    for (int i = 0; i < samples; i++)
    {
      if ((data[4 * i + 2 * conf.osc.source] > 2200 + 1000) ||
          (data[4 * i + 2 * conf.osc.source] < 2200 - 1000) ||
          (data[4 * i + 1 + 2 * conf.osc.source] > 2200 + 1000) ||
          (data[4 * i + 1 + 2 * conf.osc.source] < 2200 - 1000))
      {
        /* Start logging right away and break out */
        conf.osc.start = 1;
        break;
      }
    }
  }

  /* Copy data */
  if (conf.osc.mode == OSC_CONTINUOUS ||
      (conf.osc.mode == OSC_ONE_SHOT && conf.osc.start != 0) ||
      (conf.osc.mode == OSC_AUTO_TRIGGER && conf.osc.start != 0)
      )
  {
    /* Copy data into buffer */
    for (int i = 0; i < samples; i++)
    {
      osc.buffer[0][osc.ptr + i + BUFFER_OFFSET] = data[4 * i + 2 * conf.osc.source];
      osc.buffer[1][osc.ptr + i + BUFFER_OFFSET] = data[4 * i + 1 + 2 * conf.osc.source];
    }
    osc.ptr += samples;

    if (osc.ptr >= OSC_BUFFER_LENGTH)
    {
      osc.done = 2;
    }
  }

  return ret;
}

/* Private Functions ---------------------------------------------------------*/

/** @} */
