
#include "AppData.h"
#include "WhoAmITask.h"
#include "IOControl.h"
#include "ProcessMsg.h"
#include "main.h"
#include "UART.h"


#define macroCOUNTER_RESET				3

#define macroWAMI_LOOP_TIME				15000 //ms

#define macroNWK_UART_FAIL_TIME			90000	//ms

#define macroCONN_UART_FAIL_TIME		90000	//ms


//Task
extern taskHandle_t xTask;

//Who am i
extern WhoAmI _WhoAmI;

uint8_t uWhoAmI_SendCounter = 0;
uint8_t uNWKState_Counter = 0;
uint8_t uNWKConnection_Counter = 0;
uint8_t uCONNState_Counter = 0;
bool Reconnect = false;
uint8_t uSmartConfig_Counter = 0;

uint8_t uNWK_UART_Fail_Counter = 0;
uint8_t uCONN_UART_Fail_Counter = 0;



/******************************************************************************
 * Function		: void vWhoAmITask_Run( void *pvParameters)
 * Description	: Task xu ly ban tin Who Am I
 * Param		: none
 * Return		: none
*******************************************************************************/
void vWhoAmITask_Run( void *pvParameters)
{
	APP_DEBUG("********************** Who Am I Task *********************\r\n");
	macroTASK_DELAY_MS(10000);
	
    while( 1 )
    {
    	xTask.uiWamiTask_Finish = 1;

    	_WhoAmI.Network.Wifi = _WhoAmI.Network._2G = _WhoAmI.Network._3G = _WhoAmI.Network.ConnectionOK = false;
    	_WhoAmI.Connectivity.Zigbee = _WhoAmI.Connectivity.Sub1Ghz = false;
    	vProcessMsg_Send_Wami();
    	uWhoAmI_SendCounter++;
    	macroTASK_DELAY_MS(macroWAMI_LOOP_TIME);

    	if(uSmartConfig_Counter > 0)
    	{
    		uSmartConfig_Counter++;
			if(uSmartConfig_Counter == 6)
				uSmartConfig_Counter = 0;
    	}

    	if( (_WhoAmI.Network.Wifi == false) && (_WhoAmI.Network._3G == false) && (_WhoAmI.Network._2G == false) )
		{
    		uNWKState_Counter++;
			uNWK_UART_Fail_Counter++;
		}
		else
			uNWK_UART_Fail_Counter = 0;
			
    	if( _WhoAmI.Network.ConnectionOK == false )
    		uNWKConnection_Counter++;
		
		if( (_WhoAmI.Connectivity.Zigbee == false) && (_WhoAmI.Connectivity.Sub1Ghz == false) )
		{
			uCONN_UART_Fail_Counter++;
			uCONNState_Counter++;
		}
		else
		{
			uCONN_UART_Fail_Counter = 0;
		}

		if(uWhoAmI_SendCounter >= macroCOUNTER_RESET)
		{
			if(uNWK_UART_Fail_Counter >= (macroNWK_UART_FAIL_TIME / macroWAMI_LOOP_TIME))
			{
				APP_DEBUG("--- WhoAmITask: nwk uart fail. reseting...\r\n");
				macroTASK_DELAY_MS(1000);
				NVIC_SystemReset();
			}
			
			if(uCONN_UART_Fail_Counter >= (macroCONN_UART_FAIL_TIME / macroWAMI_LOOP_TIME))
			{
				APP_DEBUG("--- WhoAmITask: conn uart fail. reseting...\r\n");
				macroTASK_DELAY_MS(1000);
				NVIC_SystemReset();
			}
			
			if( ((uNWKState_Counter >= macroCOUNTER_RESET) || (uNWKConnection_Counter >= macroCOUNTER_RESET)) && (uSmartConfig_Counter == 0) )
			{
				APP_DEBUG("--- WhoAmITask: network lost. restarting network...\r\n");
				macroNWK_POWER_OFF(); macroNWK_POWER_WIFI_OFF();
				macroNWK_POWER_KEY_OFF();
				macroNWK_RESET_ACTIVE(); macroNWK_RESET_WIFI_ON();
				vUART_DeInit(macroUART_NETWORK_BASE);	//reset uart
				macroTASK_DELAY_MS(200);
				macroNWK_POWER_ON(); macroNWK_POWER_WIFI_ON();
				macroNWK_POWER_KEY_ON();
				macroNWK_RESET_DEACTIVE(); macroNWK_RESET_WIFI_OFF();
				vUART_Init(macroUART_NETWORK_BASE, macroUART_NETWORK_CLKSRC, macroUART_NETWORK_BAUDRATE, true, macroUART_NETWORK_IRQn);
				Reconnect = true;
				macroTASK_DELAY_MS(500);
			}

			if( uCONNState_Counter >= macroCOUNTER_RESET )
			{
				APP_DEBUG("--- WhoAmITask: connectivity lost. restarting connectivity...\r\n");
				macroCONN_POWER_OFF();
				macroRESET_CONN_ACTIVE();
				vUART_DeInit(macroUART_CONNECTIVITY_BASE);	//reset uart
				macroTASK_DELAY_MS(200);
				macroCONN_POWER_ON();
				macroRESET_CONN_DEACTIVE();
				vUART_Init(macroUART_CONNECTIVITY_BASE, macroUART_CONNECTIVITY_CLKSRC, macroUART_CONNECTIVITY_BAUDRATE, true, macroUART_CONNECTIVITY_IRQn);
				Reconnect = true;
				macroTASK_DELAY_MS(500);
			}
			uNWKState_Counter = uCONNState_Counter = uNWKConnection_Counter = uWhoAmI_SendCounter = 0;
		}

		//Task finish
		xTask.uiWamiTask_Finish = 0;
//        vTaskSuspend( NULL );
		if(Reconnect == true)
		{
			Reconnect = false;
			macroTASK_DELAY_MS(macroWAMI_LOOP_TIME);
		}
    }
}













