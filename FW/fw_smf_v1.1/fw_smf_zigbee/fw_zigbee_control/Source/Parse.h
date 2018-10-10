

#ifndef __PARSE_H
#define __PARSE_H


#define macroMSG_MAX_FIELD					18
#define macroMSG_MIN_FIELD					3
#define macroMSG_MAX_FIELD_LENGHT			24
#define macroMSG_MAX_LENGHT					(macroMSG_MAX_FIELD * macroMSG_MAX_FIELD_LENGHT + macroMSG_MAX_FIELD + 1)


typedef struct
{
	char	TypePacket[8];
	char	TypeDevice[12];
	char	IDDevice[17];
	char	IDZb[17];
	struct
	{
		char	Name[macroMSG_MAX_FIELD_LENGHT / 2];
		char	Info[macroMSG_MAX_FIELD_LENGHT];
	}Data[(macroMSG_MAX_FIELD - macroMSG_MIN_FIELD) / 2];
}PacketIO;



PacketIO xParse_getArrayData( char *pMessage );



#endif







