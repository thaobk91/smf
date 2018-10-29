
#include "Message.h"
#include "Parse.h"
#include "main.h"
#include "Control.h"
#include "FlashInternal.h"

extern uint8 uMain_TaskID; 
extern char IDZb[17];
extern uint8_t OutVAC_Status[macroMAX_OUTPUT_VAC];
extern char ZIGBEE_TX_Buffer[macroZIGBEE_TX_BUFF_SIZE];


void vMessage_isConfig( PacketIO *_PacketIO );
void vMessage_isRequest( PacketIO *_PacketIO );
void vMessage_isControl( PacketIO *_PacketIO );
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
		//get status
		vControl_get_OutVAC( OutVAC_Status );
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
			if( (_PacketIO->Data[0].Info[ui] == '1') || (_PacketIO->Data[0].Info[ui] == '0') )
			{
				OutVAC_Status[ui] = _PacketIO->Data[0].Info[ui] ^ 0x30;
			}
		}
		
		vControl_set_OutVAC( outVAC_ALL, OutVAC_Status );
		//get status
		vControl_get_OutVAC( OutVAC_Status );
		vFlashInternal_Write_OutVAC_State( OutVAC_Status );
		vMessage_sendOutVAC_Status();
	}
}




void vMessage_sendOutVAC_Status( void )
{
	PacketIO _PacketIO = {0};
	
	vControl_get_OutVAC( OutVAC_Status );
		
	strcpy(_PacketIO.TypePacket, macroTYPE_PACKET_DATA);
	strcpy(_PacketIO.TypeDevice, macroTYPE_DEVICE);
	strcpy(_PacketIO.IDDevice, IDZb);
	strcpy(_PacketIO.IDZb, IDZb);
	strcpy(_PacketIO.Data[0].Name, macroCONTROL_STATUS);
	sprintf(_PacketIO.Data[0].Info, "%d%d%d%d%d%d", OutVAC_Status[0], \
		OutVAC_Status[1], OutVAC_Status[2], OutVAC_Status[3], OutVAC_Status[4], OutVAC_Status[5]);
	vMessage_PackingMsg( &_PacketIO, ZIGBEE_TX_Buffer);
	macroSET_EVENT(uMain_TaskID, EVENT_SEND_MESSAGE);
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













