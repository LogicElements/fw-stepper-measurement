/*
 * RTD_Handle.c
 *
 *  Created on: 15. 2. 2024
 *      Author: evzen
 */


#include "RTD_Handle.h"
#include "RTD_lib.h"
#include "common.h"
#include "configuration.h"


void RTD_Handle(){


	if(conf.rtd.temp_calib == 0){

	switch(conf.rtd.mode){

	case 0:
		setResistance();
		break;
	case 1:
		setNTC();
		break;
	case 2:
		setPT();
		break;
	default:
		setResistance();
		break;
						}
	}


	if(conf.rtd.temp_calib == 1){

	switch(conf.rtd.mode){

	case 0:
		setResistance_TempCalb();
		break;
	case 1:
		setNTC_TempCalb();
		break;
	case 2:
		setPT_TempCalb();
		break;
	default:
		setResistance_TempCalb();
		break;

							}
   }

}



