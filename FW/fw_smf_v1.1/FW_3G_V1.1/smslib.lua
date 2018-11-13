-------------------------------------------------------------------------------
-- File name: sms.lua
-- Description: send/read/del sms, process data rev from sms...
-- Author: giangnv
-- Date: 06-Oct-2017
-- Project: AQI V1.3
-- Last Update: 30-Nov-2017
------------------------------for select character-----------------------------
CSCS_IRA=0;	--ref alphabet
CSCS_GSM=1;	--GSM default alphabet
CSCS_UCS2=2;--16-bit universal multiple-octet coded character

------------------------------for del sms--------------------------------------
SMS_DELETE_FLAG_NONE = 0;	--default
SMS_DELETE_FLAG_READ = 1;
SMS_DELETE_FLAG_UNREAD = 2;
SMS_DELETE_FLAG_SENT = 4;
SMS_DELETE_FLAG_UNSENT = 8;
SMS_DELETE_FLAG_SENT_ST_NOT_RECEIVED = 16;
SMS_DELETE_FLAG_ALL = 0xFF;
-------------------------------------------------------------------------------
l_cWhoAmI						= 1;
SIO_SMS_RESET					= 2;
-------------------------------------------------------------------------------
sio_sms_hdr = {};								--declare an array
sio_sms_hdr[l_cWhoAmI]                     		= "Check debug!";
sio_sms_hdr[SIO_SMS_RESET]                  	= "Reset now!";					
-------------------------------------------------------------------------------
SMS_OK											= "OK";
CUSD_CHECK_ACC              = "*101#";
-------------------------------------------------------------------------------
local sms_function = {}
-- include file	
user_board = require("parser_data_boardsensor");
-------------------------------------------------------------------------------
--@function: config sms
--@para: none
--@return: none
-------------------------------------------------------------------------------
function sms_function.vConfigSms()
	print("---- vConfigSms: Config sms-----\r\n");
	sio.send("AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n"); -- store sms to sim memory
	vmsleep(400);
	sio.send("AT+CNMI=2,1\r\n");
	vmsleep(1000);
	os.set_cscs(CSCS_IRA);	-- Select TE character set
	vmsleep(400);
	sms.set_cmgf(1);		-- set fomat sms (text=1 or PDU=0)
	vmsleep(400);
	sms.set_csmp(17,14,0,0);	
	vmsleep(400);
	sms.set_csdh(1); 		-- Show text mode parameters
	vmsleep(1000);
end;
-------------------------------------------------------------------------------
--@function: get index sms to read content sms rev
--@para: cSIO_Data_Rev: data notification that has new sms.
--@return: -1 if get index sms failed
-- 			a number >=0 if get index sms done
-------------------------------------------------------------------------------
function sms_function.iGetIndexSms(cSIO_Data_Rev)
	local iSms_index = -1;
	local token = '%s*[,]%s*';
	local arr_data = {};
	-- parse sio data just received, data get: +CMTI: "SM",Index
	arr_data = user_board.Parser_msg(cSIO_Data_Rev, token);
	iSms_index = tonumber(arr_data[2]);
	if(iSms_index ~=nil) then 				-- check if whether isms not is number
		print("---- sms_function.iGetIndexSms: iSms_index = ",iSms_index,"\r\n");
		return iSms_index;
	else
		print("---- sms_function.iGetIndexSms: get index sms failed! r\n");
		return -1;
	end;
end;
-------------------------------------------------------------------------------
--@function: get phone number that send to this sim
--@para: cContent: detail sms received: phone number, content,...
				--			fomat: "REC UNREAD","+84886137421","","17/12/07,11:18:31+28",145,4,0,0,"+8491020005",145,3,0,0,0 Alo
--@return: number phone
-------------------------------------------------------------------------------
function sms_function.cGetPhoneNumber(cContent)
	local cData="";
	local token = '%s*[,]%s*';
	-- step 1: delete token ""
	cData = string.gsub(cContent,"\"","");
	print("---- sms_function.iGetPhoneNumber: data after del \": ",cData,"\r\n");
	-- step 2: parser
	local struct_data = user_board.Parser_msg(cData,token);
	print("---- sms_function.iGetPhoneNumber:: phone number: ",struct_data[2],"\r\n");
	return struct_data[2];
end;
-------------------------------------------------------------------------------
--@function: send sms
--@para	   : cPhone: phone number that need to send
--			 cContent: content send to cPhone

--@Return  : eFailed if send failed
--			 eSuccessed if send successed
-------------------------------------------------------------------------------
function sms_function.eSend(cPhone,cContent)
	local Result;
	--check cPhone and cContent
	if((string.len(cPhone) < 9) or (string.len(cContent) == 0)) then
		return eFailed;
	end;
	Result = sms.send(cPhone,cContent);
	vmsleep(200);
	if(Result == true) then
		print("---- sms_function.eSend: Send successed!\r\n");
		return eSuccessed;
	else
		print("---- sms_function.eSend: Send failed!\r\n");
		return eFailed;
	end;
end;
-------------------------------------------------------------------------------
--@function: read sms rev
--@para	   : iSms_index: index of msg rev
--@Return  : content sms
-------------------------------------------------------------------------------
function sms_function.cRead(iSms_index)
	local sRet = "";
	ret,sms_content = sms.read(iSms_index);
	vmsleep(100);
	if(string.len(sms_content) > 0) and (sms_content ~=nil) then
		print("---- Sms_cRead: sms_content =  ",sms_content,"\r\n");
		return sms_content;
	else
		print("---- Sms_cRead: read sms failed! \r\n");
		return sRet;
	end;
end;
-------------------------------------------------------------------------------
--@function: del  sms
--@para	   : sms_del_flag: sms flag is defined in top file
--@Return  : none
-------------------------------------------------------------------------------
function sms_function.vDeleteSms()
	local ret;
	--ret = sms.delete(sms_del_flag);
	sio.send("AT+CMGD=,4\r\n");	-- del all sms using cmd
	print("---- delsms all\r\n");
	vmsleep(1000);
end;
-------------------------------------------------------------------------------
--@function : process sms command
--@para: cSmsCommand,hdr_idx
--@Return: none
-------------------------------------------------------------------------------
function sms_function.vHandleSmsCommand(hdr_idx,cSmsCommand)
	local msg_response = "";
	print("---- sms_function.vHandleSmsCommand called!\r\n");
	-- get phone number to response sms
	local cPhoneNumber = sms_function.cGetPhoneNumber(cSmsCommand);
	if(hdr_idx == l_cWhoAmI) then
		print("---- sms_function.vHandleSmsCommand: SMS check debug....\r\n");
		msg_response = string.format("DEBUG,NW:%d,getID:%d\r\n",status_network,flag_get_iddev);
		sms_function.eSend(cPhoneNumber,msg_response);
		return;
	elseif(hdr_idx == SIO_SMS_RESET) then
		sms_function.eSend(cPhoneNumber,SMS_OK);
		print("---- sms_function.vHandleSmsCommand: SMS Command Reset....\r\n");
		sio.send("AT+REBOOT\r\n");
		return;
	else
		return;
	end;
end;

-------------------------------------------------------------------------------
--@funtion: send sms cUSD
--para: cUSD cmd, ex cstr = "*101#"
--return: none
-------------------------------------------------------------------------------

function sms_function.vSendCUSD (strCUSD)
  local cmd = "";
  local iDcs = 15; -- Data Coding Scheme
  cmd = string.format("AT+CUSD=1,%s,%d\r\n",strCUSD,iDcs);
  sio.send(cmd);
  vmsleep(50);
  return;
end;


--------------------------------------------------------------------------------
return sms_function




