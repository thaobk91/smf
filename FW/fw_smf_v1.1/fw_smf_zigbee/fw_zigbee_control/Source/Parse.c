
#include "Parse.h"
#include "main.h"


typedef struct
{
	char Array[macroMSG_MAX_FIELD][macroMSG_MAX_FIELD_LENGHT];
}Array2D;


Array2D xParse_SplitMessage( char *pMessage );
void vParse_Coppy(char *Dst, char *Src, uint8_t NBCharacterBeginSplit);




Array2D xParse_SplitMessage( char *pMessage )
{
	Array2D _Array2D = {0};
	uint8_t ui = 0, uk = 0;
	
	while( *pMessage > 0 )
	{
		if( (*pMessage == ',') || (*pMessage == ':') )
		{
			ui++;
			uk = 0;
			if(*pMessage == ':')
				_Array2D.Array[ui][0] = ':';
		}
		else if(*pMessage == '!')
		{
			break;
		}
		else
		{
			_Array2D.Array[ui][uk++] = *pMessage;
		}
		
		if((ui > macroMSG_MAX_FIELD) || (uk >= macroMSG_MAX_FIELD_LENGHT))
		{
			APP_DEBUG("--- Parse: max field or max field lenght\r\n");
			memset(_Array2D.Array, 0, sizeof(_Array2D.Array));
			return _Array2D;
		}
		
		pMessage++;
	}
	
	return _Array2D;
}




PacketIO xParse_getArrayData( char *pMessage )
{
	uint8_t ui = 0;
	PacketIO _PacketIO = {0};
	Array2D _Array2D = xParse_SplitMessage( pMessage );
	
	vParse_Coppy( _PacketIO.TypePacket, _Array2D.Array[ui++], 0);
	vParse_Coppy( _PacketIO.TypeDevice, _Array2D.Array[ui++], 0);
	vParse_Coppy( _PacketIO.IDDevice, _Array2D.Array[ui++], 0);
	
	for(;;)
	{
		if(_Array2D.Array[ui][0] > 0)
			vParse_Coppy( _PacketIO.TypePacket, _Array2D.Array[ui++], 0);
		else
			break;
		
		if(_Array2D.Array[ui + 1][0] == ':')
			vParse_Coppy( _PacketIO.TypePacket, _Array2D.Array[ui++], 1);
	}
	
	return _PacketIO;
}




/******************************************************************************
 * Function		: void vParse_Coppy(char *Dst, char *Src, uint8_t NBCharacterBeginSplit)
 * Description	: Ham coppy 1 chuoi
 * Param		:
*				+ Dst: chuoi dich
*				+ Src: chuoi nguon
*				+ NBCharacterBeginSplit: so ky tu bo qua tinh tu dau
 * Return		: none
*******************************************************************************/
void vParse_Coppy(char *Dst, char *Src, uint8_t NBCharacterBeginSplit)
{
	memset(Dst, 0, strlen((char *)Dst));
    Src += NBCharacterBeginSplit;
    while(*Src > 0)
    {
        *Dst++ = *Src++;
    }
}

















