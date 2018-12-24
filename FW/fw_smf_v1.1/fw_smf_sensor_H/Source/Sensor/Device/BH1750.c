
#include "BH1750.h"
#include "main.h"
#include "UART.h"
#include "I2C.h"


//I2C
extern i2c_rtos_handle_t xMaster_Handle_BH1750;


/******************************************************************************/
static void vBH1750_WriteCmd(uint8_t cmd)
{
	uint8_t puData[2] = {0};
	uI2C_Write( &xMaster_Handle_BH1750, BH1750_Address_L, cmd, puData, 0);
}



/******************************************************************************/
void vBH1750_Begin(void)
{
	vBH1750_WriteCmd(BH1750_POWER_ON);
}



/******************************************************************************/
void vBH1750_SetMode(uint8_t Mode)
{
	vBH1750_WriteCmd(Mode);
}



/******************************************************************************/
void vBH1750_SetAddr(uint8_t Addr)
{
	vBH1750_WriteCmd(Addr);
}



/******************************************************************************/
void vBH1750_Init(void)
{
	macroGPIO_ACTIVE_LOW( macroBH1750_GPIO, macroBH1750_PIN );
	//init I2C
	vI2C_InitModule( &xMaster_Handle_BH1750, macroI2C_MASTER_BASE_BH1750, macroI2C_BAUDRATE_BH1750 );
}




/******************************************************************************/
uint16_t vBH1750_GetLux(void)
{
	uint8_t puData[2] = {0};
	uint16_t ui16Lux_val = 0;
	
	vBH1750_Begin();
	macroTASK_DELAY_MS( 50 );
	vBH1750_SetMode(BH1750_ONETIME_1LX_HRES);
	macroTASK_DELAY_MS( 180 );
	//read value
	uI2C_Read( &xMaster_Handle_BH1750, BH1750_Address_L, 0xFF, puData, 2);

	ui16Lux_val = (puData[0] <<8) | puData[1];
	ui16Lux_val = (uint16_t)(ui16Lux_val/1.2);
	
	return ui16Lux_val;
}


















