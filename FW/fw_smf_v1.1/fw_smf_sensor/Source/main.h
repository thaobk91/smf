
#ifndef __MAIN_H_
#define __MAIN_H_
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
* INCLUDES
*/
/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "fsl_debug_console.h"
#include "fsl_rtc.h"
#include "fsl_ftm.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
/*********************************************************************
* CONSTANTS
*/
#define MAIN_DEBUG
#ifdef MAIN_DEBUG
	#define APP_DEBUG 								DbgConsole_Printf
#else
	#define APP_DEBUG(...)
#endif

	
#define macroTASK_DELAY_MS( msec )					vTaskDelay(msec / portTICK_PERIOD_MS)

	
// Define time watchdog
#define macroTIME_WATCHDOG_DELAY     				(120000) 
#define macroWDOG_WCT_INSTRUCITON_COUNT 			(256U)

	
//define for timer
/* The Flextimer instance/channel used for board */
#define BOARD_FTM_BASEADDR 							FTM0

/* Interrupt number and interrupt handler for the FTM instance used */
#define BOARD_FTM_IRQ_NUM 							FTM0_IRQn
#define BOARD_FTM_HANDLER 							FTM0_IRQHandler

#define macro_GPIO_IE_IRQ							PORTD_IRQn
#define macro_GPIO_IE_IRQHandler					PORTA_IRQHandler 
/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK 							(CLOCK_GetFreq(kCLOCK_BusClk)/4)
	
	
#define macroGPIO_ACTIVE_HIGH( GPIO, PIN )   		GPIO_WritePinOutput( GPIO, PIN, 1 )
#define macroGPIO_ACTIVE_LOW( GPIO, PIN )  			GPIO_WritePinOutput( GPIO, PIN, 0 )
#define macroGPIO_TOGGLE( GPIO, PIN )				GPIO_TogglePinsOutput( GPIO, 1u << PIN )
#define macroGPIO_READ_STATE( GPIO, PIN )			GPIO_ReadPinInput( GPIO, PIN )
		
#define macroNANO_TIMER_DONE()                  	macroGPIO_TOGGLE(macroNANO_TIMER_DONE_GPIO,macroNANO_TIMER_DONE_PIN);

//power for connectivity
#define macroPOWER_ON(GPIO,PIN)						macroGPIO_ACTIVE_LOW( GPIO,PIN)
#define macroPOWER_OFF(GPIO,PIN)					macroGPIO_ACTIVE_HIGH( GPIO,PIN)
	

//For Task
/* Task priorities. */
#define macroPRIORITY_TASK_PROCESS					(configMAX_PRIORITIES - 0)
#define macroPRIORITY_TASK_SENSOR					(configMAX_PRIORITIES - 1)
#define macroPRIORITY_TASK_WhoAmI					(configMAX_PRIORITIES - 2)
#define macroPRIORITY_TASK_SDCard					(configMAX_PRIORITIES - 3)


typedef struct
{
	TaskHandle_t 	xTaskHandle_Process;
	TaskHandle_t 	xTaskHandle_Sensor;
	TaskHandle_t 	xTaskHandle_WhoAmI;
#ifdef macroUSE_SDCARD
	TaskHandle_t 	xTaskHandle_SDCard;
#endif
	
	uint16_t		uiSensorTask_Finish;
	uint16_t		uiWhoAmITask_Finish;
	uint16_t 		uiSDCardTask_Finish;
}taskHandle_t;



typedef struct 
{
  float ui16HighThresh;
  float ui16LowThresh;
}Thresh_t;

//for some data
typedef struct
{
	int16_t			uiTimeUpdate;				//Time update data
#ifdef PH_SENSOR
    Thresh_t        xPH;
#endif

#ifdef EC_SENSOR
    Thresh_t        xEC;
#endif

#ifdef TEMP_HUMI_AIR_SENSOR
    Thresh_t        xTempA;
    Thresh_t        xHumiA;
#endif

#ifdef TEMP_HUMI_SOIL_SENSOR
    Thresh_t        xTempG;
    
    Thresh_t        xHumiG;
#endif
	
#ifdef LIGHT_SENSOR
    Thresh_t        xLight;
#endif
}DataLocal;



//event
#define EVENT_SYS_IDLE							0x0000	
#define EVENT_SYS_CONN_RECV						0x0001

#ifdef PH_SENSOR
	#define EVENT_SYS_PH_SS_RECV				0x0002
#endif

#ifdef EC_SENSOR
	#define EVENT_SYS_EC_SS_RECV				0x0004
#endif
#define EVENT_SYS_SEND_READY					0x0008
#define EVENT_SYS_WAMI							0x0010
#define EVENT_SYS_READ_SENSOR					0x0020
#define EVENT_SYS_SEND_DATA						0x0040
#define EVENT_SYS_RESPONSE						0x0080





/******************************************************************************
 * Function		: void vMain_Peripheral_Init( void )
 * Description	: Ham khoi tao ngoai vi
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_Peripheral_Init( void );




/******************************************************************************
 * Function		: void vMain_Peripheral_DeInit( void )
 * Description	: Ham stop ngoai vi
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_Peripheral_DeInit( void );




/******************************************************************************
 * Function		: void vMain_RefreshWDT( void )
 * Description	: refresh wdt
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_RefreshWDT( void );



/******************************************************************************
 * Function		: void vMain_setEvent( uint16_t Event )
 * Description	: set event
 * Param		: none
 * Return		: none
*******************************************************************************/
void vMain_setEvent( uint16_t Event );



//void vMain_TimerInit(void);
#ifdef __cplusplus
}
#endif
#endif








