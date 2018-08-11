
#include "SS_EC.h"
#include "UART.h"
#include "main.h"


void vSS_EC_Init( void )
{
#ifdef USE_EC_ATLAS
	vSS_EC_Atlas_Init();
#endif
}



void vSS_EC_Read(void)
{
#ifdef USE_EC_ATLAS
	vSS_EC_Atlas_Read();
#endif
}
