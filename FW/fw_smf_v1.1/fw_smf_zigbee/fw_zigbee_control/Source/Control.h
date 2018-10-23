
#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"


typedef enum
{
	outVAC_1 = 0x01,
	outVAC_2 = 0x02,
	outVAC_3 = 0x04,
	outVAC_4 = 0x08,
	outVAC_5 = 0x10,
	outVAC_6 = 0x20,
	outVAC_ALL = outVAC_1 | outVAC_2 | outVAC_3 | outVAC_4 | outVAC_5 | outVAC_6,
}OutVAC;


void vControl_set_OutVAC( OutVAC e_OutVAC, uint8_t *pState );


void vControl_get_OutVAC( uint8_t *pState );




#endif










