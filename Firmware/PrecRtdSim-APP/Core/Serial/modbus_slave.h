/**
  * @file       modbus_slave.h
  * @version    $(APP_VERSION)
  * @date       $(RELEASE_DATE)
  * @brief      file_brief
  * @author
  *
  * @copyright  Logic Elements Copyright
  *
  * @defgroup gr group_name
  * @{
  * @brief group_brief
  *
  * This module contains
  *
  * @par Main features:
  *
  * @par Example
  * @code
  *
  * @endcode
  */
#ifndef MODBUS_SLAVE_H_
#define MODBUS_SLAVE_H_

/* Includes ------------------------------------------------------------------*/

#include "common.h"


/* Definitions----------------------------------------------------------------*/

#define MODBUS_CLK_ENABLE()       do {__HAL_RCC_USART2_CLK_ENABLE();\
                                  } while(0)

#define MODBUS_TIMEOUT            10000

#define MODBUS_UART_INSTANCE      USART2              ///< MODBUS UART Instance
#define MODBUS_UART_IRQ_NUMBER    USART2_IRQn
#define MODBUS_UART_IRQ_HANDLER   USART2_IRQHandler
#define MODBUS_UART_ALTERNATE     GPIO_AF1_USART2     ///< MODBUS UART  Alternate function

#define MODBUS_UART_RX_PORT       GPIOA               ///< MODBUS UART Rx Pin
#define MODBUS_UART_RX_PIN        GPIO_PIN_3         ///< MODBUS UART Rx Port

#define MODBUS_UART_TX_PORT       GPIOA               ///< MODBUS UART Tx Pin
#define MODBUS_UART_TX_PIN        GPIO_PIN_2         ///< MODBUS UART Tx Port

#define MODBUS_UART_DE_PORT       GPIOA               ///< MODBUS UART DE Port
#define MODBUS_UART_DE_PIN        GPIO_PIN_1         ///< MODBUS UART DE Pin

#define MODBUS_TIM_INSTANCE                 TIM3
#define MODBUS_TIM_CLK_ENABLE               __HAL_RCC_TIM3_CLK_ENABLE
#define MODBUS_TIM_IRQ_NUMBER               TIM3_IRQn
#define MODBUS_TIM_IRQ_HANDLER              TIM3_IRQHandler

/* Typedefs-------------------------------------------------------------------*/
/* Functions -----------------------------------------------------------------*/

/**
 * Initialize Modbus module.
 *
 * @param address - modbus device address
 * @return Status - standard status code (0 - OK)
 */
Status_t MbSlave_Init(void);

/**
 * Modbus packet handler.
 *
 * @return Status - standard status code (0 - OK)
 */
Status_t MbSlave_Handle(void);

/**
 * Settings of modbus has changed, reinit UART
 *
 * @return Status
 */
Status_t MbSlave_SettingsChanged(void);

/**
 * Terminate current transfer and start waiting for next packet header.
 *
 * @return Status - standard status code (0 - OK)
 */
Status_t MbSlave_BusReset(void);

/**
 * UART receive callback
 *
 * Called when complete header or payload is received.
 */
void MbSlave_RxCpltCallback(UART_HandleTypeDef *huart);

/**
 * UART transmit callback
 *
 * @param huart UART handle
 */
void MbSlave_TxCpltCallback(UART_HandleTypeDef *huart);

/**
 * Error callback
 *
 * @param huart UART handle
 */
void MbSlave_ErrorCallback(UART_HandleTypeDef *huart);

/**
 * Call this function to update modbus slave address
 * @return STATUS_OK
 */
Status_t MbSlave_UpdateSlaveAddress(void);

#endif /* MODBUS_SLAVE_H_ */
/** @} */
