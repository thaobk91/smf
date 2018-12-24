
#ifndef __SS_HUMI_SOIL_H
#define __SS_HUMI_SOIL_H

#include "fsl_adc16.h"

//ADC Humi
#define macroSS_HUMI_ADC16_BASE						ADC1
#define macroSS_HUMI_ADC16_CHANNEL_GROUP 			0U
#define macroSS_HUMI_ADC16_USER_CHANNEL				7U


void vSS_Humi_Soil_Read( void );


#endif










