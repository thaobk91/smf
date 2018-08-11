
#include "SS_PH.h"



void vSS_PH_Init(void)
{
#ifdef USE_PH_ATLAS
	vSS_PH_Atlas_Init();
#endif
}
/******************************************************************************


*******************************************************************************/
void vSS_PH_Read( void )
{
#ifdef USE_PH_ATLAS
	vSS_PH_Atlas_Read();
#endif
}