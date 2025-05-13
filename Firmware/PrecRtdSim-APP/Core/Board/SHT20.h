/*
 * SHT20.h
 *
 *  Created on: 24. 10. 2023
 *      Author: evzen
 */

#include "common.h"

#ifndef INC_SHT20_H_
#define INC_SHT20_H_


#define SHT20_I2C_ADDR			0x40
#define SHT20_READ_TEMP_HOLD	0xe3
#define	SHT20_READ_RH_HOLD		0xe5
#define SHT20_READ_TEMP_NOHOLD	0xf3
#define SHT20_READ_RH_NOHOLD	0xf5
#define	SHT20_WRITE_REG			0xe6
#define SHT20_READ_REG			0xe7
#define SHT20_SOFT_RESET		0xfe
#define SHT20_TIMEOUT			1000



typedef enum SHT20_Resolution {
	RES_14_12 = 0x00,
	RES_12_8 = 0x01,
	RES_13_10 = 0x80,
	RES_11_11 = 0x81,
} SHT20_Resolution;




void SHT20_softreset();
void SHT20_SetResolution(SHT20_Resolution res);
uint8_t SHT20_ReadUserReg(void);
float SHT20_GetTemperature();
float SHT20_GetRelativeHumidity();

#endif /* INC_SHT20_H_ */
