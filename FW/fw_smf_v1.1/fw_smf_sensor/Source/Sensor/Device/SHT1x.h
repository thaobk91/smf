
#ifndef __SHT11_H
#define __SHT11_H

#include "pin_mux.h"
#include "fsl_gpio.h"

#include "main.h"


typedef struct
{
	GPIO_Type 		*DATA_GPIOBase;
	uint8_t			DATA_Pin;
	
	GPIO_Type 		*CLK_GPIOBase;
	uint8_t			CLK_Pin;
}SHT1x_IO;


typedef struct
{
	void (*DATA_DIR_OUT)	( SHT1x_IO * );
	void (*DATA_DIR_IN) 	( SHT1x_IO * );
	void (*CLK_DIR_OUT) 	( SHT1x_IO * );
	void (*CLK_DIR_IN) 		( SHT1x_IO * );
	
	void (*DATA_OUT_HIGH) 	( SHT1x_IO * );
	void (*DATA_OUT_LOW) 	( SHT1x_IO * );
	void (*CLK_OUT_HIGH) 	( SHT1x_IO * );
	void (*CLK_OUT_LOW) 	( SHT1x_IO * );
	
	uint32_t (*DATA_IN) 	( SHT1x_IO * );
}SHT1x_Func;


/* Define function */
void vSHT11_Read( float *_fTemp, float *_fHumi, SHT1x_IO *_SHT_IO );

#endif









