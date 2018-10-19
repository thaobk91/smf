
#include "Message.h"
#include "Parse.h"
#include "main.h"

extern uint8 uMain_TaskID; 
extern char IDZb[17];
extern uint8_t OutVAC_Status[macroMAX_OUTPUT_VAC];
extern char ZIGBEE_TX_Buffer[macroZIGBEE_TX_BUFF_SIZE];


void vMessage_isConfig( PacketIO *_PacketIO );
void vMessage_isRequest( PacketIO *_PacketIO );
void vMessage_isControl( PacketIO *_PacketIO );
void vMessage_sendOutVAC_Status( void );
void vMessage_getOutVAC_Status( void );
void vMessage_PackingMsg( PacketIO *_PacketIO, char *pMessage );




void vMessage_Check( char *pMessage )
{
	PacketIO _PacketIO = xParse_getArrayData( pMessage );
	
	if(strcmp(_PacketIO.TypePacket, macroTYPE_PACKET_CONF) == 0)
	{
		APP_DEBUG("--- Message: message is config\r\n");
		vMessage_isConfig( &_PacketIO );
	}
	else if(strcmp(_PacketIO.TypePacket, macroTYPE_PACKET_REQS) == 0)
	{
		APP_DEBUG("--- Message: message is request\r\n");
		vMessage_isRequest( &_PacketIO );
	}
	else if(strcmp(_PacketIO.TypePacket, macroTYPE_PACKET_CTRL) == 0)
	{
		APP_DEBUG("--- Message: message is control\r\n");
		vMessage_isControl( &_PacketIO );
	}
	else
	{
		APP_DEBUG("--- Message: type packet message not found\r\n");
	}
}




void vMessage_isConfig( PacketIO *_PacketIO )
{
	
}




void vMessage_isRequest( PacketIO *_PacketIO )
{
	if( strcmp(_PacketIO->Data[0].Name, macroREQUEST_OUTVAC_STATUS) == 0 )
	{
		APP_DEBUG("--- Message: Request is ALL\r\n");
		vMessage_sendOutVAC_Status();
	}
}




void vMessage_isControl( PacketIO *_PacketIO )
{
	if( strcmp(_PacketIO->Data[0].Name, macroCONTROL_OUT_VAC) == 0 )
	{
		APP_DEBUG("--- Message: Control is outVAC\r\n");
		for(uint8_t ui = 0; ui < macroMAX_OUTPUT_VAC; ui++)
		{
			switch(ui)
			{
				case 0:
					if(_PacketIO->Data[0].Info[0] == '1')
						macroOUTPUT_1_ON();
					else if(_PacketIO->Data[0].Info[0] == '0')
						macroOUTPUT_1_OFF();
					break;
					
				case 1:
					if(_PacketIO->Data[0].Info[1] == '1')
						macroOUTPUT_2_ON();
					else if(_PacketIO->Data[0].Info[1] == '0')
						macroOUTPUT_2_OFF();
					break;
					
				case 2:
					if(_PacketIO->Data[0].Info[2] == '1')
						macroOUTPUT_3_ON();
					else if(_PacketIO->Data[0].Info[2] == '0')
						macroOUTPUT_3_OFF();
					break;
					
				case 3:
					if(_PacketIO->Data[0].Info[3] == '1')
						macroOUTPUT_4_ON();
					else if(_PacketIO->Data[0].Info[3] == '0')
						macroOUTPUT_4_OFF();
					break;
					
				case 4:
					if(_PacketIO->Data[0].Info[4] == '1')
						macroOUTPUT_5_ON();
					else if(_PacketIO->Data[0].Info[4] == '0')
						macroOUTPUT_5_OFF();
					break;
					
				case 5:
					if(_PacketIO->Data[0].Info[5] == '1')
						macroOUTPUT_6_ON();
					else if(_PacketIO->Data[0].Info[5] == '0')
						macroOUTPUT_6_OFF();
					break;
			}
		}
		//get status
		vMessage_getOutVAC_Status();
		vMessage_sendOutVAC_Status();
	}
}




void vMessage_sendOutVAC_Status( void )
{
	PacketIO _PacketIO = {0};
		
	strcpy(_PacketIO.TypePacket, macroTYPE_PACKET_DATA);
	strcpy(_PacketIO.TypeDevice, macroTYPE_DEVICE);
	strcpy(_PacketIO.IDDevice, IDZb);
	strcpy(_PacketIO.IDZb, IDZb);
	strcpy(_PacketIO.Data[0].Name, macroCONTROL_STATUS);
	for(uint8_t ui = 0; ui < macroMAX_OUTPUT_VAC; ui++)
		_PacketIO.Data[0].Info[ui] = OutVAC_Status[ui] | 0x30;
	vMessage_PackingMsg( &_PacketIO, ZIGBEE_TX_Buffer);
	macroSET_EVENT(uMain_TaskID, EVENT_SEND_MESSAGE);
}




void vMessage_getOutVAC_Status( void )
{
	uint32_t uValue;
	
	for(uint8_t ui = 0; ui < macroMAX_OUTPUT_VAC; ui++)
	{
		uValue = 0;
		switch(ui)
		{
			case 0:
				uValue = macroREAD_OUTPUT_1();
				OutVAC_Status[0] = (uValue != 0)? 1 : 0;
				break;
				
			case 1:
				uValue = macroREAD_OUTPUT_2();
				OutVAC_Status[1] = (uValue != 0)? 1 : 0;
				break;
				
			case 2:
				uValue = macroREAD_OUTPUT_3();
				OutVAC_Status[2] = (uValue != 0)? 1 : 0;
				break;
				
			case 3:
				uValue = macroREAD_OUTPUT_4();
				OutVAC_Status[3] = (uValue != 0)? 1 : 0;
				break;
				
			case 4:
				uValue = macroREAD_OUTPUT_5();
				OutVAC_Status[4] = (uValue != 0)? 1 : 0;
				break;
				
			case 5:
				uValue = macroREAD_OUTPUT_6();
				OutVAC_Status[5] = (uValue != 0)? 1 : 0;
				break;
		}
	}
}




void vMessage_PackingMsg( PacketIO *_PacketIO, char *pMessage )
{
	memset( pMessage, 0, strlen(pMessage) );
	
	sprintf( pMessage, "%s,%s,%s,%s", _PacketIO->TypePacket, macroTYPE_DEVICE, _PacketIO->IDDevice, _PacketIO->IDZb);
	
	for(uint8_t ui = 0; _PacketIO->Data[ui].Name[0] > 0; ui++)
	{
		sprintf(pMessage, "%s,%s", pMessage, _PacketIO->Data[ui].Name);
		if(_PacketIO->Data[ui].Info[0] > 0)
			sprintf(pMessage, "%s:%s", pMessage, _PacketIO->Data[ui].Info);
	}
	pMessage[strlen(pMessage)] = '!';
}













