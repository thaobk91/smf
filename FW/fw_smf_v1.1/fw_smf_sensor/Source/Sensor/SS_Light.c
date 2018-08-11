	
#include "SS_Light.h"

#ifdef USE_BH1750_SS
	#include "BH1750.h"
#endif




void vSS_Light_Init( void )
{
#ifdef USE_BH1750_SS
	vBH1750_Init();
#endif
}




void vSS_Light_getLux( uint16_t *uLux ) 
{
#ifdef USE_BH1750_SS
	*uLux = vBH1750_GetLux();
#endif
}