
#include "SHT1x.h"


/* Define of Sensor */
#define macroTEMP_READ_CMD  			0x03
#define macroHUMI_READ_CMD    			0x05

#define D1      						-39.7
#define D2      						0.01

#define C1      						-4.0
#define C2       						0.0405
#define C3       						-0.0000028
#define T1       						0.01
#define T2       						0.00008


/********************************* SHT1x function *****************************/
static void vSHT1x_DataDir_Out( SHT1x_IO *_SHT_IO );
static void vSHT1x_DataDir_In( SHT1x_IO *_SHT_IO );
static void vSHT1x_ClkDir_Out( SHT1x_IO *_SHT_IO );
static void vSHT1x_ClkDir_In( SHT1x_IO *_SHT_IO );

static void vSHT1x_DataOut_High( SHT1x_IO *_SHT_IO );
static void vSHT1x_DataOut_Low( SHT1x_IO *_SHT_IO );
static void vSHT1x_ClkOut_High( SHT1x_IO *_SHT_IO );
static void vSHT1x_ClkOut_Low( SHT1x_IO *_SHT_IO );

static uint32_t uSHT1x_Data_In( SHT1x_IO *_SHT_IO );
/******************************************************************************/



//for SHT1x air temp&humi index
SHT1x_Func _SHT1x_Func_Index =
{
	.DATA_DIR_OUT		= vSHT1x_DataDir_Out,
	.DATA_DIR_IN		= vSHT1x_DataDir_In,
	.CLK_DIR_OUT		= vSHT1x_ClkDir_Out,
	.CLK_DIR_IN			= vSHT1x_ClkDir_In,
	
	.DATA_OUT_HIGH		= vSHT1x_DataOut_High,
	.DATA_OUT_LOW		= vSHT1x_DataOut_Low,
	.CLK_OUT_HIGH		= vSHT1x_ClkOut_High,
	.CLK_OUT_LOW		= vSHT1x_ClkOut_Low,
	
	.DATA_IN			= uSHT1x_Data_In,
};




static int iSHT1x_ReadRaw( uint8_t cmd, SHT1x_IO *_SHT_IO )
{
	int _val = 0;
	uint8_t i;
	uint32_t cnt = 0;

	// Transmission Start
	_SHT1x_Func_Index.DATA_DIR_OUT( _SHT_IO );
	_SHT1x_Func_Index.CLK_DIR_OUT( _SHT_IO );
	_SHT1x_Func_Index.DATA_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.DATA_OUT_LOW( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_LOW( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.DATA_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_LOW( _SHT_IO );

	// The command (3 msb are address and must be 000, and last 5 bits are command)
	for (i = 0; i < 8; i++)
	{
		if ( !(cmd & (1 << (7 - i))) ) 
			_SHT1x_Func_Index.DATA_OUT_LOW( _SHT_IO );
		else    
			_SHT1x_Func_Index.DATA_OUT_HIGH( _SHT_IO );
		
		_SHT1x_Func_Index.CLK_OUT_HIGH( _SHT_IO );
		_SHT1x_Func_Index.CLK_OUT_LOW( _SHT_IO );
	}

	_SHT1x_Func_Index.CLK_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_LOW( _SHT_IO );
	_SHT1x_Func_Index.DATA_DIR_IN( _SHT_IO );
	
	while( _SHT1x_Func_Index.DATA_IN( _SHT_IO ) )
	{
		cnt++;
		if(cnt > 1000000)
			return 0;
	};
	cnt = 0;
	
	// get the MSB (most significant bits)
	for (i=0; i < 8; ++i)
	{
		_val <<= 1;
		_SHT1x_Func_Index.CLK_OUT_HIGH( _SHT_IO );
		macroTASK_DELAY_MS(1);
		_val |= _SHT1x_Func_Index.DATA_IN( _SHT_IO );
		_SHT1x_Func_Index.CLK_OUT_LOW( _SHT_IO );
	}

	// send the required ACK
	_SHT1x_Func_Index.DATA_DIR_OUT( _SHT_IO );
	_SHT1x_Func_Index.DATA_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.DATA_OUT_LOW( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_LOW( _SHT_IO );

	// get the LSB (less significant bits)
	_SHT1x_Func_Index.DATA_DIR_IN( _SHT_IO );
	for (i = 0; i < 8; ++i)
	{
		_val <<= 1;
		_SHT1x_Func_Index.CLK_OUT_HIGH( _SHT_IO );
		macroTASK_DELAY_MS(1);
		_val |= _SHT1x_Func_Index.DATA_IN( _SHT_IO );
		_SHT1x_Func_Index.CLK_OUT_LOW( _SHT_IO );	
	}

	//  skip CRC
	_SHT1x_Func_Index.DATA_DIR_OUT( _SHT_IO );
	_SHT1x_Func_Index.DATA_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_HIGH( _SHT_IO );
	_SHT1x_Func_Index.CLK_OUT_LOW( _SHT_IO );

	// Convert raw value to degrees Celsius
	return _val;
}



float fSHT1x_ReadTemp( SHT1x_IO *_SHT_IO )
{
	int raw = iSHT1x_ReadRaw(macroTEMP_READ_CMD, _SHT_IO);
	float _temp = D2*raw + D1;
	return _temp;
}



float fSHT1x_ReadHumi(  SHT1x_IO *_SHT_IO )
{
	float _linear_humi, temp, _correct_humi;
	int raw;
	raw = iSHT1x_ReadRaw(macroHUMI_READ_CMD, _SHT_IO);
	_linear_humi = C1 + C2 * raw + C3 * raw * raw;
	temp = fSHT1x_ReadTemp( _SHT_IO );
	_correct_humi = (temp - 25.0 ) * (T1 + T2 * raw) + _linear_humi;
	return _correct_humi;
}



void vSHT11_Read( float *_fTemp, float *_fHumi, SHT1x_IO *_SHT_IO )
{
	int Temp_raw = iSHT1x_ReadRaw(macroTEMP_READ_CMD, _SHT_IO);
	int Humi_raw = iSHT1x_ReadRaw(macroHUMI_READ_CMD, _SHT_IO);
	
	*_fTemp = D2 * Temp_raw + D1;
	*_fHumi = (*_fTemp - 25.0) * (T1 + T2 * Humi_raw) + C1 + C2 * Humi_raw + C3 * Humi_raw * Humi_raw;
}





/********************************* SHT1x function *****************************/
static void vSHT1x_DataDir_Out( SHT1x_IO *_SHT_IO )
{
	gpio_pin_config_t Output = { kGPIO_DigitalOutput, 0, };
	GPIO_PinInit( _SHT_IO->DATA_GPIOBase, _SHT_IO->DATA_Pin, &Output );
}


static void vSHT1x_DataDir_In( SHT1x_IO *_SHT_IO )
{
	gpio_pin_config_t Input = { kGPIO_DigitalInput, 0, };
	GPIO_PinInit( _SHT_IO->DATA_GPIOBase, _SHT_IO->DATA_Pin, &Input );
}


static void vSHT1x_ClkDir_Out( SHT1x_IO *_SHT_IO )
{
	gpio_pin_config_t Output = { kGPIO_DigitalOutput, 0, };
	GPIO_PinInit( _SHT_IO->CLK_GPIOBase, _SHT_IO->CLK_Pin, &Output );
}


static void vSHT1x_ClkDir_In( SHT1x_IO *_SHT_IO )
{
	gpio_pin_config_t Input = { kGPIO_DigitalInput, 0, };
	GPIO_PinInit( _SHT_IO->CLK_GPIOBase, _SHT_IO->CLK_Pin, &Input );
}


static void vSHT1x_DataOut_High( SHT1x_IO *_SHT_IO )
{
	GPIO_WritePinOutput( _SHT_IO->DATA_GPIOBase, _SHT_IO->DATA_Pin, 1 );
}


static void vSHT1x_DataOut_Low( SHT1x_IO *_SHT_IO )
{
	GPIO_WritePinOutput( _SHT_IO->DATA_GPIOBase, _SHT_IO->DATA_Pin, 0 );
}


static void vSHT1x_ClkOut_High( SHT1x_IO *_SHT_IO )
{
	GPIO_WritePinOutput( _SHT_IO->CLK_GPIOBase, _SHT_IO->CLK_Pin, 1 );
}


static void vSHT1x_ClkOut_Low( SHT1x_IO *_SHT_IO )
{
	GPIO_WritePinOutput( _SHT_IO->CLK_GPIOBase, _SHT_IO->CLK_Pin, 0 );
}


static uint32_t uSHT1x_Data_In( SHT1x_IO *_SHT_IO )
{
	return GPIO_ReadPinInput( _SHT_IO->DATA_GPIOBase, _SHT_IO->DATA_Pin );
}
/******************************************************************************/















