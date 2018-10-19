

#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "fsl_wdog.h"
#include "fsl_uart.h"
#include "fsl_sim.h"
#include "fsl_ftm.h"
#include "fsl_sysmpu.h"		// phai dinh nghia cai nay

#include "main.h"
#include "board.h"

#include  "DeviceControl.h"
/* Task */
#include "ProcessTask.h"
#include "SensorTask.h"
#include "WhoAmITask.h"
#include "SDCardTask.h"
#include "I2C.h"
#include "UART.h"


#ifdef PH_SENSOR
	#include "SS_PH.h"
#endif

#ifdef EC_SENSOR
	#include "SS_EC.h"
#endif

#ifdef LIGHT_SENSOR
	#include "SS_Light.h"
#endif



/*******************************************************************************
 * Variables
 ******************************************************************************/

//I2C
#ifdef LIGHT_SENSOR	 
	i2c_rtos_handle_t xMaster_Handle_BH1750;
#endif

//Uart connectivity
uint8_t uUART_CONN_RX_Buffer[macroUART_RX_BUFFER_LENGHT + 1] = {0};

//Uart sensor EC
#ifdef EC_SENSOR
	uint8_t uUART_EC_RX_Buffer[macroUART_RX_BUFFER_LENGHT + 1] = {0};
#endif

//Uart sensor pH
#ifdef PH_SENSOR
	uint8_t uUART_PH_RX_Buffer[macroUART_RX_BUFFER_LENGHT + 1] = {0};
#endif

//Data local
DataLocal xDataLocal = 
{
	.uiTimeUpdate 			= macroTIME_SEND_INTERVAL,
#ifdef PH_SENSOR
    .xPH = {macroPH_THRESH_HIGH, macroPH_THRESH_LOW},
#endif
	
#ifdef EC_SENSOR
    .xEC = {macroEC_THRESH_HIGH, macroEC_THRESH_LOW},
#endif
	
#ifdef TEMP_HUMI_AIR_SENSOR
    .xTempA = {macroTEMPA_THRESH_HIGH, macroTEMPA_THRESH_LOW},
	.xHumiA = {macroHUMIA_THRESH_HIGH, macroHUMIA_THRESH_LOW},
#endif
	
#ifdef TEMP_HUMI_SOIL_SENSOR
    .xTempG = {macroTEMPG_THRESH_HIGH, macroTEMPG_THRESH_LOW},
    .xHumiG = {macroHUMIG_THRESH_HIGH, macroHUMIG_THRESH_LOW},
#endif
	
#ifdef LIGHT_SENSOR
    .xLight = {macroLIGHT_THRESH_HIGH, macroLIGHT_THRESH_LOW},
#endif
};

//Flags
Flags xFlags =
{
	.bSentIsOK					= false,
	.bConnectivityIsConnected	= false,
};



//Task
taskHandle_t xTask =
{
	.uiSensorTask_Finish = 1,
	.uiWhoAmITask_Finish = 1,
	.uiSDCardTask_Finish = 1,
};

//WDT
static WDOG_Type *xWdog_Base = WDOG;

//for timer
uint16_t uiTimerCounter = 0;
uint16_t ui16SecCounter = 0;
uint16_t ui16WAMICounter = 0;
uint16_t ui16SendCounter=0;
uint8_t uLedStatus =0;

char cID_EndDevice[17] = {0};

//event system
uint16_t _EVENT_SYS = EVENT_SYS_IDLE;

/******************************** Function ************************************/
void vMain_InitWatchdog( void );
void vMain_TimerInit(void);
void vMain_GetUniqueID(char *pID);
/******************************************************************************/



/*******************************************************************************
 * Function		: int main(void)
 * Description	: Ham chinh cua chuong trinh
 * Param		: none
 * Return		: none
*******************************************************************************/
int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
	BOARD_BootClockVLPR();
	SYSMPU_Enable(SYSMPU, false);		//khong duoc bo
	vMain_Peripheral_Init();
	
	APP_DEBUG("\r\n");
    APP_DEBUG("*******************************************************************\r\n");
    APP_DEBUG("*************->> SmartFarm Project v1.1 is Welcome <<-*************\r\n");
    APP_DEBUG("*******************************************************************\r\n");
    APP_DEBUG("\r\n");
	
//	vDeviceControl_Init();
	
	vMain_GetUniqueID(cID_EndDevice);
	APP_DEBUG("---- cID_EndDevice = %s\r\n",cID_EndDevice);
	
    if(xTaskCreate(vSensorTask_Task, "vSensorTask_Task", configMINIMAL_STACK_SIZE + 512, NULL, macroPRIORITY_TASK_SENSOR, &xTask.xTaskHandle_Sensor) != pdPASS)
    {
        APP_DEBUG("--- Main: Failed to create sensor task\r\n");
    }
	
	if(xTaskCreate(vProcessTask_Task, "vProcessTask_Task", configMINIMAL_STACK_SIZE + 1024, NULL, macroPRIORITY_TASK_PROCESS, &xTask.xTaskHandle_Process) != pdPASS)
	{
		APP_DEBUG("--- Main: Failed to create process task\r\n");
	}
   	
	if(xTaskCreate(vWhoAmITask_Task, "vWhoAmITask_Task", configMINIMAL_STACK_SIZE + 256, NULL, macroPRIORITY_TASK_WhoAmI, &xTask.xTaskHandle_WhoAmI) != pdPASS)
	{
		APP_DEBUG("--- Main: Failed to create Who Am I task\r\n");
	}
	
#ifdef macroUSE_SDCARD
	if(xTaskCreate(vSDCardTask_Run, "vSDCardTask_Run", configMINIMAL_STACK_SIZE + 1024*3, NULL, macroPRIORITY_TASK_SDCard, &xTask.xTaskHandle_SDCard) != pdPASS)
		APP_DEBUG("--- Main: Failed to create SD Card task\r\n");
#endif
	
    vTaskStartScheduler();
    
    for (;;);
}


/******************************************************************************
 * Function		: void vMain_Peripheral_Init( void )
 * Description	: Ham khoi tao ngoai vi
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_Peripheral_Init( void )
{
	BOARD_InitDebugConsole();

	vMain_InitWatchdog();
	
	vMain_TimerInit();

	/*connectivity UART */
    macroPOWER_ON(macroCONN_POWER_GPIO, macroCONN_POWER_PIN);
	vUART_Init(macroUART_CONN_BASE, macroUART_CONN_CLKSRC, macroUART_CONN_BAUDRATE, true, macroUART_CONN_IRQn);
	
#ifdef PH_SENSOR
	/* pH sensor Init */
    macroPOWER_ON(macroPH_POWER_GPIO, macroPH_POWER_PIN);
	vUART_Init(macroUART_PH_SENSOR_BASE, macroUART_PH_SENSOR_CLKSRC, macroUART_PH_SENSOR_BAUDRATE, true, macroUART_PH_SENSOR_IRQn);
#endif
	
#ifdef EC_SENSOR
	/* EC sensor UART */
    macroPOWER_ON(macroEC_POWER_GPIO, macroEC_POWER_PIN);
	vUART_Init(macroUART_EC_SENSOR_BASE, macroUART_EC_SENSOR_CLKSRC, macroUART_EC_SENSOR_BAUDRATE, true, macroUART_EC_SENSOR_IRQn);
#endif
	
#ifdef LIGHT_SENSOR	
	/* Khoi tao I2C1 for light sensor */
	vSS_Light_Init();
#endif
}




/******************************************************************************
 * Function		: void vMain_Peripheral_DeInit( void )
 * Description	: Ham stop ngoai vi
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_Peripheral_DeInit( void )
{
//	WDOG_Deinit( xWdog_Base );
////	
//	//Deinit uart
//	vUART_DeInit( macroUART_CONN_BASE );
//	
//	//Deinit uart
//	vUART_DeInit( macroUART_EC_SENSOR_BASE );
//	
////	//Deinit i2c
//	I2C_RTOS_Deinit( &xMaster_Handle_SensorGas );
//
//	DbgConsole_Deinit();
}




/******************************************************************************
 * Function		: void macroUART_CONN_IRQHandler( void )
 * Description	: This function get data receive from UART to array data_receive
 * Param		: none
 * Return		: none
*******************************************************************************/
void macroUART_CONN_IRQHandler( void )
{
	uint16_t uiLenght = strlen((char *)uUART_CONN_RX_Buffer);
	
	if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(macroUART_CONN_BASE))
    {
        uUART_CONN_RX_Buffer[uiLenght] = UART_ReadByte( macroUART_CONN_BASE );
		
        if(uiLenght >= macroUART_RX_BUFFER_LENGHT)
			memset(uUART_CONN_RX_Buffer, 0, uiLenght);
        else if(uUART_CONN_RX_Buffer[uiLenght] == macroPACKET_STRING_ENDCHAR)
            vMain_setEvent( EVENT_SYS_CONN_RECV );
    }
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}



/******************************************************************************
 * Function		: void macroUART_EC_SENSOR_IRQHandler( void )
 * Description	: This function get data receive from UART to array data_receive
 * Param		: none
 * Return		: none
*******************************************************************************/
#ifdef EC_SENSOR
	void macroUART_EC_SENSOR_IRQHandler( void )
	{
		uint16_t uiLenght = strlen((char *)uUART_EC_RX_Buffer);
		
		if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(macroUART_EC_SENSOR_BASE))
		{
			uUART_EC_RX_Buffer[uiLenght] = UART_ReadByte( macroUART_EC_SENSOR_BASE );
			
			if(uiLenght >= macroUART_RX_BUFFER_LENGHT - 1)
				memset(uUART_EC_RX_Buffer, 0, uiLenght);
			else if(uUART_EC_RX_Buffer[uiLenght] == macroEC_END_CMD)
				vMain_setEvent( EVENT_SYS_EC_SS_RECV );
		}
		
	#if defined __CORTEX_M && (__CORTEX_M == 4U)
		__DSB();
	#endif 
	}
#endif




/******************************************************************************
 * Function		: void macroUART_PH_SENSOR_IRQHandler( void )
 * Description	: This function get data receive from UART to array data_receive
 * Param		: none
 * Return		: none
*******************************************************************************/
#ifdef PH_SENSOR 
	void macroUART_PH_SENSOR_IRQHandler( void )
	{
		uint16_t uiLenght = strlen((char *)uUART_PH_RX_Buffer);
		if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(macroUART_PH_SENSOR_BASE))
		{
			uUART_PH_RX_Buffer[uiLenght] = UART_ReadByte( macroUART_PH_SENSOR_BASE );
			
			if(uiLenght >= macroUART_RX_BUFFER_LENGHT - 1)
				memset(uUART_PH_RX_Buffer, 0, macroUART_RX_BUFFER_LENGHT);
			else if(uUART_PH_RX_Buffer[uiLenght] == macroPH_END_CMD)
				vMain_setEvent( EVENT_SYS_PH_SS_RECV );
		}
	#if defined __CORTEX_M && (__CORTEX_M == 4U)
		__DSB();
	#endif
	}
#endif   




/******************************************************************************
 * Function		: void vMain_InitWatchdog( void )
 * Description	: Ham khoi tao WDT
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_InitWatchdog( void )
{
    wdog_config_t xWDTConfig;

    WDOG_GetDefaultConfig( &xWDTConfig );
    xWDTConfig.timeoutValue = macroTIME_WATCHDOG_DELAY;
    
    /* wdog refresh test in window mode */
    WDOG_Init(xWdog_Base, &xWDTConfig);
	
	for (uint32_t i = 0; i < macroWDOG_WCT_INSTRUCITON_COUNT; i++)
    {
        (void)xWdog_Base->RSTCNT;
    }
    
    /* Delay watchdog 1 second */
    while (((uint32_t)((((uint32_t)xWdog_Base->TMROUTH) << 16U) | (xWdog_Base->TMROUTL))) < (1000));
    APP_DEBUG("--- Main: Init watchdog time delay: %d ms complete\r\n", macroTIME_WATCHDOG_DELAY);
}



/******************************************************************************
 * Function		: void vMain_RefreshWDT( void )
 * Description	: refresh wdt
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_RefreshWDT( void )
{
	WDOG_Refresh(xWdog_Base);
}


/******************************************************************************
 * Function		: void vMain_TimerInit( void )
 * Description	: Init Timer 
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_TimerInit(void)
{
	ftm_config_t ftmInfo;
	
    FTM_GetDefaultConfig(&ftmInfo);

    /* Divide FTM clock by 4 */
    ftmInfo.prescale = kFTM_Prescale_Divide_4;

    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    //Set timer period.
    FTM_SetTimerPeriod(BOARD_FTM_BASEADDR, USEC_TO_COUNT(1000U, FTM_SOURCE_CLOCK));

    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(BOARD_FTM_IRQ_NUM);

    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
}


/******************************************************************************
 * Function		: void vMain_TimerInit( void )
 * Description	: Init Timer 
 * Param		: none
 * Return		: none
*******************************************************************************/
void BOARD_FTM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, kFTM_TimeOverflowFlag);
	
	uiTimerCounter++;
	if(uiTimerCounter >= 1000)
	{
		uiTimerCounter = 0;
		uLedStatus++;
		ui16SecCounter++;
		ui16WAMICounter++;
		ui16SendCounter++;
		
		if(ui16SecCounter >= macroTIME_READ_SENSOR)
		{
			vMain_setEvent( EVENT_SYS_READ_SENSOR );
			ui16SecCounter = 0;
		}
		
		if(ui16WAMICounter >= macroTIME_SEND_WAMI)
		{
			vMain_setEvent( EVENT_SYS_WAMI );
			ui16WAMICounter = 0;
		}
		
		if(ui16SendCounter >= xDataLocal.uiTimeUpdate)	//time send to connectivity           
		{
			//send to connectivity
			vMain_setEvent( EVENT_SYS_SEND_DATA );
			ui16SendCounter=0;
		}
	}
}



/******************************************************************************
 * Function		: void vMain_GetUniqueID(char *pID)
 * Description	: Ham lay Unique ID of mcu
 * Param		: pcDeviceID - ID device of mcu
 * Return		: none
*******************************************************************************/
void vMain_GetUniqueID(char *pID)
{
	sim_uid_t xSim_UID;
	char uChar[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz@-";
	
	SIM_GetUniqueId(&xSim_UID);
	pID[0] = uChar[((xSim_UID.H >> 20) & 0xFFF) / 64]; 
	pID[1] = uChar[((xSim_UID.H >> 20) & 0xFFF) % 64];
	pID[2] = uChar[((xSim_UID.H >> 8) & 0xFFF) / 64];
	pID[3] = uChar[((xSim_UID.H >> 8) & 0xFFF) % 64];
	pID[4] = uChar[(((xSim_UID.H & 0xFF) << 4) | (xSim_UID.ML >> 28)) / 64];
	pID[5] = uChar[(((xSim_UID.H & 0xFF) << 4) | (xSim_UID.ML >> 28)) % 64];
	pID[6] = uChar[((xSim_UID.ML >> 16) & 0xFFF) / 64];
	pID[7] = uChar[((xSim_UID.ML >> 16) & 0xFFF) % 64];
	pID[8] = uChar[((xSim_UID.ML >> 4) & 0xFFF) / 64];
	pID[9] = uChar[((xSim_UID.ML >> 4) & 0xFFF) % 64];
	pID[10] = uChar[((xSim_UID.ML & 0xF) | (xSim_UID.L >> 24)) / 64];
	pID[11] = uChar[((xSim_UID.ML & 0xF) | (xSim_UID.L >> 24)) % 64];
	pID[12] = uChar[((xSim_UID.L >> 12) & 0xFFF) / 64];
	pID[13] = uChar[((xSim_UID.L >> 12) & 0xFFF) % 64];
	pID[14] = uChar[(xSim_UID.L & 0xFFF) / 64];
	pID[15] = uChar[(xSim_UID.L & 0xFFF) % 64];
}




/******************************************************************************
 * Function		: void vMain_setEvent( uint16_t Event )
 * Description	: set event
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_setEvent( uint16_t Event )
{
	_EVENT_SYS |= Event;
}















