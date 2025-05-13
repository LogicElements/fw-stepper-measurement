/**
 * @file       flash_fs.c
 * @brief      Flash memory file system implementation
 * @addtogroup grFlashFs
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "flash_conf.h"
#include "system_msp.h"
#include "configuration.h"


/* Private defines -----------------------------------------------------------*/
/**
 * Definition of empty value in external flash
 */
#define FLASH_EMPTY_VALUE         0xFFFFFFFF

/**
 * Address of the first sector that is used for configuration storage (MUST be multiple of sector)
 */
#define FLASH_CONF_ADDR_START    ((uint32_t)CONF_C_CONFIGURATION_OFFSET)


/**
 * Address of the last sector plus one (MUST be multiple of sector)
 */
#define FLASH_CONF_ADDR_END       ((uint32_t)CONF_C_CONFIGURATION_OFFSET + (uint32_t)CONF_C_CONFIGURATION_SIZE)

/**
 * Size of memory block that we can erase at once
 */
#define FLASH_MEM_BLOCK_SIZE          2048

/**
 * Alignment in memory where new entry can start at
 */
#define FLASH_MEM_PAGE_SIZE           16


/* Private macros  -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/

/**
 * Definition of all private variables
 */
typedef struct
{
  uint32_t configAddr;
  uint32_t backupAddr;
  FlashConf_Entry_t data;
  FlashConf_Header_t tempHead;
  FlashConf_Header_t header;
  uint32_t invalidAddress;
}FlashFs_Private_t;

/* Private constants ---------------------------------------------------------*/

/**
 * Key value for the start and end addresses of configuration
 */
static const uint32_t FLASH_KEY = 0xdeadbeef;

/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables (except HAL handles)
 */
static FlashFs_Private_t flCo;

/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

Status_t FlashConf_FindEntry(void);

Status_t FlashConf_Read(uint32_t address);

Status_t FlashConf_Write(void);

Status_t FlashConf_EraseBlock(uint32_t address);

/* Functions -----------------------------------------------------------------*/

Status_t FlashConf_Init(void)
{
  Status_t ret = STATUS_OK;

  return ret;
}


Status_t FlashConf_Load(void)
{
  Status_t ret = STATUS_OK;

  /* Read headers */
  ret = FlashConf_FindEntry();

  if (ret == STATUS_OK)
  {
    /* Read main configuration */
    ret = FlashConf_Read(flCo.configAddr);

    if (ret == STATUS_ERROR)
    {
      /* Read backup configuration */
      ret = FlashConf_Read(flCo.backupAddr);
    }

    if (ret == STATUS_OK)
    {
      /* Configuration have been read successfully */
      /* Parse and apply config data */
      Config_ReadStream(flCo.data.data, flCo.data.header.completeSize - sizeof(FlashConf_Header_t) - sizeof(FLASH_KEY));
    }
  }
  else
  {
    /* No configuration found */
    ret = STATUS_ERROR;
  }

  conf.dbg.writes_conf = flCo.data.header.writes;

  return ret;
}

Status_t FlashConf_Store(void)
{
  Status_t ret = STATUS_OK;
  uint32_t length = 0;

  /* Create config data to write*/
  ret = Config_FillStream(flCo.data.data, &length, FLASH_CONF_SIZE - 4);

  memcpy(flCo.data.data + length, &FLASH_KEY, sizeof(FLASH_KEY));

  flCo.data.header.completeSize = length + sizeof(FLASH_KEY) + sizeof(FlashConf_Header_t);
  flCo.data.header.reserved = FLASH_KEY;

  /* Write config data into flash */
  ret += FlashConf_Write();

  conf.dbg.writes_conf = flCo.data.header.writes;

  if (ret == STATUS_OK)
  {
    CLEAR_BIT(conf.sys.status, STAT_BIT_CONFIG_FLASH);
  }
  else
  {
    SET_BIT(conf.sys.status, STAT_BIT_CONFIG_FLASH);
  }

  return ret;
}


Status_t FlashConf_Test(void)
{
  Status_t ret = STATUS_OK;
  uint32_t length;
  uint32_t nb;
  uint32_t writes;

  /* Compute length of single entry (ceiled up to pages) */
  length = CONF_REG_FLASH_LENGTH + sizeof(FLASH_KEY) + sizeof(FlashConf_Header_t);
  if (length & (FLASH_MEM_PAGE_SIZE - 1))
  {
    length &= ~(FLASH_MEM_PAGE_SIZE - 1);
    length += (FLASH_MEM_PAGE_SIZE);
  }

  /* Compute number of entries to write whole memory */
  nb = (FLASH_CONF_ADDR_END - FLASH_CONF_ADDR_START) / length;
  nb += 5;

  for (int i = 0; i < nb; i++)
  {
    /* Store back number of writes */
    writes = flCo.data.header.writes;

    /* Write one entry */
    FlashConf_Write();

    /* Find last entry again */
    ret += FlashConf_FindEntry();

    /* Read from backup address */
    ret += FlashConf_Read(flCo.backupAddr);

    /* Check writes value is the same as stored */
    if (writes != flCo.data.header.writes)
    {
      ret += 1;
    }

    /* Read from current address */
    ret += FlashConf_Read(flCo.configAddr);

    /* Check writes value is +1*/
    if ((writes + 1) != flCo.data.header.writes)
    {
      ret += 1;
    }
    System_ReloadWdg();
  }

  return ret;
}


/* Private Functions ---------------------------------------------------------*/


Status_t FlashConf_FindEntry(void)
{
  Status_t ret = STATUS_OK;
  uint32_t lastHeaderFound = 0;
  uint32_t address;

  /* Initial values */
  flCo.data.header.addressNext = FLASH_CONF_ADDR_START;
  flCo.data.header.writes = 1;
  flCo.data.header.completeSize = sizeof(FlashConf_Header_t);
  flCo.data.header.reserved = sizeof(FlashConf_Header_t);
  flCo.invalidAddress = -1;
  flCo.configAddr = FLASH_CONF_ADDR_START;
  flCo.backupAddr = FLASH_CONF_ADDR_START;

  address = FLASH_CONF_ADDR_START;

  /* Find first sector with non-empty values*/
  while ((address < FLASH_CONF_ADDR_END) && (lastHeaderFound == 0))
  {
    /* Read header into temp */
    memcpy(&flCo.tempHead, (uint8_t*)address, sizeof(FlashConf_Header_t));

    if (flCo.tempHead.addressNext == 0 && flCo.tempHead.completeSize == 0 && flCo.tempHead.reserved != FLASH_KEY)
    {
      /* We can erase sector of backup */
      ret = FlashConf_EraseBlock(address);

      /* Jump to next sector */
      address += FLASH_MEM_BLOCK_SIZE;
    }
    else if (flCo.tempHead.addressNext != FLASH_EMPTY_VALUE)
    {
      /* We reached an empty value, so the previous value is final */
      lastHeaderFound = 1;
    }
    else
    {
      /* Jump to next sector */
      address += FLASH_MEM_BLOCK_SIZE;
    }
  }

  /* Nothing found in flash */
  if (address == FLASH_CONF_ADDR_END)
  {
    ret = STATUS_ERROR;
  }
  else
  {
    ret = STATUS_OK;
    lastHeaderFound = 0;

    /* Go through chain list until last non-empty value is found */
    while ((address < FLASH_CONF_ADDR_END) && (lastHeaderFound == 0))
    {
      /* Read header into temp */
      memcpy((uint8_t *)&flCo.tempHead, (uint8_t*)address, sizeof(FlashConf_Header_t));

      if (flCo.tempHead.addressNext == FLASH_EMPTY_VALUE)
      {
        /* We reached an empty value, try to read next beginning of sector */

        /* This entry is the first */
        if (flCo.configAddr == FLASH_CONF_ADDR_START && flCo.backupAddr == FLASH_CONF_ADDR_START )
        {
          /* Try the last sector */
          address = FLASH_CONF_ADDR_END - FLASH_MEM_BLOCK_SIZE;
        }
        /* Jump to the beginning of sector */
        else if ((address % FLASH_MEM_BLOCK_SIZE) >= FLASH_MEM_BLOCK_SIZE / 2) // address & (FLASH_MEM_BLOCK_SIZE - 1) &&
        {
          address &= ~(FLASH_MEM_BLOCK_SIZE - 1);
          address += (FLASH_MEM_BLOCK_SIZE);

          /* Wrap around the config space */
          if (address == FLASH_CONF_ADDR_END)
          {
            address = FLASH_CONF_ADDR_START;
          }
        }

        /* Read header into temporary storage */
        memcpy((uint8_t *)&flCo.tempHead, (uint8_t*)address, sizeof(FlashConf_Header_t));
      }

      /* Still empty value */
      if (flCo.tempHead.addressNext == FLASH_EMPTY_VALUE)
      {
        /* We reached an empty value, so the previous value is final */
        lastHeaderFound = 1;
      }
      /* Check next address on configuration boundaries and page beginning*/
      else if (flCo.tempHead.addressNext > FLASH_CONF_ADDR_END || (flCo.tempHead.addressNext & (FLASH_MEM_PAGE_SIZE - 1)))
      {
        /* The next address is invalid */
        flCo.invalidAddress = address;

        /* Maybe the erase has failed, try to peek into next sector */
        address &= ~(FLASH_MEM_BLOCK_SIZE - 1);
        address += (FLASH_MEM_BLOCK_SIZE);
        /* Wrap around the config space */
        if (address == FLASH_CONF_ADDR_END)
        {
          address = FLASH_CONF_ADDR_START;
        }
      }
      else
      {
        /* Store temp as new valid value */
        memcpy(&flCo.header, &flCo.tempHead, sizeof(FlashConf_Header_t));

        /* Shift config pointers and find out if some sector is left useless */
        if ((flCo.backupAddr ^ flCo.configAddr) & ~(FLASH_MEM_BLOCK_SIZE - 1)
            && (flCo.configAddr - flCo.backupAddr < FLASH_MEM_BLOCK_SIZE))
        {
          /* We can erase sector of backup */
          ret = FlashConf_EraseBlock(flCo.backupAddr);
        }

        flCo.backupAddr = flCo.configAddr;
        flCo.configAddr = address;
        address = flCo.tempHead.addressNext;
      }
    }
  }

  /* If invalid address was found and backup entry is in different sector, we can erase corrupted sector */
  if (flCo.invalidAddress != -1 && ((flCo.backupAddr ^ flCo.invalidAddress) & ~(FLASH_MEM_BLOCK_SIZE - 1)) != 0)
  {
    /* Erase the corrupted sector */
    ret = FlashConf_EraseBlock(flCo.invalidAddress);
  }

  return ret;
}


Status_t FlashConf_EraseBlock(uint32_t address)
{
  Status_t ret = STATUS_OK;

  ret = System_FlashErase(address, address + 1);

  return ret;
}

Status_t FlashConf_Read(uint32_t address)
{
  Status_t ret = STATUS_OK;
  uint32_t lengthToRead;

  /* Read header */
  memcpy((uint8_t *)&flCo.data.header, (uint8_t*)address, sizeof(FlashConf_Header_t));

  lengthToRead = flCo.data.header.completeSize - sizeof(FlashConf_Header_t);
  SAT_UP(lengthToRead, FLASH_CONF_SIZE);

  /* Read the rest of configuration data */
  memcpy((uint8_t *)&flCo.data.data, (uint8_t*)address + sizeof(FlashConf_Header_t), lengthToRead);

  /* Consistency check at reserved field*/
  if (flCo.data.header.reserved != FLASH_KEY)
  {
    ret = STATUS_ERROR;
  }

  /* Check consistency at the end of stream */
  if (memcmp(flCo.data.data + flCo.data.header.completeSize - sizeof(FlashConf_Header_t) - sizeof(FLASH_KEY), &FLASH_KEY, sizeof(FLASH_KEY)) != 0)
  {
    ret = STATUS_ERROR;
  }

  return ret;
}


Status_t FlashConf_Write(void)
{
  Status_t ret = STATUS_OK;
  uint32_t address = flCo.data.header.addressNext;    // previous address next become current address
  uint32_t endAddress = (address & ~(FLASH_MEM_BLOCK_SIZE - 1)) + FLASH_MEM_BLOCK_SIZE; // end addess of current sector

  /* Increment the writes counter */
  flCo.data.header.writes += 1;

  /* Compute the next address */
  flCo.data.header.addressNext += flCo.data.header.completeSize;
  /* Ceil up to whole pages */
  if (flCo.data.header.addressNext & (FLASH_MEM_PAGE_SIZE - 1))
  {
    flCo.data.header.addressNext &= ~(FLASH_MEM_PAGE_SIZE - 1);
    flCo.data.header.addressNext += (FLASH_MEM_PAGE_SIZE);
  }
  
  if (flCo.data.header.addressNext == FLASH_CONF_ADDR_END)
  {
    flCo.data.header.addressNext = FLASH_CONF_ADDR_START;
  }
  else if (flCo.data.header.addressNext > endAddress)
  {
    /* Is this the last sector ? */
    if (endAddress == FLASH_CONF_ADDR_END)
    {
      /* Go to first sector */
      address = FLASH_CONF_ADDR_START ;
    }
    else
    {
      /* Go to next sector */
      address += FLASH_MEM_BLOCK_SIZE;
    }

    /* Reset the address pointer to the beginning of next sector */
    address = address & ~(FLASH_MEM_BLOCK_SIZE - 1);
    /* Increment Next address pointer */
    flCo.data.header.addressNext = address + flCo.data.header.completeSize;
    /* Ceil up to whole pages */
    if (flCo.data.header.addressNext & (FLASH_MEM_PAGE_SIZE - 1))
    {
      flCo.data.header.addressNext &= ~(FLASH_MEM_PAGE_SIZE - 1);
      flCo.data.header.addressNext += (FLASH_MEM_PAGE_SIZE);
    }
  }

  ret = System_FlashProgram(address, (uint8_t*)&flCo.data, ((flCo.data.header.completeSize + 3 ) >> 2)* 4);

  /* Shift config pointers and find out if some sector is left useless */
  if ((flCo.backupAddr ^ flCo.configAddr) & ~(FLASH_MEM_BLOCK_SIZE - 1))
  {
    /* We can erase sector of backup */
    FlashConf_EraseBlock(flCo.backupAddr);
  }

  flCo.backupAddr = flCo.configAddr;
  flCo.configAddr = address;

  return ret;
}

/** @} */
