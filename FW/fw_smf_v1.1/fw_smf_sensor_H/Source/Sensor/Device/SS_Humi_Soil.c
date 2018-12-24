
#include "SS_Humi_Soil.h"
#include "main.h"

extern adc16_channel_config_t xADC16ChannelConfigStruct;


void vSS_Humi_Soil_Read( void )
{
	float fVol = 0;
	
	//Detect AC input
	xADC16ChannelConfigStruct.channelNumber = macroSS_HUMI_ADC16_USER_CHANNEL;
	ADC16_SetChannelConfig(macroSS_HUMI_ADC16_BASE, macroSS_HUMI_ADC16_CHANNEL_GROUP, &xADC16ChannelConfigStruct);
	while (0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(macroSS_HUMI_ADC16_BASE, macroSS_HUMI_ADC16_CHANNEL_GROUP)))
	{
	}
	fVol = (float)ADC16_GetChannelConversionValue(macroSS_HUMI_ADC16_BASE, macroSS_HUMI_ADC16_CHANNEL_GROUP);// * 3.3 / 65535;
	APP_DEBUG("\r\n--- SS_Humi_Soil: Humidity Voltage = %f [V]\r\n", fVol);
}