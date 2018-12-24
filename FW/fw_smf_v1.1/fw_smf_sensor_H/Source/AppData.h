
#ifndef __APPDATA_H
#define __APPDATA_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/******************************************************************************
define for sensor user
******************************************************************************/



#define macroTIME_READ_SENSOR							30				//30s
#define macroTIME_SEND_WAMI								10				//10s
#define macroTIME_SEND_INTERVAL							300				//5mins

//Dinh nghia do dai cua ban tin String
#define macroPACKET_STRING_MAX_FIELD					17
#define macroPACKET_STRING_MIN_FIELD					3
#define macroPACKET_STRING_MAX_FIELD_LENGHT				20
#define macroPACKET_STRING_ENDCHAR						'!'
#define macroPACKET_STRING_MAX_LENGHT					(macroPACKET_STRING_MAX_FIELD * macroPACKET_STRING_MAX_FIELD_LENGHT + macroPACKET_STRING_MAX_FIELD)


#define macroTYPEDEVICE_SS								"SMF_SENSN"
#define macroTYPEDEVICE_GW								"SMF_GW"

//Type packet
#define macroTYPEPACKET_DATA							 "DATA"
#define macroTYPEPACKET_CONF							 "CONF"
#define macroTYPEPACKET_REQS							 "REQS"
#define macroTYPEPACKET_CTRL							 "CTRL"
#define macroTYPEPACKET_RESP							 "RESP"
#define macroTYPEPACKET_WAMI							 "WAMI"


//ID sensor
#define macroSENSOR_PH								 	"PH"
#define macroSENSOR_EC								 	"EC"
#define macroSENSOR_TEMPG								"TempG"
#define macroSENSOR_HUMIG								"HumiG"
#define macroSENSOR_TEMPA								"TempA"
#define macroSENSOR_HUMIA								"HumiA"
#define macroSENSOR_LIGHT								"Light"
#define macroSENSOR_D_STATE								"D_State"

//ID Data control
#define macroCTRL_STATUS								"CtrlStat"
#define macroDEV_STATUS									"D_State" 
#define macroOUT_VAC                                    "OutVAC"

//ID Reqs
#define macroREQUEST_ALL								"ALL"
#define macroREQUSET_RTC                            	"RTC"
#define macroREQUSET_D_STATE                           	"D_State"


//ID name Config
#define macroCFG_SENSOR_PH								"PH"
#define macroCFG_SENSOR_EC								"EC"
#define macroCFG_SENSOR_TEMPG							"TempG"
#define macroCFG_SENSOR_HUMIG							"HumiG"
#define macroCFG_SENSOR_TEMPA							"TempA"
#define macroCFG_SENSOR_HUMIA							"HumiA"
#define macroCFG_SENSOR_LIGHT							"Light"
#define macroCFG_DataUT                                 "DataUT"

#define macroCFG_TIMESEND 								30


//define for Thresh
#ifdef PH_SENSOR
	#define macroPH_THRESH_HIGH							14
	#define macroPH_THRESH_LOW							1
#endif

#ifdef EC_SENSOR
	#define macroEC_THRESH_HIGH 					 	2
	#define macroEC_THRESH_LOW							1
#endif

#ifdef TEMP_HUMI_AIR_SENSOR
	#define macroTEMPA_THRESH_HIGH						50
	#define macroTEMPA_THRESH_LOW						0

	#define macroHUMIA_THRESH_HIGH						100
	#define macroHUMIA_THRESH_LOW						5
#endif

#ifdef TEMP_HUMI_SOIL_SENSOR
	#define macroTEMPG_THRESH_HIGH						50
	#define macroTEMPG_THRESH_LOW						0

	#define macroHUMIG_THRESH_HIGH						100
	#define macroHUMIG_THRESH_LOW						0
#endif

#ifdef LIGHT_SENSOR
	#define macroLIGHT_THRESH_HIGH						65000
	#define macroLIGHT_THRESH_LOW						0
#endif

#define macroCFG_NO_CHANGE_THRESH						"x"

//Response
#define macroRESPONSE_OK								"OK"
#define macroRESPONSE_NOK								"NOK"



/******************************* Define connectivity **************************/
#define macroCONNECTIVITY_SSB					        "Zigb"
/******************************************************************************/

typedef struct
{
	bool			bSentIsOK;
	
	bool 			bConnectivityIsConnected;
}Flags;



//store data after read sensor
typedef struct
{
	bool	noSensor;
#ifdef PH_SENSOR
	float fpH;
#endif
#ifdef EC_SENSOR
	float fEC;
#endif
#ifdef TEMP_HUMI_SOIL_SENSOR
	float fTempG;
	float fHumiG;
#endif
#ifdef TEMP_HUMI_AIR_SENSOR
	float fTempA;
	float fHumiA;
#endif
#ifdef LIGHT_SENSOR
	uint16_t fLight;
#endif
}SS_Value;

#endif







