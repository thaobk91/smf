

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "fsl_gpio.h"  
#include "fsl_port.h"

#include "main.h"
#include "AppData.h"

#include "I2C.h"
#include "UART.h"
#include "SensorTask.h"
#include "DeviceControl.h"

#ifdef PH_SENSOR
	#include "SS_PH.h"
#endif

#if defined(TEMP_HUMI_AIR_SENSOR) || defined(TEMP_HUMI_SOIL_SENSOR)
	#include "SS_TempAndHumi.h"
#endif

#ifdef LIGHT_SENSOR
	#include "SS_Light.h"
#endif

#ifdef EC_SENSOR
	#include "SS_EC.h"
#endif


PacketIO xPacketSend = {0};

SS_Value xSS_Value = {0};

SS_Value xSS_Value_Current = {0};		//luu tru gia tri sensor tuc thoi

//Task
extern taskHandle_t xTask;

//Flags
extern Flags xFlags;

extern char cID_EndDevice[18];

extern DataLocal xDataLocal;

uint32_t uReadSS_Counter = 0;

bool bSensor_Feedback = false;


/********************************* Function ***********************************/
static void vSensorTask_Wait_SS_Feedback( void );
/******************************************************************************/





/******************************************************************************
 * Function		: void vSensorTask_Task( void *pvParameters)
 * Description	: Task xu ly doc du lieu tu sensor
 * Param		: N/A
 * Return		: none
*******************************************************************************/
void vSensorTask_Task( void *pvParameters)
{
	
	APP_DEBUG("************************* SensorTask *************************\r\n");
	macroTASK_DELAY_MS(1000);
#ifdef PH_SENSOR
	vSS_PH_Init();
#endif
	
#ifdef EC_SENSOR
	vSS_EC_Init();
#endif
	
	macroTASK_DELAY_MS(20000);
    while( 1 )
    {
		xTask.uiSensorTask_Finish = 1;

		uReadSS_Counter++;
		APP_DEBUG("--- SensorTask: Reading sensor... Counter = %d\r\n", uReadSS_Counter);
		
#ifdef TEMP_HUMI_AIR_SENSOR
		/*TempA & HumiA*/
		APP_DEBUG("--- SensorTask: Reading Temp & Humi Air sensor\r\n");
		vSS_TempAndHumi_Air_Index( &xSS_Value_Current.fTempA, &xSS_Value_Current.fHumiA );
		
		//check val vua doc dc
		if((xSS_Value_Current.fTempA > xDataLocal.xTempA.ui16LowThresh) && (xSS_Value_Current.fTempA < xDataLocal.xTempA.ui16HighThresh))
			xSS_Value.fTempA = xSS_Value_Current.fTempA;
		if((xSS_Value_Current.fHumiA > xDataLocal.xHumiA.ui16LowThresh) && (xSS_Value_Current.fHumiA < xDataLocal.xHumiA.ui16HighThresh))
			xSS_Value.fHumiA = xSS_Value_Current.fHumiA;
#endif
			
#ifdef TEMP_HUMI_SOIL_SENSOR
			/*TempG & HumiG*/
		APP_DEBUG("--- SensorTask: Reading Temp & Humi Soil sensor\r\n");
		vSS_TempAndHumi_Soil_Index( &xSS_Value_Current.fTempG, &xSS_Value_Current.fHumiG );
		
		//check val vua doc dc
		if((xSS_Value_Current.fTempG > xDataLocal.xTempG.ui16LowThresh) && (xSS_Value_Current.fTempG < xDataLocal.xTempG.ui16HighThresh))
			xSS_Value.fTempG = xSS_Value_Current.fTempG;
		if((xSS_Value_Current.fHumiG > xDataLocal.xHumiG.ui16LowThresh) && (xSS_Value_Current.fHumiG < xDataLocal.xHumiG.ui16HighThresh))
			xSS_Value.fHumiG = xSS_Value_Current.fHumiG;
#endif
		
#ifdef PH_SENSOR
		/*pH sensor, data pH already store xss_value.fpH after this funtion*/
		APP_DEBUG("--- SensorTask: Reading pH sensor\r\n");
		vSS_PH_Read( xSS_Value.fTempG );
		vSensorTask_Wait_SS_Feedback();
		//check val pH vua doc dc
		if((xSS_Value_Current.fpH >= xDataLocal.xPH.ui16LowThresh) && (xSS_Value_Current.fpH <= xDataLocal.xPH.ui16HighThresh))
			xSS_Value.fpH = xSS_Value_Current.fpH;
#endif

#ifdef EC_SENSOR
		/*EC sensor, data ec already store xss_value.fEC after this funtion*/
		APP_DEBUG("--- SensorTask: Reading EC sensor\r\n");
		vSS_EC_Read( xSS_Value.fTempG );
		vSensorTask_Wait_SS_Feedback();
		//check val vua doc dc
		if((xSS_Value_Current.fEC >= xDataLocal.xEC.ui16LowThresh) && (xSS_Value_Current.fEC <= xDataLocal.xEC.ui16HighThresh))
			xSS_Value.fEC = xSS_Value_Current.fEC;
#endif

#ifdef LIGHT_SENSOR
		/*Light sensor*/
		APP_DEBUG("--- SensorTask: Reading Light sensor\r\n");
		vSS_Light_getLux( &xSS_Value_Current.fLight );

		//check val vua doc dc
		if((xSS_Value_Current.fLight >= xDataLocal.xLight.ui16LowThresh) && (xSS_Value_Current.fLight <= xDataLocal.xLight.ui16HighThresh))
			xSS_Value.fLight = xSS_Value_Current.fLight;
#endif

#ifdef PH_SENSOR
		APP_DEBUG("--- SensorTask: pH = %f\r\n",xSS_Value.fpH);
#endif

#ifdef EC_SENSOR			
		APP_DEBUG("--- SensorTask: EC = %f\r\n",xSS_Value.fEC);
#endif           
			
#ifdef TEMP_HUMI_AIR_SENSOR           
		APP_DEBUG("--- SensorTask: TempA = %f, HumiA = %f\r\n",xSS_Value.fTempA,xSS_Value.fHumiA);
#endif            
			
#ifdef TEMP_HUMI_SOIL_SENSOR            
		APP_DEBUG("--- SensorTask: TempG = %f, HumiG = %f\r\n",xSS_Value.fTempG,xSS_Value.fHumiG);
#endif            
			
#ifdef LIGHT_SENSOR            
		APP_DEBUG("--- SensorTask: Light Lux = %d\r\n",xSS_Value.fLight);
#endif			
		vSensorTask_SetDataSensor( &xPacketSend, &xSS_Value, macroTYPEPACKET_DATA );
		
		//Task finish
		xTask.uiSensorTask_Finish = 0;
		vTaskSuspend( NULL );
    }
}


/******************************************************************************
 * Function		: void vSensorTask_SetDataSensor( PacketIO *xPacketIO, SS_Value *xSS_Value, char *cTypePacket )
 * Description	: Ham tao cau truc du lieu
 * Param		: 
 * 				+ xPacketIO - Cau truc chua du lieu
 *				+ xSS_Value: cau truc du lieu dau vao
 *				+ cTypePacket - loai ban tin
 * Return		: none
*******************************************************************************/
void vSensorTask_SetDataSensor( PacketIO *xPacketIO, SS_Value *xSS_Value, char *cTypePacket )
{
	uint8_t uCnt = 0;
	vPacketHandle_Coppy(xPacketIO->cTypePacket, cTypePacket, 0);
	vPacketHandle_Coppy(xPacketIO->cTypeDevice, macroTYPEDEVICE_SS, 0);
    vPacketHandle_Coppy(xPacketIO->cID, cID_EndDevice , 0);

#ifdef PH_SENSOR
	//pH
	vPacketHandle_Coppy(xPacketIO->Data[uCnt].cName, macroSENSOR_PH, 0);
	sprintf(xPacketIO->Data[uCnt].cInfo, "%0.2f", xSS_Value->fpH);
	uCnt++;
#endif

#ifdef EC_SENSOR
	//EC
	vPacketHandle_Coppy(xPacketIO->Data[uCnt].cName, macroSENSOR_EC, 0);
	sprintf(xPacketIO->Data[uCnt].cInfo, "%0.2f", xSS_Value->fEC);
	uCnt++;
#endif

#ifdef TEMP_HUMI_SOIL_SENSOR
	//TempG
	vPacketHandle_Coppy(xPacketIO->Data[uCnt].cName, macroSENSOR_TEMPG, 0);
	sprintf(xPacketIO->Data[uCnt].cInfo, "%0.2f", xSS_Value->fTempG);
	uCnt++;

	//Humi
	vPacketHandle_Coppy(xPacketIO->Data[uCnt].cName, macroSENSOR_HUMIG, 0);
	sprintf(xPacketIO->Data[uCnt].cInfo, "%0.2f", xSS_Value->fHumiG);
	uCnt++;
#endif
	
#ifdef TEMP_HUMI_AIR_SENSOR
	//TempA
	vPacketHandle_Coppy(xPacketIO->Data[uCnt].cName, macroSENSOR_TEMPA, 0);
	sprintf(xPacketIO->Data[uCnt].cInfo, "%0.2f", xSS_Value->fTempA);
	uCnt++;
	
	//HumiA
	vPacketHandle_Coppy(xPacketIO->Data[uCnt].cName, macroSENSOR_HUMIA, 0);
	sprintf(xPacketIO->Data[uCnt].cInfo, "%0.2f", xSS_Value->fHumiA);
	uCnt++;
#endif
	
#ifdef LIGHT_SENSOR
	//Light
	vPacketHandle_Coppy(xPacketIO->Data[uCnt].cName, macroSENSOR_LIGHT, 0);
	sprintf(xPacketIO->Data[uCnt].cInfo, "%d", xSS_Value->fLight);
	uCnt++;
#endif
	xPacketIO->uDataNumber = uCnt;
}
		
		

static void vSensorTask_Wait_SS_Feedback( void )
{
	bSensor_Feedback = false;
	uint8_t uTimeout = 0;
	
	#define macroWAIT_LOOP_TIME							200
	#define macroSENSOR_FEEDBACK_TIMEOUT				5000	//5s
		
	for(;;)
	{
		macroTASK_DELAY_MS(macroWAIT_LOOP_TIME);
		uTimeout++;
		
		if(bSensor_Feedback == true)
			break;
		else if(uTimeout >= (macroSENSOR_FEEDBACK_TIMEOUT / macroWAIT_LOOP_TIME))
		{
			APP_DEBUG("--- SensorTask: read sensor time out.\r\n");
			break;
		}
	}
}

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		




    
