
#ifndef __SS_EC_H
#define __SS_EC_H


#define USE_EC_ATLAS

#ifdef USE_EC_ATLAS
	#include "SS_EC_Atlas.h"
#endif


void vSS_EC_Init( void );



void vSS_EC_Read( float fTemp );



#endif









