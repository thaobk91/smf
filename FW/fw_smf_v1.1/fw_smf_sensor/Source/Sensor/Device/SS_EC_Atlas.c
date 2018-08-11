

#include "main.h"
#include "UART.h"
#include "SS_EC_Atlas.h"


void vSS_EC_Atlas_SendCMD(char* cmd)
{
    vUART_Write(macroUART_EC_SENSOR_BASE, (char*)cmd, strlen(cmd));
}



/*******************************************************************************/
void vSS_EC_Atlas_Init(void)
{
	//disable response
	vSS_EC_Atlas_SendCMD(macroEC_DISABLE_RESP_CMD);
	macroTASK_DELAY_MS(200);
	//disable read continous mode
	vSS_EC_Atlas_SendCMD(macroEC_DISABLE_CONT_MODE_CMD);
	macroTASK_DELAY_MS(200);
}



/*******************************************************************************/
void vSS_EC_Atlas_Read(void)
{
	vSS_EC_Atlas_SendCMD(macroEC_SINGLE_READ_CMD);
}
