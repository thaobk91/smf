
#include "SS_PH_Atlas.h"
#include "UART.h"
#include "main.h"

#define macroPH_SEND_CMD(uart_base,cmd, len_cmd)		vUART_Write(macroUART_PH_SENSOR_BASE, (char*)cmd, len_cmd)



static void vSS_PH_Atlas_SendCMD(char* cmd)
{
    vUART_Write(macroUART_PH_SENSOR_BASE, (char*)cmd, strlen(cmd));
}



/*******************************************************************************/
void vSS_PH_Atlas_Init(void)
{
	//disable response
	vSS_PH_Atlas_SendCMD(macroPH_DISABLE_RESP_CMD);
	macroTASK_DELAY_MS(200);
	//disable read continous mode
	vSS_PH_Atlas_SendCMD(macroPH_DISABLE_CONT_MODE_CMD);
	macroTASK_DELAY_MS(200);
}





/*****************************************************************************/
void vSS_PH_Atlas_Read(void)
{
	vSS_PH_Atlas_SendCMD(macroPH_SINGLE_READ_CMD);
}
