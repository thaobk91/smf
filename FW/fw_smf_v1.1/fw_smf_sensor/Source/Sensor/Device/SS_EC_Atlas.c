

#include "main.h"
#include "UART.h"
#include "SS_EC_Atlas.h"
#include <stdio.h>


void vSS_EC_Atlas_SendCMD(char* cmd)
{
    vUART_Write(macroUART_EC_SENSOR_BASE, (char*)cmd, strlen(cmd));
}



/*******************************************************************************/
void vSS_EC_Atlas_Init( void )
{
	//disable response
	vSS_EC_Atlas_SendCMD(macroEC_DISABLE_RESP_CMD);
	macroTASK_DELAY_MS(200);
	//disable read continous mode
	vSS_EC_Atlas_SendCMD(macroEC_DISABLE_CONT_MODE_CMD);
	macroTASK_DELAY_MS(200);
}



/*******************************************************************************/
void vSS_EC_Atlas_Read( float fTemp )
{
	//char cStr[16] = {0};
	//sprintf(cStr, "RT,%0.2f\r\n", fTemp);
	//vSS_EC_Atlas_SendCMD(cStr);
	macroTASK_DELAY_MS(200);
	
	vSS_EC_Atlas_SendCMD(macroEC_SINGLE_READ_CMD);
}
