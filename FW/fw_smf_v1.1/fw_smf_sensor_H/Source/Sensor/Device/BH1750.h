
#ifndef __BH1750_H
#define __BH1750_H

#include "pin_mux.h"
#include "fsl_gpio.h"

/******************************Define BH1750***********************************/

/*Recommend to use H Res Mode*/
#define BH1750_Address_L 					0x23 // Device address when address pin LOW, this is default val
#define BH1750_Address_H 					0x5C // Device address when address pin HIGH


#define BH1750_POWER_DOWN        			0x00 
#define BH1750_POWER_ON          			0x01 
#define BH1750_DATA_REG_RESET    			0x07 

//BH1750 Mode
#define BH1750_CONT_1LX_HRES       			0x10
#define BH1750_CONT_05LX_HRES    			0x11
#define BH1750_CONT_4LX_LRES       			0x13
#define BH1750_ONETIME_1LX_HRES        		0x20
#define BH1750_ONETIME_05LX_HRES     		0x21
#define BH1750_ONETIME_4LX_LRES        		0x23


/******************************************************************************/
void vBH1750_Init(void);




/******************************************************************************/
uint16_t vBH1750_GetLux(void);


#endif








