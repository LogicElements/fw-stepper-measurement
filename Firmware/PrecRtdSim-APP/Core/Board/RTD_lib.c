/*
 * RTD_lib.c
 *
 *  Created on: 16. 10. 2023
 *      Author: Evzen Steif
 */


#include "RTD_lib.h"
#include "MAX7300.h"
#include "configuration.h"
#include "SHT20.h"


/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables
 */

typedef struct
{
	float switch_on_resistance;
	uint32_t switch_position[6];
	float resistor_array[6];

	float resistor_array_temp_calib[6];
	uint32_t switch_position_temp_calib[6];

	float resistance;

	float temp_c;
	float temp_koef;

}RTD_t;


static RTD_t rtd_app;



/**
* @brief: Initialization RTD values
*/
	void RTD_Init(){
		// values of resistor array - implicitly soldered to the board
		memcpy(rtd_app.resistor_array, (float[]){1.2, 9.53, 75, 590, 4700, 37400}, sizeof(rtd_app.resistor_array));

		// temperature coefficient of resistor array in ppm/K
		rtd_app.temp_koef = 100;

		// average analog switch on resistance of NX3L4051PW
		rtd_app.switch_on_resistance = 0.36;
	}



/* Auxiliary function */


	/**
	* @brief: Initialization RTD values
	*/

	float exponential(float x, int n) {
		float result = 1.0;
		float term = 1.0;

		for (int i = 1; i <= n; ++i) {
			term *= x / i;
			result += term;
		}
		return result;
	}


/* Basic functions of RTD */


		/**
		* @brief: Calculation of resistor array switch positions according to the request
		* @param request format: uint32_t from 10 to 290000
		*/
		void switch_position(uint32_t request)
		{

			uint8_t multiple = 0;
			int8_t i = 0;

			float resistance = rtd_app.switch_on_resistance * 6;

			for (i = 5; i >= 0; i--) {
				multiple = (((float)request - resistance) / rtd_app.resistor_array[i]);

				if (multiple >= 7) multiple = 7;

				resistance = resistance + multiple * rtd_app.resistor_array[i];
				rtd_app.switch_position[i] = multiple;

				if (resistance >= request) break;
			}
		}


		/**
		* @brief: Set the electrical resistance at the output of the RTD emulator according to the request
		* @param request format: uint32_t from 10 to 290000
		*/

		void set_switch_rezistor(uint32_t request){

			int8_t i = 0;

			switch_position(request);

			uint32_t cmd = 0x00;

			  for (i = 0; i < 6; i++) {
				  cmd = cmd | rtd_app.switch_position[i] << i * 3;
			  }

			  MAX_write_bin(cmd);
		}


/* Basic functions of RTD with temperature correction */


		/**
		* @brief: Calculation of resistor array with temperature dependence
		* @param 		* temperature format: float - in celsius
		                * temp koef format: float   - in ppm/K
		*/

	void rezistorArrayTemperature(float temperature, float temp_koef) {

		int8_t i = 0;

		for (i = 0; i < 6; i++) {
			rtd_app.resistor_array_temp_calib[i] = rtd_app.resistor_array[i] * (1 + (temperature - 20.0) * temp_koef / 1000000);
		}
	}


	/**
	* @brief: Calculation of resistor array switch positions with temperature dependence
	* @param 		* temperature format: float - in celsius
	                * temp koef format: float   - in ppm/K
	                * request format: uint32_t from 10 to 290000
	*/

	void switch_position_temp_calib(uint32_t request, float temperature, float temp_koef)
	{

		uint8_t multiple = 0;
		int8_t i = 0;

		rezistorArrayTemperature(temperature, temp_koef);

		float resistance = rtd_app.switch_on_resistance * 6;

		for (i = 5; i >= 0; i--) {
			multiple = (((float)request - resistance) / rtd_app.resistor_array_temp_calib[i]);

			if (multiple >= 7) multiple = 7;

			resistance = resistance + multiple * rtd_app.resistor_array_temp_calib[i];
			rtd_app.switch_position_temp_calib[i] = multiple;

			if (resistance >= request) break;
		}
	}



	/**
	* @brief: Set the electrical resistance at the output of the RTD emulator with temperature calibration
	* @param 		* temperature format: float - in celsius
	                * temp koef format: float   - in ppm/K
	                * request format: uint32_t from 10 to 290000
	*/

		void set_switch_rezistor_temp_calib(uint32_t request, float temperature, float temp_koef){

			int8_t i = 0;

			switch_position_temp_calib(request, temperature, temp_koef);

			uint32_t cmd = 0x00;

			  for (i = 0; i < 6; i++) {
				  cmd = cmd | rtd_app.switch_position_temp_calib[i] << i * 3;
			  }

			  MAX_write_bin(cmd);
		}


/* Handle functions */


	void setResistance(){

		if (conf.rtd.resistance >= 10 && conf.rtd.resistance <=290000)
			set_switch_rezistor(conf.rtd.resistance);
	}

	void setNTC (){
		float val = conf.rtd.ntc_beta*(1.0/(conf.rtd.temperature+273.15) - 1.0/(25.0+273.15));
		rtd_app.resistance = conf.rtd.ntc_stock_res*exponential(val, 20);

		if (conf.rtd.temperature >= -30 && conf.rtd.temperature<=200)
			set_switch_rezistor(rtd_app.resistance);
	}

	void setPT(){
		float A = 3.91e-3;
		rtd_app.resistance =  conf.rtd.pt_stock_res*(1+A*conf.rtd.temperature);

		if (conf.rtd.temperature >= -30 && conf.rtd.temperature<=200)
			set_switch_rezistor(rtd_app.resistance);
	}


	void setResistance_TempCalb(){
		rtd_app.resistance = conf.rtd.resistance;
		rtd_app.temp_c = SHT20_GetTemperature();

		if (conf.rtd.resistance >= 10 && conf.rtd.resistance <=290000)
			set_switch_rezistor_temp_calib(rtd_app.resistance, rtd_app.temp_c, rtd_app.temp_koef);
	}

	void setNTC_TempCalb (){
		float val = conf.rtd.ntc_beta*(1.0/(conf.rtd.temperature+273.15) - 1.0/(25.0+273.15));
		rtd_app.resistance = conf.rtd.ntc_stock_res*exponential(val, 20);
		rtd_app.temp_c = SHT20_GetTemperature();

		if (conf.rtd.temperature >= -30 && conf.rtd.temperature<=200)
			set_switch_rezistor_temp_calib(rtd_app.resistance, rtd_app.temp_c, rtd_app.temp_koef);
	}

	void setPT_TempCalb(){
		float A = 3.91e-3;
		rtd_app.resistance =  conf.rtd.pt_stock_res*(1+A*conf.rtd.temperature);
		rtd_app.temp_c = SHT20_GetTemperature();

		if (conf.rtd.temperature >= -30 && conf.rtd.temperature<=200)
			set_switch_rezistor_temp_calib(rtd_app.resistance, rtd_app.temp_c, rtd_app.temp_koef);
	}









