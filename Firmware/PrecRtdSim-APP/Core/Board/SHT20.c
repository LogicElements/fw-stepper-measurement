/*
 * SHT20.c
 *
 *  Created on: 24. 10. 2023
 *      Author: Evzen Steif
 */


#include "SHT20.h"
#include "i2c.h"
#include "common.h"


/**
* @brief: Performs a soft reset of SHT20
*/
void SHT20_softreset(){
	uint8_t cmd = SHT20_SOFT_RESET;
	HAL_I2C_Master_Transmit(&hi2c2, SHT20_I2C_ADDR << 1, &cmd, 1, SHT20_TIMEOUT);
}

/**
* @brief: Measuring current temperature
* @param: return temp_c: float, in Celsius
*/


float SHT20_GetTemperature() {

	uint8_t val[3] = { 0 };
	HAL_StatusTypeDef ret;
	int16_t raw = 0;
	uint8_t cmd = SHT20_READ_TEMP_NOHOLD;
	float temp_c;

  	HAL_I2C_Master_Transmit(&hi2c2, SHT20_I2C_ADDR << 1, &cmd, 1, HAL_MAX_DELAY);

  	do {
    ret = HAL_I2C_Master_Receive(&hi2c2, SHT20_I2C_ADDR << 1, val, 3, HAL_MAX_DELAY);
  	}
    while (ret != HAL_OK);

    raw = (val[0] << 8) | val[1];
    temp_c =  -46.85 + 175.72*((float)raw/65536.0);
    return temp_c;
}


/**
* @brief: Measuring current relative humidity
* @param: return temp_c: float, in %RH
*/

float SHT20_GetRelativeHumidity() {

	uint8_t val[3] = { 0 };
	int16_t raw = 0;
	HAL_StatusTypeDef ret;
	uint8_t cmd = SHT20_READ_RH_NOHOLD;
    HAL_I2C_Master_Transmit(&hi2c2, SHT20_I2C_ADDR << 1, &cmd, 1, SHT20_TIMEOUT);


    do {
    ret = HAL_I2C_Master_Receive(&hi2c2, SHT20_I2C_ADDR << 1, val, 3, SHT20_TIMEOUT);
    }
    while (ret != HAL_OK);


    raw = (val[0] << 8) | val[1];
    return -6 + 125.00 * ((float)raw / 65536.0);
}


/**
 * @brief Gets the value stored in user register.
 * @return 8-bit value stored in user register, 0 to 255.
 */

uint8_t SHT20_ReadUserReg(void) {
	uint8_t val;
	uint8_t cmd = SHT20_READ_REG;
	HAL_I2C_Master_Transmit(&hi2c2, SHT20_I2C_ADDR << 1, &cmd, 1, SHT20_TIMEOUT);
	HAL_I2C_Master_Receive(&hi2c2, SHT20_I2C_ADDR << 1, &val, 1, SHT20_TIMEOUT);
	return val;
}


/**
 * @brief Sets the measurement resolution.
 * @param res Enum resolution.
 * @note Available resolutions: RES_14_12, RES_12_8, RES_13_10, RES_11_11.
 * @note RES_14_12 = 14-bit temperature and 12-bit RH resolution, etc.
 */

void SHT20_SetResolution(SHT20_Resolution res) {
	uint8_t val = SHT20_ReadUserReg();
	val = (val & 0x7e) | res;
	uint8_t temp[2] = { SHT20_WRITE_REG, val };
	HAL_I2C_Master_Transmit(&hi2c2, SHT20_I2C_ADDR << 1, temp, 2, SHT20_TIMEOUT);
}





