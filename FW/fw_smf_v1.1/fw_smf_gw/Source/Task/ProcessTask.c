
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
#include "RTC.h"
#include "IOControl.h"
#include "ProcessMsg.h"

#include "WhoAmITask.h"
#include "ProcessTask.h"


#define macroTIME_DELAY_LOOP				10
#define macroTIME_LED_STATUS_OFF			3000 //3s
#define macroTIME_LED_STATUS_ON				100 //0.1s
#define macroTIME_TASK_RUN_TO_FINISH		90000//90s


extern taskHandle_t xTask;
extern uint16_t Event_IO;

//Uart to Connectivity
extern uint8_t uUART_CONN_RX_Buffer[macroUART_RX_BUFFER_LENGHT];
extern uint8_t uUART_CONN_TX_Buffer[macroUART_TX_BUFFER_LENGHT];
extern uint16_t uiUART_CONN_RX_Lenght;

//UART to network
extern uint8_t uUART_NWK_RX_Buffer[macroUART_RX_BUFFER_LENGHT];
extern uint8_t uUART_NWK_TX_Buffer[macroUART_TX_BUFFER_LENGHT];
extern uint16_t uiUART_NWK_RX_Lenght;

//response
extern bool isResponse;

extern WhoAmI _WhoAmI;

//RTC
extern rtc_datetime_t _RTC;

//sync RTC
extern bool bRTC_Sync;
uint16_t uMinuteCounter = 0;
uint8_t uMinuteOld = 0;
uint8_t uHourOld = 0;

uint16_t uLedStatus = 0;

uint32_t uiNWK_Send_Counter = 0;
uint16_t uiWaitNWK_Connected_Counter = 0;
uint16_t uMessageNWK_Send_Counter = 0;
bool bSend_D_State = false;


/********************************* Function ***********************************/
static void vProcessTask_Event( void );
/******************************************************************************/


/******************************************************************************
 * Function		: void vProcessTask_Run( void *pvParameters )
 * Description	: Task xu ly kiem tra cac co de thuc hien cac action tuong ung
 * Param		: N/A
 * Return		: none
*******************************************************************************/
void vProcessTask_Run( void *pvParameters )
{	
	APP_DEBUG("*********************** ProcessTask *************************\r\n");
	APP_DEBUG("--- ProcessTask: Device init finish\r\n");
	
	macroTASK_DELAY_MS( 1000 );
	uMinuteOld = _RTC.minute;
	vIOControl_Init();

    while( 1 )
    {
    	xTask.uiProcessTask_Finish = 1;
		macroTASK_DELAY_MS( macroTIME_DELAY_LOOP );
		
		uLedStatus++;		
		//blink led status	
		if( uLedStatus < ((macroTIME_LED_STATUS_ON < macroTIME_DELAY_LOOP)? 1 : (macroTIME_LED_STATUS_ON / macroTIME_DELAY_LOOP)) )
			macroLED_STATUS_ON();
		else if(uLedStatus < (macroTIME_LED_STATUS_OFF / macroTIME_DELAY_LOOP))
			macroLED_STATUS_OFF();
		else
			uLedStatus = 0;
		
		vRTC_GetDateTime(&_RTC);
		if(uMinuteOld != _RTC.minute)
		{
			APP_DEBUG("--- ProcessTask: RTC = %d:%d:%d - %d/%d/%d\r\n", _RTC.hour, _RTC.minute, _RTC.second, _RTC.day, _RTC.month, _RTC.year);
			uMinuteOld = _RTC.minute;
			bSend_D_State = false;
			macroNANO_TIME_DONE();
			
			if(bRTC_Sync == false)
				vProcessMsg_Send_Request(macroID_REQS_RTC, false);

			uMinuteCounter++;
			if( uMinuteCounter > (60*24) )
			{
				bRTC_Sync = false;
				uMinuteCounter = 0;
			}
			
			if( (bSend_D_State == false) && ((uMinuteCounter % 15) == 0) )
			{
				vProcessMsg_Send_Data( NULL, macroID_DATA_DEVICE_STATE, macroRESP_OK );
				bSend_D_State = true;
			}
		}

		//process event
		vProcessTask_Event();
		
		//Kiem tra trang thai cac Task. Neu task nao bi loop hoac die thi restart lai
		if( xTask.uiWamiTask_Finish >= 1 )
			xTask.uiWamiTask_Finish++;
	#ifdef macroUSE_SDCARD
		if( xTask.uiSDCardTask_Finish >= 1 )
			xTask.uiSDCardTask_Finish++;
	#endif

		//Kiem tra trang thai cac Task. Neu task nao bi loop hoac die thi restart lai
		if( (xTask.uiWamiTask_Finish >= (macroTIME_TASK_RUN_TO_FINISH / macroTIME_DELAY_LOOP))  || (xTask.uiProcessTask_Finish >= macroTIME_TASK_RUN_TO_FINISH / macroTIME_DELAY_LOOP)
	#ifdef macroUSE_SDCARD
			|| (xTask.uiSDCardTask_Finish >= macroTIME_TASK_RUN_TO_FINISH / macroTIME_DELAY_LOOP)
	#endif
			)
		{
			APP_DEBUG("--- ProcessTask: task is loop. Reseting...\r\n");
			macroTASK_DELAY_MS( 1000 );
			NVIC_SystemReset();
		}

		//Refresh wdt
		vMain_RefreshWDT();
		xTask.uiProcessTask_Finish = 0;
    }
}




/******************************************************************************
 * Function		: void vProcessTask_Event( void )
 * Description	: Process event
 * Param		: None
 * Return		: None
*******************************************************************************/
static void vProcessTask_Event( void )
{
	//event
	uint16_t uEvent = uiMain_getEvent();
	
	if(uEvent & EVENT_RTC_CONFIG)		//uart conn received
	{
		APP_DEBUG("\r\n--- ProcessTask: RTC Config done\r\n");
		bRTC_Sync = true;
		uEvent ^= EVENT_RTC_CONFIG;
	}

	if(uEvent & EVENT_UART_CONN_RECEIVED)		//uart conn received
	{
		APP_DEBUG("\r\n--- ProcessTask: EVENT_UART_CONN_RECEIVED\r\n");
		APP_DEBUG("--- ProcessTask: uart conn recv = \"%s\"\r\n", uUART_CONN_RX_Buffer);
		vProcessMsg_AllMessage(uUART_CONN_RX_Buffer, true, true);
		uiUART_CONN_RX_Lenght = 0;
		uEvent ^= EVENT_UART_CONN_RECEIVED;
	}

	if(uEvent & EVENT_UART_NWK_SEND)		//uart nwk send
	{
		//APP_DEBUG("\r\n--- ProcessTask: EVENT_UART_NWK_SEND\r\n");
		if(isResponse == false)
		{
			if((uiNWK_Send_Counter == 0) || (uiNWK_Send_Counter % (5000 / macroTIME_DELAY_LOOP) == 0))
			{
				isResponse = false;
				UART_NWK_WRITE_DATA(uUART_NWK_TX_Buffer);
			}
			if(uiNWK_Send_Counter >= (120 * 1000 / macroTIME_DELAY_LOOP))
			{
				APP_DEBUG("--- ProcessTask: send data to nwk failture. Reseting...\r\n");
				uiNWK_Send_Counter = 0;
				macroTASK_DELAY_MS( 1000 );
				NVIC_SystemReset();
			}
			uiNWK_Send_Counter++;
			vMain_setEvent(EVENT_UART_NWK_SEND);
		}
		else
		{
			uMessageNWK_Send_Counter++;
			APP_DEBUG("--- ProcessTask: UART nwk send finish. Data Count = %d\r\n", uMessageNWK_Send_Counter);
			uiNWK_Send_Counter = 0;
			isResponse = false;
		}
		uEvent ^= EVENT_UART_NWK_SEND;
	}

	if(uEvent & EVENT_UART_CONN_SEND)		//uart conn send
	{
		APP_DEBUG("\r\n--- ProcessTask: EVENT_UART_CONN_SEND\r\n");
		UART_CONN_WRITE_DATA(uUART_CONN_TX_Buffer);
		uEvent ^= EVENT_UART_CONN_SEND;
	}

	if(uEvent & EVENT_UART_NWK_RECEIVED)	//uart nwk received
	{
		APP_DEBUG("\r\n--- ProcessTask: EVENT_UART_NWK_RECEIVED\r\n");
		APP_DEBUG("--- ProcessTask: uart nwk recv = \"%s\"\r\n", uUART_NWK_RX_Buffer);
		vProcessMsg_AllMessage(uUART_NWK_RX_Buffer, true, false);
		uiUART_NWK_RX_Lenght = 0;
		uEvent ^= EVENT_UART_NWK_RECEIVED;
	}
	
	if(uEvent & EVENT_WAIT_NWK_CONNECTED)		//uart nwk send
	{
		APP_DEBUG("\r\n--- ProcessTask: EVENT_WAIT_NWK_CONNECTED\r\n");
		if( (_WhoAmI.Network.Wifi == true) || (_WhoAmI.Network._2G == true) || (_WhoAmI.Network._3G == true) )
			vMain_setEvent(EVENT_UART_NWK_SEND);
		uEvent ^= EVENT_WAIT_NWK_CONNECTED;
	}

	Event_IO |= uEvent;
}

















