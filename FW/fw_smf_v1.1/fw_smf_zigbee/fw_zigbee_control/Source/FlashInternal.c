
#include "hw_memmap.h"
#include "flash.h"

#include "main.h"
#include "FlashInternal.h"



int32_t iFlashInternal_Write(uint32_t uiAddr32BIT, uint8_t* pDataIn);
void vFlashInternal_Read(uint32_t uiAddr32BIT, uint8_t* pDataOut, uint8_t uiByteRead);
void vFlashInternal_EraseBlock(uint32_t uiAddr32Block, uint32_t size_block);
void vFlashInternal_ErasePage(uint32_t uiAddr32Page);





/******************************************************************************
 * Function		: int32_t iFlashInternal_Write(uint32_t uiAddr32BIT, uint32_t* pDataIn)
 * Description	: write data into flash
 * Param		: 
 *				+ uiAddr32BIT: addr write
 *				+ pDataIn: data need to write
 * Return		: // 0 = pass, -1 = fail, -2 = wrong param
******************************************************************************/
int32_t iFlashInternal_Write(uint32_t uiAddr32BIT, uint8_t* pDataIn)
{
	int32_t i32Res = 0;	// 0 = pass, -1 = fail, -2 = wrong param
	i32Res = FlashMainPageProgram((uint32_t *)pDataIn, uiAddr32BIT, sizeof(pDataIn));
	return i32Res;
}




/******************************************************************************
* Function		: void vFlashInternal_Read(uint32_t uiAddr32BIT, uint8_t* pDataOut, uint8_t uiByteRead)
 * Description	: read data from flash
 * Param		: 
 *				+ uiAddr32BIT: addr read
 *				+ pDataOut: buffer contain data read out
 *				+ uiByteRead: number of bytes is read, ui32Byte = 5 ~ 5bytes
 * Return		: none
******************************************************************************/
void vFlashInternal_Read(uint32_t uiAddr32BIT, uint8_t* pDataOut, uint8_t uiByteRead)
{
	for(uint32_t ui = 0; ui < uiByteRead; ui++)
	{
		uint32_t uValue = FlashGet(uiAddr32BIT + ui) & 0xff;
		APP_DEBUG("--- FlashInternal: value = %x", (uValue >> 24) & 0xFF);
		APP_DEBUG("%x", (uValue >> 16) & 0xFF);
		APP_DEBUG("%x", (uValue >> 8) & 0xFF);
		APP_DEBUG("%x\r\n", uValue & 0xFF);
		
		*(pDataOut + ui) = (uint8_t)uValue;
	}
	
}




/******************************************************************************
* Function		: void vFlashInternal_EraseBlock(uint32_t uiAddr32Block, uint32_t size_block)
 * Description	: erase data from flash
 * Param		: 
 *				+ uiAddr32Block: addr need erase
 *				+ size_block: size of block need del
 * Return		: none
******************************************************************************/
void vFlashInternal_EraseBlock(uint32_t uiAddr32Block, uint32_t size_block)
{
	char *buff = (char *) malloc( size_block * sizeof(char));
	memset(buff, 0xff, size_block);
	iFlashInternal_Write(uiAddr32Block, (uint8_t *)buff);
	free(buff);
}




/******************************************************************************
* Function		: void vFlashInternal_ErasePage(uint32_t uiAddr32Page)
 * Description	: erase all page
 * Param		: 
 *				+ uiAddr32Page: addr page
 * Return		: none

******************************************************************************/
void vFlashInternal_ErasePage(uint32_t uiAddr32Page)
{
	int32_t i32Res = FlashMainPageErase(uiAddr32Page);
	return;
}




/******************************************************************************
* Function		: void vFlashInternal_Write_OutVAC_State( uint8_t *pOutState )
 * Description	: write outVAC state
 * Param		: 
 *				+ pOutState: out VAC state
 * Return		: none

******************************************************************************/
void vFlashInternal_Write_OutVAC_State( uint8_t *pOutState )
{
	int32_t result;
	uint8_t uValue = 0;
	
	uValue = ((pOutState[0] << 5) & 0x20) | ((pOutState[1] << 4) & 0x10) | ((pOutState[2] << 3) & 0x08) | \
				((pOutState[3] << 2) & 0x04) | ((pOutState[4] << 1) & 0x02) | (pOutState[5] & 0x01);
	
	vFlashInternal_ErasePage(macroOUT_VAC_STATE_ADDR);
	SysCtrlDelay(3200000);

	if( result = iFlashInternal_Write( macroOUT_VAC_STATE_ADDR, (uint8_t *)&uValue ) == 0)
		APP_DEBUG("--- FlashInternal: write outVAC state successfully\r\n");
	else
		APP_DEBUG("--- FlashInternal: write outVAC state failture. result = %d\r\n", result);
	SysCtrlDelay(3200000);
}




/******************************************************************************
* Function		: void vFlashInternal_Read_OutVAC_State( uint8_t *pOutState )
 * Description	: read outVAC state
 * Param		: 
 *				+ pOutState: out VAC state
 * Return		: none

******************************************************************************/
void vFlashInternal_Read_OutVAC_State( uint8_t *pOutState )
{
	uint8_t uValue = 0;
	vFlashInternal_Read( macroOUT_VAC_STATE_ADDR, (uint8_t *)&uValue, 1);
	
	APP_DEBUG("--- FlashInternal: data read = %x\r\n", uValue);
	
	APP_DEBUG("--- FlashInternal: outVAC read = ");
	for(uint8_t ui = 0; ui < macroMAX_OUTPUT_VAC; ui++)
	{
		pOutState[ui] = (uValue >> (5 - ui)) & 0x01;
		APP_DEBUG("%d", pOutState[ui]);
	}
	APP_DEBUG("\r\n");
}

















