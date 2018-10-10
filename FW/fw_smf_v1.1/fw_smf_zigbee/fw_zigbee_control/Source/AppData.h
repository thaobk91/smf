
#ifndef __APPDATA_H
#define __APPDATA_H

#include "Main.h"


#define APPDATA_ENDPOINT           			8
#define APPDATA_PROFILE						0x0F04
#define APPDATA_DEVICE_ID					0x0001
#define APPDATA_DEVICE_VERSION				0
#define APPDATA_FLAGS						0

/************************CLUSTER**********************/
#define APPDATA_MAX_CLUSTERS_IN				1
#define APPDATA_MAX_CLUSTERS_OUT			1

//Cluster ID
#define APPDATA_CLUSTER_COOR_OUT_NODE_IN	0xD001
#define APPDATA_CLUSTER_COOR_IN_NODE_OUT	0xD002

/*****************************************************/

//Output
#define macroOUTPUT_1_PORT					GPIO_C_BASE
#define macroOUTPUT_1_PIN					GPIO_PIN_7
#define macroOUTPUT_1_ON()					GPIOPinWrite( macroOUTPUT_1_PORT, macroOUTPUT_1_PIN, macroOUTPUT_1_PIN)
#define macroOUTPUT_1_OFF()					GPIOPinWrite( macroOUTPUT_1_PORT, macroOUTPUT_1_PIN, 0)

#define macroOUTPUT_2_PORT					GPIO_C_BASE
#define macroOUTPUT_2_PIN					GPIO_PIN_6
#define macroOUTPUT_2_ON()					GPIOPinWrite( macroOUTPUT_2_PORT, macroOUTPUT_2_PIN, macroOUTPUT_2_PIN)
#define macroOUTPUT_2_OFF()					GPIOPinWrite( macroOUTPUT_2_PORT, macroOUTPUT_2_PIN, 0)

#define macroOUTPUT_3_PORT					GPIO_C_BASE
#define macroOUTPUT_3_PIN					GPIO_PIN_5
#define macroOUTPUT_3_ON()					GPIOPinWrite( macroOUTPUT_3_PORT, macroOUTPUT_3_PIN, macroOUTPUT_3_PIN)
#define macroOUTPUT_3_OFF()					GPIOPinWrite( macroOUTPUT_3_PORT, macroOUTPUT_3_PIN, 0)

#define macroOUTPUT_4_PORT					GPIO_C_BASE
#define macroOUTPUT_4_PIN					GPIO_PIN_4
#define macroOUTPUT_4_ON()					GPIOPinWrite( macroOUTPUT_4_PORT, macroOUTPUT_4_PIN, macroOUTPUT_4_PIN)
#define macroOUTPUT_4_OFF()					GPIOPinWrite( macroOUTPUT_4_PORT, macroOUTPUT_4_PIN, 0)

#define macroOUTPUT_5_PORT					GPIO_C_BASE
#define macroOUTPUT_5_PIN					GPIO_PIN_1
#define macroOUTPUT_5_ON()					GPIOPinWrite( macroOUTPUT_5_PORT, macroOUTPUT_5_PIN, macroOUTPUT_5_PIN)
#define macroOUTPUT_5_OFF()					GPIOPinWrite( macroOUTPUT_5_PORT, macroOUTPUT_5_PIN, 0)

#define macroOUTPUT_6_PORT					GPIO_C_BASE
#define macroOUTPUT_6_PIN					GPIO_PIN_0
#define macroOUTPUT_6_ON()					GPIOPinWrite( macroOUTPUT_6_PORT, macroOUTPUT_6_PIN, macroOUTPUT_6_PIN)
#define macroOUTPUT_6_OFF()					GPIOPinWrite( macroOUTPUT_6_PORT, macroOUTPUT_6_PIN, 0)


//Led status
#define macroLED_STATUS_PORT				GPIO_B_BASE
#define macroLED_STATUS_PIN					GPIO_PIN_3
#define macroLED_STATUS_ON()				GPIOPinWrite( macroLED_STATUS_PORT, macroLED_STATUS_PIN, 0)
#define macroLED_STATUS_OFF()				GPIOPinWrite( macroLED_STATUS_PORT, macroLED_STATUS_PIN, macroLED_STATUS_PIN)
												



extern cId_t GenericApp_InClusterList[APPDATA_MAX_CLUSTERS_IN];
extern cId_t GenericApp_Out_ClusterList[APPDATA_MAX_CLUSTERS_OUT];
extern SimpleDescriptionFormat_t GenericApp_SimpleDesc;
extern endPointDesc_t xDataEPDesc;


#endif











