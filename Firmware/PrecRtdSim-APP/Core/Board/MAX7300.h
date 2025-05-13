/*
 * MAX7300.h
 *
 *  Created on: 19. 10. 2023
 *      Author: evzen
 */

#include "common.h"

#ifndef INC_MAX7300_H_
#define INC_MAX7300_H_



#define MAX_I2C_ADDR  		0x44<<1
#define MAX_CONFIG 			0x04
#define MAX_PC1_CONF  		0x0B // P15, P14, P13, P12
#define MAX_PC2_CONF  		0x0C // P19, P18, P17, P16
#define MAX_PC3_CONF  		0x0D // P23, P22, P21, P20
#define MAX_PC4_CONF  		0x0E // P27, P26, P25, P24
#define MAX_PC5_CONF  		0x0F //r P31, P30, P29, P28
#define MAX_SET_12_19		0x4C
#define MAX_SET_20_27		0x54
#define MAX_SET_28_31		0x5C


void MAX_Init();
void MAX_conf();
void MAX_write_bin(uint32_t b);



#endif /* INC_MAX7300_H_ */
