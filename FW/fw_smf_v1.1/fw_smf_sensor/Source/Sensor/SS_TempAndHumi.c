
#include "SS_TempAndHumi.h"

#ifdef USE_SHT1X
	#include "SHT1x.h"
#endif


#ifdef USE_SHT1X
	SHT1x_IO _SHT1x_IO_Air_Index = 
	{
		.DATA_GPIOBase	= macroSHT_AIR_SDA_GPIO,
		.DATA_Pin		= macroSHT_AIR_SDA_PIN,
		
		.CLK_GPIOBase	= macroSHT_AIR_SCL_GPIO,
		.CLK_Pin		= macroSHT_AIR_SCL_PIN,
	};

	SHT1x_IO _SHT1x_IO_Soil_Index = 
	{
		.DATA_GPIOBase	= macroSHT_SOIL_SDA_GPIO,
		.DATA_Pin		= macroSHT_SOIL_SDA_PIN,
		
		.CLK_GPIOBase	= macroSHT_SOIL_SCL_GPIO,
		.CLK_Pin		= macroSHT_SOIL_SCL_PIN,
	};
#endif



void vSS_TempAndHumi_Air_Index( float *fTemp, float *fHumi )
{
#ifdef USE_SHT1X
	vSHT11_Read( fTemp, fHumi, &_SHT1x_IO_Air_Index );
#endif
}




void vSS_TempAndHumi_Soil_Index( float *fTemp, float *fHumi )
{
#ifdef USE_SHT1X
	vSHT11_Read( fTemp, fHumi, &_SHT1x_IO_Soil_Index );
#endif
}

















