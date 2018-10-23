
#include "Control.h"
#include "AppData.h"



void vControl_set_OutVAC( OutVAC e_OutVAC, uint8_t *pState )
{
	//out 1
	if( (e_OutVAC & outVAC_1) == outVAC_1 )
	{
		if( pState[0] == 1 )
			macroOUTPUT_1_ON();
		else if( pState[0] == 0 )
			macroOUTPUT_1_OFF();
	}
	//out2
	if( (e_OutVAC & outVAC_2) == outVAC_2 )
	{
		if( pState[1] == 1 )
			macroOUTPUT_2_ON();
		else if( pState[1] == 0 )
			macroOUTPUT_2_OFF();
	}
	//out3
	if( (e_OutVAC & outVAC_3) == outVAC_3 )
	{
		if( pState[2] == 1 )
			macroOUTPUT_3_ON();
		else if( pState[2] == 0 )
			macroOUTPUT_3_OFF();
	}
	//out4
	if( (e_OutVAC & outVAC_4) == outVAC_4 )
	{
		if( pState[3] == 1 )
			macroOUTPUT_4_ON();
		else if( pState[3] == 0 )
			macroOUTPUT_4_OFF();
	}
	//out5
	if( (e_OutVAC & outVAC_5) == outVAC_5 )
	{
		if( pState[4] == 1 )
			macroOUTPUT_5_ON();
		else if( pState[4] == 0 )
			macroOUTPUT_5_OFF();
	}
	//out6
	if( (e_OutVAC & outVAC_6) == outVAC_6 )
	{
		if( pState[5] == 1 )
			macroOUTPUT_6_ON();
		else if( pState[5] == 0 )
			macroOUTPUT_6_OFF();
	}
}



void vControl_get_OutVAC( uint8_t *pState )
{
	uint32_t uValue = 0;

	uValue = macroREAD_OUTPUT_1();
	pState[0] = (uValue != 0)? 0 : 1;

	uValue = macroREAD_OUTPUT_2();
	pState[1] = (uValue != 0)? 0 : 1;

	uValue = macroREAD_OUTPUT_3();
	pState[2] = (uValue != 0)? 0 : 1;

	uValue = macroREAD_OUTPUT_4();
	pState[3] = (uValue != 0)? 0 : 1;

	uValue = macroREAD_OUTPUT_5();
	pState[4] = (uValue != 0)? 0 : 1;

	uValue = macroREAD_OUTPUT_6();
	pState[5] = (uValue != 0)? 0 : 1;
}






