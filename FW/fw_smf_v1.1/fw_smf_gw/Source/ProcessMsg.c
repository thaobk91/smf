/*
 * ProcessMsg.c
 *
 *  Created on: Jul 16, 2018
 *      Author: ThaoHuyen
 */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "Parse.h"
#include "CheckMsg.h"
#include "main.h"
#include "ProcessMsg.h"
#include "UART.h"

#ifdef macroUSE_SDCARD
	#include "UserFile.h"
#endif

#include "RTC.h"


//Uart to Connectivity
extern uint8_t uUART_CONN_TX_Buffer[macroUART_TX_BUFFER_LENGHT];

//UART to network
extern uint8_t uUART_NWK_TX_Buffer[macroUART_TX_BUFFER_LENGHT];

extern WhoAmI _WhoAmI;

//response
extern bool isResponse;

extern rtc_datetime_t _RTC;

//ID Gw
extern uint8_t uIDGw[17];

extern uint8_t uSmartConfig_Counter;


bool bWaitNWK_Connected = false;



/********************************* Function ***********************************/
static void vProcessMsg_DATA(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity);
static void vProcessMsg_CONF(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity);
static void vProcessMsg_REQS(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity);
static void vProcessMsg_CTRL(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity);
static void vProcessMsg_RESP(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity);
static void vProcessMsg_WAMI(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity);
/******************************************************************************/




/******************************************************************************
 * Function		: void vProcessMsg_AllMessage( uint8_t *pDataIn, bool isStringMessage, bool fromConnectivity )
 * Description	: process all message
 * Param		: None
 * Return		: None
******************************************************************************/
void vProcessMsg_AllMessage( uint8_t *pDataIn, bool isStringMessage, bool fromConnectivity )
{
	PacketIO _PacketIO = {0};

	if(isStringMessage == true)
	{
		_PacketIO = xParseMsg_MessageIsString( pDataIn, fromConnectivity );
		if(_PacketIO.TypePacket[0] > 0)
			vCheckMsg_StringMessage( &_PacketIO );
	}
	else
	{
		_PacketIO = xParseMsg_MessageIsJson( pDataIn );
		if(_PacketIO.TypePacket[0] > 0)
			vCheckMsg_JsonMessage( &_PacketIO );
	}

	memset(pDataIn, 0, strlen((char *)pDataIn) + 1);

	if(_PacketIO.bPacketIsOK == false)
	{
		if(fromConnectivity == true)
			vProcessMsg_Send_Response( macroRESP_NOK, true );
		APP_DEBUG("--- ProcessMsg: Message is not found\r\n");
		return;
	}

	if( strcmp((char *)(char *)_PacketIO.TypePacket, macroTYPE_PACKET_DATA) == 0 )
	{
		APP_DEBUG("--- ProcessMsg: message is DATA\r\n");
		vProcessMsg_DATA( &_PacketIO, isStringMessage, fromConnectivity );
	}
	else if( strcmp((char *)(char *)_PacketIO.TypePacket, macroTYPE_PACKET_CONF) == 0 )
	{
		APP_DEBUG("--- ProcessMsg: message is CONF\r\n");
		vProcessMsg_CONF( &_PacketIO, isStringMessage, fromConnectivity );
	}
	else if( strcmp((char *)(char *)_PacketIO.TypePacket, macroTYPE_PACKET_REQS) == 0 )
	{
		APP_DEBUG("--- ProcessMsg: message is REQS\r\n");
		vProcessMsg_REQS( &_PacketIO, isStringMessage, fromConnectivity );
	}
	else if( strcmp((char *)(char *)_PacketIO.TypePacket, macroTYPE_PACKET_CTRL) == 0 )
	{
		APP_DEBUG("--- ProcessMsg: message is CTRL\r\n");
		vProcessMsg_CTRL( &_PacketIO, isStringMessage, fromConnectivity );
	}
	else if( strcmp((char *)(char *)_PacketIO.TypePacket, macroTYPE_PACKET_RESP) == 0 )
	{
		APP_DEBUG("--- ProcessMsg: message is RESP\r\n");
		vProcessMsg_RESP( &_PacketIO, isStringMessage, fromConnectivity );
	}
	else if( strcmp((char *)(char *)_PacketIO.TypePacket, macroTYPE_PACKET_WAMI) == 0 )
	{
		APP_DEBUG("--- ProcessMsg: message is WAMI\r\n");
		vProcessMsg_WAMI( &_PacketIO, isStringMessage, fromConnectivity );
	}
}





/******************************************************************************
 * Function		: void vProcessMsg_DATA(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
 * Description	: process message DATA
 * Param		: None
 * Return		: None
******************************************************************************/
static void vProcessMsg_DATA(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
{
	if(isStringMessage == false)
		return;

	for(uint8_t ui = 0; ;ui++)
	{
		if(_PacketIO->Data[ui].Name[0] == 0)
		{
			vRTC_GetDateTime(&_RTC);
			sprintf((char *)_PacketIO->Data[ui].Name, "%s", macroID_DATA_TIME);
			sprintf((char *)_PacketIO->Data[ui].Info, "%d-%d-%dT%d:%d:%d", \
							_RTC.day, _RTC.month, _RTC.year, _RTC.hour, _RTC.minute, _RTC.second);
			break;
		}
	}

	if(fromConnectivity == true)
	{
		isResponse = false;
		vParseMsg_Packing_StringMessage(_PacketIO, uUART_NWK_TX_Buffer, false);
		
		if( (_WhoAmI.Network.Wifi == true) || (_WhoAmI.Network._2G == true) || (_WhoAmI.Network._3G == true) )
		{
			vMain_setEvent(EVENT_UART_NWK_SEND);
		}
		else
		{
			APP_DEBUG("--- ProcessMsg: Lost all nwk connection. waiting nwk connected\r\n");
			bWaitNWK_Connected = true;
		}
		vProcessMsg_Send_Response(macroRESP_OK, true);
	}
	else
	{
		APP_DEBUG("--- ProcessMsg: DATA don't from Connectivity\r\n");
	}
}




/******************************************************************************
 * Function		: void vProcessMsg_CONF(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
 * Description	: process message CONF
 * Param		: None
 * Return		: None
******************************************************************************/
static void vProcessMsg_CONF(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
{
	if(fromConnectivity == false)
	{
		if( strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_RTC) == 0)
		{
			APP_DEBUG("--- ProcessMsg: CONF is RTC\r\n");

			#define macroTIME_LENGHT		5
			uint8_t Array[6][macroTIME_LENGHT] = {0};
			uint8_t ui = 0, uj = 0, uk = 0;

			while(_PacketIO->Data[0].Info[ui] > 0)
			{
				if(_PacketIO->Data[0].Info[ui] == '/')
				{
					uj++;
					uk = 0;
				}
				else
				{
					Array[uj][uk++] = _PacketIO->Data[0].Info[ui];
				}
				if(uk >= macroTIME_LENGHT)
					break;
				ui++;
			}

			signed Hour 	= (Array[0][0] > 0)? atoi((char *)Array[0]) : 0xFF;
			signed Minute 	= (Array[1][0] > 0)? atoi((char *)Array[1]) : 0xFF;
			signed Second 	= (Array[2][0] > 0)? atoi((char *)Array[2]) : 0xFF;
			signed Day 		= (Array[3][0] > 0)? atoi((char *)Array[3]) : 0xFF;
			signed Month 	= (Array[4][0] > 0)? atoi((char *)Array[4]) : 0xFF;
			signed Year 	= (Array[5][0] > 0)? atoi((char *)Array[5]) : 0xFF;

			if( (Hour == 0xFF) || (Minute == 0xFF) || (Second == 0xFF) || (Day == 0xFF) || (Month == 0xFF) || (Year == 0xFF) || \
				(Hour < 0 || Hour > 23) || (Minute < 0 || Minute > 60) || (Second < 0 || Second > 60)  || (Day < 0 || Day > 31) || \
				(Month < 0 || Month > 12) || (Year < 0 || Year > 99) \
				)
			{
				APP_DEBUG("--- ProcessMsg: Time not found\r\n");
				return;
			}
			
			_RTC.hour 	= Hour;
			_RTC.minute = Minute;
			_RTC.second = Second;
			_RTC.day 	= Day;
			_RTC.month 	= Month;
			_RTC.year 	= 2000 + Year;

			APP_DEBUG("--- ProcessMsg: set Time = %d:%d:%d - %d/%d/%d\r\n", _RTC.hour, _RTC.minute, _RTC.second, _RTC.day, _RTC.month, _RTC.year);
			vRTC_SetDateTime(&_RTC);
			macroTASK_DELAY_MS( 100 );
		#ifdef macroUSE_SDCARD
			vUserFile_Write_RTC( _RTC.hour, _RTC.minute, _RTC.second, _RTC.day, _RTC.month, _RTC.year );
		#endif
			vMain_setEvent(EVENT_RTC_CONFIG);
		}

		else if( strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_SMC_WIFI) == 0)
		{
			APP_DEBUG("--- ProcessMsg: CONF is Smart config wifi\r\n");
			uSmartConfig_Counter = 1;
		}
		else if( (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_THRESHOLD_PH) == 0) || (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_THRESHOLD_EC) == 0) || \
				 (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_THRESHOLD_TEMP_G) == 0) || (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_THRESHOLD_HUMI_G) == 0) || \
				 (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_THRESHOLD_TEMP_A) == 0) || (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_THRESHOLD_HUMI_A) == 0) || \
				 (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_THRESHOLD_LIGHT) == 0) \
				 )
		{
			APP_DEBUG("--- ProcessMsg: CONF is Sensor Threshold\r\n");
			vParseMsg_Packing_StringMessage(_PacketIO, uUART_CONN_TX_Buffer, true);
			vMain_setEvent(EVENT_UART_CONN_SEND);
		}
		else if( (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_AUTO_OUTVAC_1) == 0) || (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_AUTO_OUTVAC_2) == 0) || \
				 (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_AUTO_OUTVAC_3) == 0) || (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_AUTO_OUTVAC_4) == 0) || \
				 (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_AUTO_OUTVAC_5) == 0) || (strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_AUTO_OUTVAC_6) == 0) \
				)
		{
			APP_DEBUG("--- ProcessMsg: CONF is auto VAC\r\n");
			vParseMsg_Packing_StringMessage(_PacketIO, uUART_CONN_TX_Buffer, true);
			vMain_setEvent(EVENT_UART_CONN_SEND);
		}
		else if( strcmp((char *)_PacketIO->Data[0].Name, macroID_CONF_DATA_UPDATE_TIME) == 0 )
		{
			APP_DEBUG("--- ProcessMsg: CONF is Data Update time\r\n");
			vParseMsg_Packing_StringMessage(_PacketIO, uUART_CONN_TX_Buffer, true);
			vMain_setEvent(EVENT_UART_CONN_SEND);
		}
		else
		{
			APP_DEBUG("--- ProcessMsg: CONF is none\r\n");
		}
	}
	else
	{
		APP_DEBUG("--- ProcessMsg: CONF don't from Network\r\n");
	}
}




/******************************************************************************
 * Function		: void vProcessMsg_REQS(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
 * Description	: process message REQS
 * Param		: None
 * Return		: None
******************************************************************************/
static void vProcessMsg_REQS(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
{
	if(fromConnectivity == false)
	{
		if( strcmp((char *)_PacketIO->Data[0].Name, macroID_REQS_ALL) == 0 )
		{
			APP_DEBUG("--- ProcessMsg: REQS is ALL\r\n");
			vParseMsg_Packing_StringMessage(_PacketIO, uUART_CONN_TX_Buffer, true);
			vMain_setEvent(EVENT_UART_CONN_SEND);
		}
		else if( strcmp((char *)_PacketIO->Data[0].Name, macroID_REQS_IDGW) == 0 )
		{
			uint8_t uBuff[macroUART_TX_BUFFER_LENGHT] = {0};
			
			APP_DEBUG("--- ProcessMsg: REQS is IDGw\r\n");
			strcpy((char *)_PacketIO->TypePacket, macroTYPE_PACKET_CONF);
			strcpy((char *)_PacketIO->Data[0].Name, macroID_CONF_IDGW);
			strcpy((char *)_PacketIO->Data[0].Info, (char *)uIDGw);
			if(isStringMessage ==  true)
			{
				vParseMsg_Packing_StringMessage(_PacketIO, uBuff, false);
				UART_NWK_WRITE_DATA(uBuff);
			}
		}
		else if( strcmp((char *)_PacketIO->Data[0].Name, macroID_REQS_D_STATE) == 0 )
		{
			if( (strcmp((char *)_PacketIO->IDEd, (char *)uIDGw) == 0) || (_PacketIO->IDEd[0] == 0) )
			{	
				APP_DEBUG("--- ProcessMsg: REQS is gw device state\r\n");
				vProcessMsg_Send_Data( NULL, macroID_DATA_DEVICE_STATE, macroRESP_OK );
			}
			else
			{
				APP_DEBUG("--- ProcessMsg: REQS is end device state\r\n");
				vParseMsg_Packing_StringMessage(_PacketIO, uUART_CONN_TX_Buffer, true);
				vMain_setEvent(EVENT_UART_CONN_SEND);
			}
		}
		else
		{
			APP_DEBUG("--- ProcessMsg: REQS is none\r\n");
		}
	}
	else
	{
		APP_DEBUG("--- ProcessMsg: REQS don't from Network\r\n");
	}
}




/******************************************************************************
 * Function		: void vProcessMsg_CTRL(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
 * Description	: process message CTRL
 * Param		: None
 * Return		: None
******************************************************************************/
static void vProcessMsg_CTRL(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
{
	APP_DEBUG("--- ProcessMsg: CTRL is none\r\n");
	if( strcmp((char *)_PacketIO->Data[0].Name, macroCTRL_OUTPUT_VAC) == 0 )
	{
		APP_DEBUG("--- ProcessMsg: CTRL is Out VAC\r\n");
		vParseMsg_Packing_StringMessage(_PacketIO, uUART_CONN_TX_Buffer, true);
		vMain_setEvent(EVENT_UART_CONN_SEND);
	}
}




/******************************************************************************
 * Function		: void vProcessMsg_CONF(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
 * Description	: process message CONF
 * Param		: None
 * Return		: None
******************************************************************************/
static void vProcessMsg_RESP(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
{
	if(fromConnectivity == false)
	{
		if(strcmp((char *)_PacketIO->Data[0].Name, macroRESP_OK) == 0)
			isResponse = true;
		else
			isResponse = false;
	}
	//vMain_setEvent(EVENT_RESP);
}




/******************************************************************************
 * Function		: void vProcessMsg_CONF(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
 * Description	: process message CONF
 * Param		: None
 * Return		: None
******************************************************************************/
static void vProcessMsg_WAMI(PacketIO *_PacketIO, bool isStringMessage, bool fromConnectivity)
{
	APP_DEBUG("--- ProcessMsg: WAMI is %s\r\n", (char *)_PacketIO->Data[0].Name);
	if(fromConnectivity == false)
	{
#ifdef macroCONNECTIVITY_WIFI
		if(strcmp((char *)_PacketIO->Data[0].Name, macroCONNECTIVITY_WIFI) == 0)
		{
			_WhoAmI.Network.Wifi = true;
			APP_DEBUG("--- ProcessMsg: Wifi is running\r\n");
		}
		else 
#endif	
#ifdef macroCONNECTIVITY_2G
		if(strcmp((char *)_PacketIO->Data[0].Name, macroCONNECTIVITY_2G) == 0)
		{
			_WhoAmI.Network._2G = true;
			APP_DEBUG("--- ProcessMsg: 2G is running\r\n");
		}
		else 
#endif	
#ifdef macroCONNECTIVITY_3G
		if(strcmp((char *)_PacketIO->Data[0].Name, macroCONNECTIVITY_3G) == 0)
		{
			_WhoAmI.Network._3G = true;
			APP_DEBUG("--- ProcessMsg: 3G is running\r\n");
		}
		else
#endif
		{
			APP_DEBUG("--- ProcessMsg: WAMI network is not running\r\n");
			return;
		}

		if(strcmp((char *)_PacketIO->Data[0].Info, macroRESP_OK) == 0)
		{
			APP_DEBUG("--- ProcessMsg: nwk is connected\r\n");
			_WhoAmI.Network.ConnectionOK = true;
		}
		else
		{
			APP_DEBUG("--- ProcessMsg: nwk is disconnected\r\n");
		}
	}
	else
	{
#ifdef macroCONNECTIVITY_ZIGB
		if(strcmp((char *)_PacketIO->Data[0].Name, macroCONNECTIVITY_ZIGB) == 0)
			_WhoAmI.Connectivity.Zigbee = true;
		else
#endif
#ifdef macroCONNECTIVITY_SUB1G
		if(strcmp((char *)_PacketIO->Data[0].Name, macroCONNECTIVITY_SUB1G) == 0)
			_WhoAmI.Connectivity.Sub1Ghz = true;
		else
#endif
		{
			APP_DEBUG("--- ProcessMsg: WAMI connectivity is none\r\n");
			return;
		}
	}

	if(bWaitNWK_Connected == true)
		vMain_setEvent(EVENT_WAIT_NWK_CONNECTED);
	bWaitNWK_Connected = false;
}




/******************************************************************************
 * Function		: void vProcessMsg_Send_Data( char *IDEp, char *ID_Data, bool State )
 * Description	: send data
 * Param		: None
 * Return		: None
******************************************************************************/
void vProcessMsg_Send_Data( char *IDEp, char *ID_Data, char *State )
{
	PacketIO _PacketIO = {0};

	strcpy((char *)_PacketIO.TypePacket, macroTYPE_PACKET_DATA);
	strcpy((char *)_PacketIO.TypeDevice, macroAPP_TYPE_DEVICE_GW);

	if(IDEp != NULL)
		strcpy((char *)_PacketIO.IDEd, IDEp);
	else
		strcpy((char *)_PacketIO.IDEd, (char *)uIDGw);
	strcpy((char *)_PacketIO.Data[0].Name, ID_Data);

	if(State != NULL)
		strcpy((char *)_PacketIO.Data[0].Info, State);
	
	sprintf((char *)_PacketIO.Data[1].Name, "%s", macroID_DATA_TIME);
	sprintf((char *)_PacketIO.Data[1].Info, "%d-%d-%dT%d:%d:%d", _RTC.day, _RTC.month, _RTC.year, _RTC.hour, _RTC.minute, _RTC.second);

	if( (_WhoAmI.Network.Wifi == true) || (_WhoAmI.Network._2G == true) || (_WhoAmI.Network._3G == true) )
	{
		vParseMsg_Packing_StringMessage(&_PacketIO, uUART_NWK_TX_Buffer, false);
		vMain_setEvent(EVENT_UART_NWK_SEND);
	}
}




/******************************************************************************
 * Function		: void vProcessMsg_Send_Request( bool toConnectivity )
 * Description	: send request
 * Param		: None
 * Return		: None
******************************************************************************/
void vProcessMsg_Send_Request( char *Request, bool toConnectivity )
{
	if(toConnectivity == false)
	{
		PacketIO _PacketIO = {0};
		uint8_t uDataOut[128] = {0};

		strcpy((char *)_PacketIO.TypePacket, macroTYPE_PACKET_REQS);
		strcpy((char *)_PacketIO.TypeDevice, macroAPP_TYPE_DEVICE_GW);
		strcpy((char *)_PacketIO.IDGw, (char *)uIDGw);
		strcpy((char *)_PacketIO.Data[0].Name, Request);

		if( (_WhoAmI.Network.Wifi == true) || (_WhoAmI.Network._2G == true) || (_WhoAmI.Network._3G == true) )
		{
			vParseMsg_Packing_StringMessage(&_PacketIO, uDataOut, toConnectivity);
//			vMain_setEvent(EVENT_UART_CONN_SEND);
			UART_NWK_WRITE_DATA(uDataOut);
		}
	}
}





/******************************************************************************
 * Function		: void vProcessMsg_Send_Response( char *Response, bool toConnectivity )
 * Description	: send response
 * Param		: None
 * Return		: None
******************************************************************************/
void vProcessMsg_Send_Response( char *Response, bool toConnectivity )
{
	if(toConnectivity == true)
	{
//		PacketIO _PacketIO = {0};
		uint8_t uBuff[128] = {0};

		sprintf((char *)uBuff, "%s,%s,%s!", macroTYPE_PACKET_RESP, macroAPP_TYPE_DEVICE_GW, Response);
		UART_CONN_WRITE_DATA(uBuff);
//		strcpy((char *)_PacketIO.TypePacket, macroTYPE_PACKET_RESP);
//		strcpy((char *)_PacketIO.TypeDevice, macroAPP_TYPE_DEVICE_GW);
//		strcpy((char *)_PacketIO.Data[0].Name, macroRESP_OK);
//		vParseMsg_Packing_StringMessage(&_PacketIO, uUART_CONN_TX_Buffer, toConnectivity);
//		vMain_setEvent(EVENT_UART_CONN_SEND);
	}
}




/******************************************************************************
 * Function		: void vProcessMsg_Send_Wami( void )
 * Description	: send who am i
 * Param		: None
 * Return		: None
******************************************************************************/
void vProcessMsg_Send_Wami( void )
{
	UART_NWK_WRITE_DATA("WAMI!");
	UART_CONN_WRITE_DATA("WAMI!");
}






























