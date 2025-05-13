/**
 * @file       mb_upgrade.c
 * @brief      Modbus RTU upgrade implementation
 * @addtogroup grMbUpgrade
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "mb_upgrade.h"
#include "system_msp.h"

/* Private defines -----------------------------------------------------------*/
/* Private macros  -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/

/**
 * Declaration of all private variables
 */
typedef struct __packed
{
	uint16_t type;      ///< Type of binary (mcu, graphics, peripheral, etc. - Application specific)
	uint16_t mode;      ///< Mode of operation (0 - erase on the fly, 1 - erase at start, 2 - apply)
	uint32_t size;      ///< Size of binary in bytes
	uint16_t page_size; ///< Size of current page in bytes
	uint32_t offset;    ///< Offset of current page programming
	uint8_t data[MB_UPGR_PAGE_SIZE];  ///< Current page data
	uint16_t status;    ///< Status of writing (0 - busy, 1 - ready, 2 - done and verified, 3 - done with error)
	uint16_t writeDone; ///< Request to write data (1 - write request)
	uint8_t startClb;
}MbUpgrade_Private_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables (except HAL handles)
 */
static MbUpgrade_Private_t mbu;

/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/

Status_t MbUpgr_Init(void)
{
  Status_t ret = STATUS_OK;

  /* Set status ready */
  mbu.status = MB_UPGR_STATUS_BUSY;
  mbu.writeDone = 0;

  return ret;
}


Status_t MbUpgr_Handle(void)
{
  Status_t ret = STATUS_OK;

  /* First 4 registers were written? */
  if (mbu.startClb != 0)
  {
    mbu.startClb = 0;
    /* Start of programming procedure */
    ret = MbUpgr_HeaderClb(mbu.type, mbu.mode, mbu.size);
    if (mbu.writeDone == 0)
    {
      if (ret == STATUS_OK)
      {
        mbu.status = MB_UPGR_STATUS_READY;
      }
      else
      {
        mbu.status = MB_UPGR_STATUS_DONE_ERROR;
      }
    }
  }
  /* Write done flag is set */
  if (mbu.writeDone != 0)
  {
    /* Call function that writes data into flash */
    ret = MbUpgr_WritePageClb(mbu.offset, mbu.data, mbu.page_size);
    if (ret == STATUS_OK)
    {
      mbu.status = MB_UPGR_STATUS_READY;
    }
    else
    {
      mbu.status = MB_UPGR_STATUS_DONE_ERROR;
    }
    /* Did we receive all data? */
    if (mbu.size - mbu.offset <= mbu.page_size)
    {
      /* The whole binary has been received, callback */
      ret = MbUpgr_WriteDoneClb(mbu.size);
      if (ret == STATUS_OK)
      {
        mbu.status = MB_UPGR_STATUS_DONE_OK;
      }
      else
      {
        mbu.status = MB_UPGR_STATUS_DONE_ERROR;
      }
    }
    /* Reset flags */
    mbu.writeDone = 0;
  }

  return ret;
}


Status_t MbUpgr_WriteRegisters(uint16_t offset, uint16_t length, uint8_t* data)
{
  Status_t ret = STATUS_OK;
  uint8_t *storage = (uint8_t*)&mbu + (offset - MB_UPGR_BASE_ADDRESS) * 2;

  /* Copy and byte reverse */
  for (int i = 0; i < length; i++)
  {
    storage[i * 2 + 1] = data[i * 2 + 0];
    storage[i * 2 + 0] = data[i * 2 + 1];
  }

  /* Write into starting registers */
  if ((offset - MB_UPGR_BASE_ADDRESS) < 4)
  {
    mbu.startClb = 1;
  }

  return ret;
}


Status_t MbUpgr_ReadRegisters(uint16_t offset, uint16_t length, uint8_t* data)
{
  Status_t ret = STATUS_OK;
  uint8_t *storage = (uint8_t*)&mbu + (offset - MB_UPGR_BASE_ADDRESS) * 2;

  /* Copy and byte reverse */
  for (int i = 0; i < length; i++)
  {
    data[i * 2 + 1] = storage[i * 2 + 0];
    data[i * 2 + 0] = storage[i * 2 + 1];
  }

  return ret;
}

uint16_t MbUpgr_GetType(void)
{
  return mbu.type;
}

uint16_t MbUpgr_GetMode(void)
{
  return mbu.mode;
}

/* Private Functions ---------------------------------------------------------*/

__weak Status_t MbUpgr_HeaderClb(uint16_t type, uint16_t mode, uint32_t size)
{
  Status_t ret = STATUS_OK;

  switch (mode)
  {
    case MB_UPGR_MODE_ERASE_AT_START:
//      ret = System_FlashErase((uint32_t)CONF_C_APP_BUFFER_OFFSET, (uint32_t)CONF_C_APP_BUFFER_OFFSET + size);
      break;
    case MB_UPGR_MODE_APPLY:
//      EventMngr_Event(ATT_SYS_RESTART_REQ, NULL);
      break;
    default:
      break;
  }

  return ret;
}


__weak Status_t MbUpgr_WritePageClb(uint32_t offset, uint8_t *data, uint16_t length)
{
  Status_t ret = STATUS_OK;

//  ret = System_FlashProgram((uint32_t)CONF_C_APP_BUFFER_OFFSET + offset, data, length);

  return ret;
}

__weak Status_t MbUpgr_WriteDoneClb(uint32_t length)
{
  Status_t ret = STATUS_OK;

  /* The very last packet, verify image */
//  ret = System_VerifyImage((uint32_t*) CONF_C_APP_BUFFER_OFFSET);

  return ret;
}

/** @} */
