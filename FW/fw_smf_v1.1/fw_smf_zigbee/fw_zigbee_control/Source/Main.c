
#include "mac_low_level.h"

#include "Main.h"
#include "Message.h"


uint8 uMain_TaskID; 			
devStates_t xMain_NWKSatte;
uint8 uPacketIDSent; 				//Counter send msg
afAddrType_t xAddrCoordinator;

char IDZb[17] = {0};
uint16_t uCounterNWK = 0;
uint8_t uLedCounter = 0;
uint16_t uSendCounter = 0;

char ZIGBEE_RX_Buffer[macroZIGBEE_RX_BUFF_SIZE] = {0};
char ZIGBEE_TX_Buffer[macroZIGBEE_TX_BUFF_SIZE] = {0};

//output status
uint8_t OutVAC_Status[macroMAX_OUTPUT_VAC] = {0};



void vMain_GetIDLocal( char *IDLocal );





/*******************************************************************************
* Function: void vMain_Init( byte task_id )
* Description: main task init
* Param: task_id - task id
* Return: none
*******************************************************************************/
void vMain_Init( byte task_id )
{
	uMain_TaskID = task_id;
	xMain_NWKSatte = DEV_INIT;
	uPacketIDSent = 0;

	xAddrCoordinator.addrMode = (afAddrMode_t)Addr16Bit;
	xAddrCoordinator.endPoint = 8;
	xAddrCoordinator.addr.shortAddr = 0;

	// Register the endpoint description with the AF
	afRegister( &xDataEPDesc );

	ZDO_RegisterForZDOMsg( uMain_TaskID, Device_annce );
	ZDO_RegisterForZDOMsg( uMain_TaskID, NWK_addr_rsp );
	
	//Output
	GPIOPinTypeGPIOOutput(macroOUTPUT_1_PORT, macroOUTPUT_1_PIN);
	IOCPadConfigSet(macroOUTPUT_1_PORT, macroOUTPUT_1_PIN, IOC_OVERRIDE_PDE);//Pull up resistor
	
	GPIOPinTypeGPIOOutput(macroOUTPUT_2_PORT, macroOUTPUT_2_PIN);
	IOCPadConfigSet(macroOUTPUT_2_PORT, macroOUTPUT_2_PIN, IOC_OVERRIDE_PDE);//Pull up resistor
	
	GPIOPinTypeGPIOOutput(macroOUTPUT_3_PORT, macroOUTPUT_3_PIN);
	IOCPadConfigSet(macroOUTPUT_3_PORT, macroOUTPUT_3_PIN, IOC_OVERRIDE_PDE);//Pull up resistor
	
	GPIOPinTypeGPIOOutput(macroOUTPUT_4_PORT, macroOUTPUT_4_PIN);
	IOCPadConfigSet(macroOUTPUT_4_PORT, macroOUTPUT_4_PIN, IOC_OVERRIDE_PDE);//Pull up resistor
	
	GPIOPinTypeGPIOOutput(macroOUTPUT_5_PORT, macroOUTPUT_5_PIN);
	IOCPadConfigSet(macroOUTPUT_5_PORT, macroOUTPUT_5_PIN, IOC_OVERRIDE_PDE);//Pull up resistor
	
	GPIOPinTypeGPIOOutput(macroOUTPUT_6_PORT, macroOUTPUT_6_PIN);
	IOCPadConfigSet(macroOUTPUT_6_PORT, macroOUTPUT_6_PIN, IOC_OVERRIDE_PDE);//Pull up resistor
	
	//led status
	GPIOPinTypeGPIOOutput(macroLED_STATUS_PORT, macroLED_STATUS_PIN);
	IOCPadConfigSet(macroLED_STATUS_PORT, macroLED_STATUS_PIN, IOC_OVERRIDE_PUE);//Pull up resistor
	
	vUART_Init(macroUART_DEBUG_BASE, 115200);
	APP_DEBUG("\r\n------------------------------ MAIN TASK ------------------------------\r\n");
	
	vMain_GetIDLocal( IDZb );
	APP_DEBUG("--- Main: ID Zigbee = %s\r\n", IDZb);

	if(macRadioSetTxPower(0xFE) == 0xFE)
		APP_DEBUG("--- Main: Set Tx Power with CC2592 Successfully!\r\n");
	else
		APP_DEBUG("--- Main: Set Tx Power with CC2592 failture!\r\n");
	
	macroLOOP_EVENT(uMain_TaskID, EVENT_LED_STATUS, macroTIME_EVENT_LED_STATUS);
}
//End main init





/*******************************************************************************
* Function: UINT16 uiMain_ProcessEvent( byte task_id, UINT16 events )
* Description: main task event process
* Param: 
*		+ task_id: task id
*		+ events: event task
* Return: none
*******************************************************************************/
UINT16 uiMain_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  // Data Confirmation message fields
  (void)task_id;          //Intentionally unreferenced parameter

	if ( events & SYS_EVENT_MSG )
	{
		MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( uMain_TaskID );
		while ( MSGpkt )
		{
			switch ( MSGpkt->hdr.event )
			{
				case ZDO_CB_MSG: {
					APP_DEBUG("--- MAIN: ZDO_CB_MSG Stat\r\n");
					vZigbeeIO_ProcessZDOMsgs((zdoIncomingMsg_t *)MSGpkt);
					break;
				}
				case AF_DATA_CONFIRM_CMD: {
					if((ZStatus_t)(MSGpkt->hdr.status) == afStatus_SUCCESS)	
					{
						APP_DEBUG("--- MAIN: Send Data OK!. TX Counter = %d\r\n", uPacketIDSent);
					}
					else 
					{
						APP_DEBUG("--- MAIN: Send Data FAIL! Status = 0x%x\r\n", (ZStatus_t)(MSGpkt->hdr.status));
						macroLOOP_EVENT(uMain_TaskID, EVENT_SEND_MESSAGE, macroTIME_EVENT_RESEND_DATA);
					}
					break;
				}
				case AF_INCOMING_MSG_CMD: {
					uZigbeeIO_RecvMessage(MSGpkt);
					break;
				}
				case ZDO_STATE_CHANGE: {
					//zAddrType_t dstAddr;
					xMain_NWKSatte = (devStates_t)(MSGpkt->hdr.status);
					uCounterNWK++;
					if(uCounterNWK >= 90)
					{
						uCounterNWK = 0;
						//SysCtrlReset();
					}

					APP_DEBUG("--- MAIN: ZDO_STATE. NWK State = 0x%x\r\n", xMain_NWKSatte);
					if((xMain_NWKSatte == DEV_ZB_COORD) || (xMain_NWKSatte == DEV_ROUTER) || (xMain_NWKSatte == DEV_END_DEVICE))
					{
						uCounterNWK = 0;
						APP_DEBUG("--- MAIN: NLME_GetShortAddr = 0x%x\r\n", NLME_GetShortAddr());
					}
				}
				break;
			}
			//Release the memory
			osal_msg_deallocate( (uint8 *)MSGpkt );
			MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( uMain_TaskID );
		}
		//return unprocessed events
		return (events ^ SYS_EVENT_MSG);
	}
	
	//recv data from zigbee
	if(events & EVENT_ZIGBEE_RECV)
	{
		APP_DEBUG("--- Main: EVENT_ZIGBEE_RECV\r\n");
		vMessage_Check( ZIGBEE_RX_Buffer );
		
		events ^= EVENT_ZIGBEE_RECV;
	}
	
	//send data
	if(events & EVENT_SEND_MESSAGE)
	{
		uSendCounter++;
		if(uSendCounter == 1)
		{
			if(uZigbeeIO_SendMessage(xAddrCoordinator, APPDATA_CLUSTER_COOR_IN_NODE_OUT, (uint8_t *)ZIGBEE_TX_Buffer) == 0)
				uSendCounter++;
		}
		else if(uSendCounter >= (5000 / macroTIME_EVENT_RESEND_DATA))
		{
			uSendCounter = 0;
		}
		
		if(uSendCounter != 1)
			macroLOOP_EVENT(uMain_TaskID, EVENT_SEND_MESSAGE, macroTIME_EVENT_RESEND_DATA);
		
		events ^= EVENT_SEND_MESSAGE;
	}
	
	//led status
	if(events & EVENT_LED_STATUS)
	{
		uLedCounter++;
		
		if(uLedCounter < (3000 / macroTIME_EVENT_LED_STATUS))
		{
			macroLED_STATUS_OFF();
		}
		else
		{
			uLedCounter = 0;
			macroLED_STATUS_ON();
		}

		macroLOOP_EVENT(uMain_TaskID, EVENT_LED_STATUS, macroTIME_EVENT_LED_STATUS);
		events ^= EVENT_LED_STATUS;
	}
	
	return 0;
}




/*******************************************************************************
* Function: void vMain_GetIDLocal( char *IDLocal )
* Description: get ID device local
* Param: None
* Return: None
*******************************************************************************/
void vMain_GetIDLocal( char *IDLocal )
{
	uint8 *IDExtLocal = 0;
	uint8 cHex[17] = "0123456789ABCDEF";
	uint8 uCnt = 0;
	
	IDExtLocal = NLME_GetExtAddr();
	
	for(uint8 ui = 0; ui < Z_EXTADDR_LEN; ui++)
	{
		IDLocal[uCnt++] = cHex[IDExtLocal[ui] / 0x10];
		IDLocal[uCnt++] = cHex[IDExtLocal[ui] % 0x10];
	}
}


						








														
														
														




