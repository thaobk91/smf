
#ifndef __FLASHINTERNAL_H
#define __FLASHINTERNAL_H


#define macroPAGE_SIZE               	2048	//byte
#define macroPAGE_ORDER            	 	254	// read datasheet before edit this define
#define macroPAGE_START_ADDR 			(FLASH_BASE + (macroPAGE_ORDER * macroPAGE_SIZE))

/*******************define for area memory on a page***************************/
//define address for calib sensor
#define macroOUT_VAC_STATE_ADDR			macroPAGE_START_ADDR



/******************************************************************************
* Function		: void vFlashInternal_Write_OutVAC_State( uint8_t *pOutState )
 * Description	: write outVAC state
 * Param		: 
 *				+ pOutState: out VAC state
 * Return		: none

******************************************************************************/
void vFlashInternal_Write_OutVAC_State( uint8_t *pOutState );




/******************************************************************************
* Function		: void vFlashInternal_Read_OutVAC_State( uint8_t *pOutState )
 * Description	: read outVAC state
 * Param		: 
 *				+ pOutState: out VAC state
 * Return		: none

******************************************************************************/
void vFlashInternal_Read_OutVAC_State( uint8_t *pOutState );


#endif




































