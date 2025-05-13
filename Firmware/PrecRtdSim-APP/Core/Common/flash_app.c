/**
 * @file       flash_app.c
 * @brief      External flash application task implementation
 * @addtogroup grFlashApp
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "flash_app.h"

#include "configuration.h"
#include "system_msp.h"
#include "flash_conf.h"
//#include "calib.h"

/* Private defines -----------------------------------------------------------*/

#define FLASH_APP_NOTIFY_WRITE          1
#define FLASH_APP_NOTIFY_FACTORY        2

/* Private macros  -----------------------------------------------------------*/

/**
 * Configuration storage dead time [ms]
 */
#define CONFIG_DEAD_TIME      1500

/* Configuration store flag */
#define CONFIG_STORE          1

/* Factory storage flags */
#define FACTORY_CONFIG        1
#define FACTORY_RESET         2

/* Private typedefs ----------------------------------------------------------*/

/**
 * Definition of all private variables
 */
typedef struct
{
  uint8_t config;     ///< Configuration store flags
  uint8_t reset;      ///< Factory reset flags
  uint32_t tick;      ///< Dead time tick value
}FlashApp_Private_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables (except HAL handles)
 */
static FlashApp_Private_t flashApp;

/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/


Status_t FlashApp_Init(void)
{
  Status_t ret = STATUS_OK;

  /* Initialize flash configuration */
  FlashConf_Init();

  /* Read configuration - first attempt */
  ret = FlashConf_Load();

  if (ret == STATUS_OK)
  {
    /* We have configuration */
  }
  else
  {
    /* No success in reading configuration, we store the default values*/
    ret = FlashConf_Store();
  }

  /* FIXME: temporary disabled calibration */
//  Calib_Init();

  return ret;
}

Status_t FlashApp_Handle(void)
{
  Status_t ret = STATUS_OK;

  /* Configuration requested, set dead time */
  if (flashApp.config)
  {
    flashApp.config = 0;
    flashApp.tick = HAL_GetTick() + CONFIG_DEAD_TIME;
    flashApp.tick = (flashApp.tick == 0)?(UINT32_MAX):(flashApp.tick);
  }

  /* If factory reset was requested */
  if (flashApp.reset == FACTORY_CONFIG)
  {
    /* Load default factory settings */
    RegMap_RestoreFactoryValues();

    /* Request immediate storage and reset */
    flashApp.reset = FACTORY_RESET;
    flashApp.tick = HAL_GetTick();
    flashApp.tick = (flashApp.tick == 0)?(UINT32_MAX):(flashApp.tick);
  }

  /* Wait on tick expire */
  if (flashApp.tick != 0 && TICK_EXPIRED(flashApp.tick))
  {
    /* Clear tick to prevent tick expire false events */
    flashApp.tick = 0;

    /* Write non-volatile registers to external Flash */
    ret = FlashConf_Store();

    /* System reset */
    if (flashApp.reset == FACTORY_RESET)
    {
      HAL_Delay(1000);
      System_Reset();
    }
  }

  /* FIXME: temporary disabled calibration */
//  Calib_Write();

  return ret;
}

Status_t FlashApp_StartupFactoryReset(void)
{
  Status_t ret = STATUS_OK;

  /* Load default factory settings */
  RegMap_RestoreFactoryValues();

  /* Write non-volatile registers to external Flash */
  ret = FlashConf_Store();

  /* Short delay */
  HAL_Delay(100);

  return ret;
}


Status_t FlashApp_RequestConfStorage(void)
{
  Status_t ret = STATUS_OK;

  /* Do not store to flash in testing mode */
  if (!(conf.sys.status & STAT_BIT_TESTING))
  {
    flashApp.config = CONFIG_STORE;
  }

  return ret;
}


Status_t FlashApp_RequestFactorySettings(void)
{
  Status_t ret = STATUS_OK;

  flashApp.reset = FACTORY_CONFIG;

  return ret;
}

/* Private Functions ---------------------------------------------------------*/

/** @} */
