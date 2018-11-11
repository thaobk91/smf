
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pin_mux.h"
#include "fsl_gpio.h"  
#include "fsl_port.h"
#include "fsl_lptmr.h"

#include "board.h"
#include "AppData.h"
#include "main.h"
#include "UART.h"
#include "PacketHandle.h"
#include "ProcessTask.h"
#include "SensorTask.h"
#include "SDCardTask.h"

#define macroTIME_DELAY_LOOP				20

extern taskHandle_t xTask;

//For uart
extern uint8_t uUART_CONN_RX_Buffer[macroUART_RX_BUFFER_LENGHT + 1];

//Uart EC sensor
#ifdef EC_SENSOR
	extern uint8_t uUART_EC_RX_Buffer[macroUART_RX_BUFFER_LENGHT + 1];
#endif

//UART for pH sensor
#ifdef PH_SENSOR
	extern uint8_t uUART_PH_RX_Buffer[macroUART_RX_BUFFER_LENGHT + 1];
#endif

extern Flags xFlags;
extern PacketIO xPacketSend;
extern SS_Value xSS_Value;
extern SS_Value xSS_Value_Current;
extern uint8_t uLedStatus;

//event system
extern uint16_t _EVENT_SYS;

//sensor feedback
extern bool bSensor_Feedback;

uint32_t uSendData_Counter = 0;

bool bData_Sending = false;
uint16_t uiSendWait_Counter = 0;
uint16_t uResend_Counter = 0;


/********************************* Function ***********************************/
static void vProcessTask_CheckEvent( void );
/******************************************************************************/


/******************************************************************************
 * Function		: void vProcessTask_Task( void *pvParameters )
 * Description	: Task xu ly kiem tra cac co de thuc hien cac action tuong ung
 * Param		: N/A
 * Return		: none
*******************************************************************************/
void vProcessTask_Task( void *pvParameters )
{	
	APP_DEBUG("*********************** ProcessTask *************************\r\n");
	APP_DEBUG("--- ProcessTask: Device init finish\r\n");
	macroTASK_DELAY_MS( 3000 );
	
    while( 1 )
    {
		macroTASK_DELAY_MS( macroTIME_DELAY_LOOP );
		macroNANO_TIMER_DONE();
		
		//blink led status	
		if(uLedStatus < 1)
		{
			macroGPIO_ACTIVE_HIGH(macroLED_STATUS_GPIO, macroLED_STATUS_GPIO_PIN);
		}
		else if(uLedStatus >= 1)
		{
			macroGPIO_ACTIVE_LOW(macroLED_STATUS_GPIO, macroLED_STATUS_GPIO_PIN);
			uLedStatus = 0;
		}
		
		//Process event
		vProcessTask_CheckEvent();
		
		//Kiem tra trang thai cac Task. Neu task nao bi loop hoac die thi restart lai
		if( xTask.uiSensorTask_Finish >= 1 )
			xTask.uiSensorTask_Finish++;
		if (xTask.uiWhoAmITask_Finish >=1)
			xTask.uiWhoAmITask_Finish++;
		if (xTask.uiSDCardTask_Finish >=1 )
			xTask.uiSDCardTask_Finish++;
		
		if( ( xTask.uiSensorTask_Finish >= (90 * (1000 / macroTIME_DELAY_LOOP)) ) ||
			( xTask.uiWhoAmITask_Finish >= (90 * (1000 / macroTIME_DELAY_LOOP)) )
		#ifdef macroUSE_SDCARD
			 || ( xTask.uiSDCardTask_Finish >= (90 * (1000 / macroTIME_DELAY_LOOP)) )
		#endif
			)
		{
			APP_DEBUG("--- ProcessTask: Task loop or die....\r\n");
			macroTASK_DELAY_MS( 1000 );
			NVIC_SystemReset();
		}
		//Refresh wdt
		vMain_RefreshWDT();
    }
}




/******************************************************************************
 * Function		: void vProcessTask_CheckEvent( void )
 * Description	: process event sys
 * Param		: none
 * Return		: none
*******************************************************************************/
static void vProcessTask_CheckEvent( void )
{
	//Recv data from connectivity
	if(_EVENT_SYS & EVENT_SYS_CONN_RECV)
	{
		vPacketHandle_HandleMessage( (char *)uUART_CONN_RX_Buffer );
		_EVENT_SYS ^= EVENT_SYS_CONN_RECV;
	}

	//Data pH
#ifdef PH_SENSOR
	if(_EVENT_SYS & EVENT_SYS_PH_SS_RECV)
	{
		APP_DEBUG("--- ProcessTask: pH data = %s\r\n", uUART_PH_RX_Buffer);
		xSS_Value_Current.fpH = atof((char*)uUART_PH_RX_Buffer);
		bSensor_Feedback = true;
		memset((void *)uUART_PH_RX_Buffer, 0, strlen((char *)uUART_PH_RX_Buffer));
		_EVENT_SYS ^= EVENT_SYS_PH_SS_RECV;
	}
#endif
	
	//Data EC
#ifdef EC_SENSOR
	if(_EVENT_SYS & EVENT_SYS_EC_SS_RECV)
	{
		APP_DEBUG("--- ProcessTask: EC data = %s\r\n", uUART_EC_RX_Buffer);
//		for(uint8_t ui = 0; ui < strlen((char *)uUART_EC_RX_Buffer); ui++)
//		{
//			if((uUART_EC_RX_Buffer[ui] >= '0') && (uUART_EC_RX_Buffer[ui] <= '9'))
//			   cStr[uk++] = uUART_EC_RX_Buffer[ui];		   
//		}
		xSS_Value_Current.fEC = atof((char*)uUART_EC_RX_Buffer) / 1000; //uS/cm to dS/m
		APP_DEBUG("--- ProcessTask: EC data current = %f\r\n", xSS_Value_Current.fEC);
		bSensor_Feedback = true;
		memset((void *)uUART_EC_RX_Buffer, 0, strlen((char *)uUART_EC_RX_Buffer));
		_EVENT_SYS ^= EVENT_SYS_EC_SS_RECV;
	}
#endif
	
	//send who am i dinh ky
	if(_EVENT_SYS & EVENT_SYS_WAMI)
	{
		vTaskResume(xTask.xTaskHandle_WhoAmI);
		_EVENT_SYS ^= EVENT_SYS_WAMI;
	}
	
	//Kiem tra doc cam bien
	if(_EVENT_SYS & EVENT_SYS_READ_SENSOR)
	{
		vTaskResume( xTask.xTaskHandle_Sensor );
		_EVENT_SYS ^= EVENT_SYS_READ_SENSOR;
	}
	
	//Kiem tra gui du lieu
	if(_EVENT_SYS & EVENT_SYS_SEND_DATA)
	{
		if(bData_Sending == false)
		{
			vPacketHandle_SendMessage( &xPacketSend );
			uSendData_Counter++;
			APP_DEBUG("--- SensorTask: Send data Counter = %d\r\n", uSendData_Counter);
			bData_Sending = true;
			xFlags.bSentIsOK = false;
			uResend_Counter = 0;
		}
		else if(xFlags.bSentIsOK == true)
		{
			bData_Sending = false;
			xFlags.bSentIsOK = false;
			_EVENT_SYS ^= EVENT_SYS_SEND_DATA;
		}
		else if(bData_Sending == true)
		{
			uiSendWait_Counter++;
			if(uiSendWait_Counter >= (5 * (1000 / macroTIME_DELAY_LOOP))) //after 5s then resend
			{
				//resend
				uResend_Counter++;
				APP_DEBUG("--- ProcessTask: Resend data counter = %d\r\n", uResend_Counter);
				vPacketHandle_SendMessage( &xPacketSend );
				uiSendWait_Counter = 0;
			}
		}
	}
}



















