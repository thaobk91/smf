/*
 * UserFile.c
 *
 *  Created on: Jul 20, 2018
 *      Author: ThaoHuyen
 */
#include <stdio.h>

#include "UserFile.h"
#include "main.h"




/******************************************************************************/
bool bUserFile_ReadFile(char* filename, float* fThresh_Low, float* fThresh_High)
{
	//read file
#define macroMAX_FIELD				3
#define macroMAX_FIELD_LENGHT		48
	char Buffer[macroMAX_FIELD * macroMAX_FIELD_LENGHT + 1] = {0};
	char Array[macroMAX_FIELD][macroMAX_FIELD_LENGHT] = {0};
	uint8_t ui = 0, uj = 0, uk = 0;
	
	if(bFileAccess_Read_All(filename, Buffer) == false)
	{
		APP_DEBUG("--- UserFile: Read config failture\r\n");
		return false;
	}
	
	for(ui = 0; Buffer[ui] > 0; ui++)
	{
		if(Buffer[ui] == '\n')
		{
			uj++;
			uk = 0;
		}
		else if(Buffer[ui] == '\r')
			continue;
		else
		{
			Array[uj][uk++] = Buffer[ui];
		}
	}
	
	//APP_DEBUG("--- UserFile: Read file \"%s\", data = %s - %s\r\n", filename, Array[0], Array[1]);
    
    if(*fThresh_High != -1)
      *fThresh_High= atof(Array[1]);
    
    *fThresh_Low = atof(Array[0]);
    
	return true;
}


/******************************************************************************/
bool bUserFile_WriteFile( char* filename, float fThresh_Low, float fThresh_High )
{
	char cStr[256] = {0};

    if(fThresh_High == -1)
        sprintf(cStr, "%f\r\n", fThresh_Low);
    else
        sprintf(cStr, "%f\r\n%f\r\n", fThresh_Low, fThresh_High);

	if(bFileAccess_Write_All(filename, cStr) == false)
	{
		APP_DEBUG("--- UserFile: Write parameter config error\r\n");
		return false;
	}
	return true;
}


/******************************************************************************/
bool bUserFile_ReadAll(DataLocal* xDataLocalIO)
{
  float iX = -1;
  float fDataUT = 0;
  
	if(bUserFile_ReadFile(macroUSER_FILE_TIMESEND, &fDataUT, &iX) == false)
	{
		APP_DEBUG("--- UserFile: Read config failture\r\n");
		return false;
	}

	xDataLocalIO->uiTimeUpdate = (uint16_t)fDataUT;
	APP_DEBUG("--- UserFile: Data update time = %d\r\n", xDataLocalIO->uiTimeUpdate);
	  
#ifdef PH_SENSOR
    if(bUserFile_ReadFile(macroUSER_FILE_PH, &xDataLocalIO->xPH.ui16LowThresh, (float*)&xDataLocalIO->xPH.ui16HighThresh) == false)
    {
        APP_DEBUG("--- UserFile: Read config failture\r\n");
        return false;
    }
	APP_DEBUG("--- UserFile: pH Threshold = L:%.3f - H:%.3f\r\n", xDataLocalIO->xPH.ui16LowThresh, xDataLocalIO->xPH.ui16HighThresh);
#endif
    
#ifdef EC_SENSOR
    if(bUserFile_ReadFile(macroUSER_FILE_EC, &xDataLocalIO->xEC.ui16LowThresh, &xDataLocalIO->xEC.ui16HighThresh) == false)
    {
        APP_DEBUG("--- UserFile: Read config failture\r\n");
        return false;
    }
	APP_DEBUG("--- UserFile: EC Threshold = L:.3%f - H:%.3f\r\n", xDataLocalIO->xEC.ui16LowThresh, xDataLocalIO->xEC.ui16HighThresh);
#endif
    
#ifdef TEMP_HUMI_AIR_SENSOR
    if(bUserFile_ReadFile(macroUSER_FILE_TEMPA, &xDataLocalIO->xTempA.ui16LowThresh, &xDataLocalIO->xTempA.ui16HighThresh) == false)
    {
        APP_DEBUG("--- UserFile: Read config failture\r\n");
        return false;
    }
    APP_DEBUG("--- UserFile: TempA Threshold = L:%.3f - H:%.3f\r\n", xDataLocalIO->xTempA.ui16LowThresh, xDataLocalIO->xTempA.ui16HighThresh);
	
    if(bUserFile_ReadFile(macroUSER_FILE_HUMIA, &xDataLocalIO->xHumiA.ui16LowThresh, &xDataLocalIO->xHumiA.ui16HighThresh) == false)
    {
        APP_DEBUG("--- UserFile: Read config failture\r\n");
        return false;
    }
	APP_DEBUG("--- UserFile: HumiA Threshold = L:%.3f - H:%.3f\r\n", xDataLocalIO->xHumiA.ui16LowThresh, xDataLocalIO->xHumiA.ui16HighThresh);
#endif
    
#ifdef TEMP_HUMI_SOIL_SENSOR
    if(bUserFile_ReadFile(macroUSER_FILE_TEMPG, &xDataLocalIO->xTempG.ui16LowThresh, &xDataLocalIO->xTempG.ui16HighThresh) == false)
    {
        APP_DEBUG("--- UserFile: Read config failture\r\n");
        return false;
    }
	APP_DEBUG("--- UserFile: TempG Threshold = L:%.3f - H:%.3f\r\n", xDataLocalIO->xTempG.ui16LowThresh, xDataLocalIO->xTempG.ui16HighThresh);
    
    if(bUserFile_ReadFile(macroUSER_FILE_HUMIG, &xDataLocalIO->xHumiG.ui16LowThresh, &xDataLocalIO->xHumiG.ui16HighThresh) == false)
    {
        APP_DEBUG("--- UserFile: Read config failture\r\n");
        return false;
    }
	APP_DEBUG("--- UserFile: HumiG Threshold = L:%.3f - H:%.3f\r\n", xDataLocalIO->xHumiG.ui16LowThresh, xDataLocalIO->xHumiG.ui16HighThresh);
#endif

#ifdef LIGHT_SENSOR
    if(bUserFile_ReadFile(macroUSER_FILE_LIGHT, &xDataLocalIO->xLight.ui16LowThresh, &xDataLocalIO->xLight.ui16HighThresh) == false)
    {
        APP_DEBUG("--- UserFile: Read config failture\r\n");
        return false;
    }
	APP_DEBUG("--- UserFile: Light Threshold = L:%.3f - H:%.3f\r\n", xDataLocalIO->xLight.ui16LowThresh, xDataLocalIO->xLight.ui16HighThresh);
#endif

    
	return true;
}










