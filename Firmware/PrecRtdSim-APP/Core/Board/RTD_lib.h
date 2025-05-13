/*
 * RTD_lib.h
 *
 *  Created on: 16. 10. 2023
 *      Author: evzen
 */

#include "common.h"

#ifndef INC_RTD_LIB_H_
#define INC_RTD_LIB_H_


void RTD_Init();
void RTD_Handle();
void switch_position(uint32_t request);
void set_switch_rezistor(uint32_t request);
void rezistorArrayTemperature(float temperature, float temp_koef);
void switch_position_temp_calib(uint32_t request, float temperature, float temp_koef);
void set_switch_rezistor_temp_calib(uint32_t request, float temperature, float temp_koef);


void setResistance();
void setNTC();
void setPT();

void setResistance_TempCalb();
void setNTC_TempCalb();
void setPT_TempCalb();

#endif /* INC_RTD_LIB_H_ */
