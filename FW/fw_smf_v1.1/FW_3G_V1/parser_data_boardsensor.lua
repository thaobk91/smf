-------------------------------------------------------------------------------
-- File name: parser_data_boardsensor.lua
-- Description: -- interface to GW
--              -- process msg from GW
-- Date: 04-Aug-2018
-- Last Update: 04-Aug-2018
-------------------------------------------------------------------------------

local parser_data_boardsensor =  {}
-------------------------------------------------------------------------------
--user define
--------------------------------------------------
--type device
--------------------------------------------------
macroTYPE_DEVICE_NODESS			= "SMF_SENSN";
macroTYPE_DEVICE_NODECTRL		= "SMF_CTRLN";
macroTYPE_DEVICE_GW				= "SMF_GW";
--------------------------------------------------
--type packet
--------------------------------------------------
macroTYPE_PACKET_DATA			= "DATA";
macroTYPE_PACKET_CONF			= "CONF";
macroTYPE_PACKET_REQS			= "REQS";
macroTYPE_PACKET_RESP			= "RESP";
macroTYPE_PACKET_CTRL			= "CTRL";
macroTYPE_PACKET_WAMI			= "WAMI";
--------------------------------------------------
--Config msg
--------------------------------------------------
macroCONF_FOTA					= "OTA";
macroCONF_DATAUT				= "DataUT";
macroCONF_RTC					= "RTC";
macroCONF_IDGW					= "IDGW";
macroCONF_MQTT                  = "MQTT";
--------------------------------------------------
-- response msg
--------------------------------------------------
macroRESP_OK					= "OK";
macroRESP_NOK					= "NOK";
--------------------------------------------------
-- request msg
--------------------------------------------------
macroREQS_ALL					= "ALL"
macroREQS_TEMPA					= "TempA"
macroREQS_HUMIA					= "HumiA"
macroREQS_TEMPG					= "TempG"
macroREQS_HUMIG					= "HumiG"	
macroREQS_PH					= "PH"
macroREQS_EC					= "EC"
macroREQS_LIGHT					= "Light"
macroREQS_RTC					= "RTC"
macroREQS_IDGW					= "IDGW"
--------------------------------------------------
--define sensor
--------------------------------------------------
macroID_TEMPA_SENSOR				= "TempA"
macroID_HUMIA_SENSOR				= "HumiA"
macroID_TEMPG_SENSOR				= "TempG"
macroID_HUMIG_SENSOR				= "HumiG"
macroID_PH_SENSOR					= "PH"
macroID_EC_SENSOR					= "EC"
macroID_LIGHT_SENSOR				= "Light"
--------------------------------------------------
--define ID Connectivity
macroCONNECTIVITY_3G			= "3G"
--------------------------------------------------
eSuccessed						= 1;
eFailed							= 0;
--------------------------------------------------
-- define type enum for type packet
--------------------------------------------------
eData							= 0
eResponse						= 1
eConfig							= 2
eRequest						= 3
eControl						= 4
--------------------------------------------------
--define cmd config
macroCONF_OTA   					      = "OTA"   
macroCONF_DATAUT 					      = "DataUT"
macroCONF_RTC    					      = "RTC"
macroCONF_TEMPA_SENSOR				      = "TempA"
macroCONF_HUMIA_SENSOR				      = "HumiA"
macroCONF_TEMPG_SENSOR				      = "TempG"
macroCONF_HUMIG_SENSOR				      = "HumiG"
macroCONF_PH_SENSOR					      = "PH"
macroCONF_EC_SENSOR					      = "EC"
macroCONF_LIGHT_SENSOR				      = "Light"


macroCONF_DEBUG                           = "DEBUG"
macroCONF_RST                               = "RESET"
--------------------------------------------------
--define type enum for fields in packet_t
--------------------------------------------------
eTypePacketField 					 = 1
eTypeDeviceField					 = 2
--eIDGateWay							 = 3
eIDEndDevice						 = 3
eStart_DataField					 = 4
--------------------------------------------------

---------------------------------------------------
-- for package send to server
macroJSTYPE_PACKET					         = "TypePacket";
macroJSTYPE_DEVICE					         = "TypeDevice";
macroJSID_EP							     = "DeviceIDEp";
macroJSID_GW							     = "DeviceIDGw";
macroJS_DATA							     = "Data";
macroJS_TIME							     = "Time";
---------------------------------------------------

--parser msg from board sensor
--pakage msg
--send to server
-------------------------------------------------------------------------------
--@function: check packet
--@para: cPacket: string that received from board 
--@return: eSuccessed if passed
--		   eFailed if failed
--note: if need to check more, please enter code in this function
-------------------------------------------------------------------------------
function parser_data_boardsensor.ucCheckPacket(cPacket)
	-- only check end character'!'
	if(string.find(cPacket,'!') ~= nil) then 
		return eSuccessed;
	else
		return eFailed;
	end;
end;
-------------------------------------------------------------------------------
-- function: parser msg
-- para: 	str: msg input
--			delim: token parser
--			maxNb: number parser. ex: maxNb = 6, --> split = [1], [2],..[6]
-------------------------------------------------------------------------------
function parser_data_boardsensor.Split(str, delim, maxNb)
      -- Eliminate bad cases...
   if string.find(str, delim) == nil then
      return { str }
   end
   if maxNb == nil or maxNb < 1 then
      maxNb = 0    -- No limit
   end
   local result = {}
   local pat = "(.-)" .. delim .. "()"	-- ?
   local nb = 0
   local lastPos
   for part, pos in string.gfind(str, pat) do
      nb = nb + 1
      result[nb] = part
      lastPos = pos
      if nb == maxNb then
         break
      end
   end
   -- Handle the last field
   if nb ~= maxNb then
      result[nb + 1] = string.sub(str, lastPos)
   end
   return result;
end
-------------------------------------------------------------------------------
-- @function: parser msg
-- @para: strmsg: string
--		  token: parse follow token --> '%s*[,:]%s*'
-- @return: array contains each field after parse
-------------------------------------------------------------------------------
function parser_data_boardsensor.Parser_msg(strmsg, token)
	local xfield= {};
	local arr_field = {};
	--split msg
	xfield = parser_data_boardsensor.Split(strmsg, token);
	-- assign value for arr_field
	for i=1,#xfield do
		arr_field[i] = xfield[i];
	end;

	--print arr_data
--	print("---- Parser_msg: data after parse\n");
--	for i=1,#arr_field do
--		print(arr_field[i],"\n");
--	end;
	
	return arr_field;
end;

-------------------------------------------------------------------------------
-- HandleMessageData
-------------------------------------------------------------------------------
function parser_data_boardsensor.HandleMessageData(xpacket_t)
    print("---- parser_data_boardsensor: Message is ",xpacket_t[1],"\r\n");
	local l_Data2Server = parser_data_boardsensor.cSend2server(xpacket_t, eData);
	return l_Data2Server;
end;

-------------------------------------------------------------------------------
-- HandleMessageConfig
-------------------------------------------------------------------------------
function parser_data_boardsensor.HandleMessageConfig(xpacket_t)
	-- bo qua check type device
	-- check data
	print("---- parser_data_boardsensor: Message is ",xpacket_t[1],"\r\n");
	if(xpacket_t[3] == macroCONF_IDGW) then
		cID_GateWay = xpacket_t[4];		-- cID_GateWay: declare in autorun.lua
		return;
	end;
end;

-------------------------------------------------------------------------------
-- HandleMessageResponse
-- strResult = macroRESP_OK/macroRESP_NOK
-------------------------------------------------------------------------------
function parser_data_boardsensor.HandleMessageResponse(strResult)
    local cSendToGW = string.format("%s,%s,%s!",macroTYPE_PACKET_RESP,macroTYPE_DEVICE_GW,strResult);
    atctl.send(cSendToGW);
    print("---- parser_data_boardsensor: Send ",cSendToGW," to GW \r\n");
end;

-------------------------------------------------------------------------------
-- HandleMessageControl
-------------------------------------------------------------------------------
function parser_data_boardsensor.HandleMessageControl()

end;
-------------------------------------------------------------------------------
-- HandleMessageRequest
-------------------------------------------------------------------------------
function parser_data_boardsensor.HandleMessageRequest(xpacket_t)
	-- bo qua check type device
	-- check data
	print("---- HandleMessageRequest: Message is ",xpacket_t[1],"\r\n");
	if(xpacket_t[3] == macroCONF_RTC) then
		local l_Data2Server = parser_data_boardsensor.cSend2server(xpacket_t, eRequest);
		return l_Data2Server;
	end;
end;
-------------------------------------------------------------------------------
--
-------------------------------------------------------------------------------
-- xpacket: table(struct)
-- xpacket[1-4]: type_packet, TypeEnddevice(SMF_SENSN), IDGW, ID end device
-- xpacket[5]: ID_ss1
-- xpacket[6]: valua_ss1
-- xpacket [7..]
--@function: send msg to server
--@para: packet_t: packet after parser
--@para: etype_packet: type packet need to send to server
--@return: a string cMsg2server that send to server
-------------------------------------------------------------------------------
function parser_data_boardsensor.cSend2server(xpacket, etype_packet)
	
	local Flag_TimeJS = 0;
	cMsg2server = "";	-- string that send to sv
	-- process for data msg
	if(etype_packet == eData) then
		cMsg2server = string.format("{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":{"
								,macroJSTYPE_PACKET
								,xpacket[eTypePacketField]
								,macroJSTYPE_DEVICE
								,xpacket[eTypeDeviceField]
								,macroJSID_EP
								,xpacket[eIDEndDevice]
								,macroJSID_GW
								,cID_GateWay
								,macroJS_DATA);
		index_data_start = eStart_DataField;			-- =5
		index_data_end = #xpacket;
		for i= index_data_start, index_data_end do
			if(Flag_TimeJS == 1) then				-- time field
				cMsg2server = cMsg2server.."\"";
				cMsg2server = cMsg2server..xpacket[i]; -- yyyy-mm-ddTh
				cMsg2server = cMsg2server..":";
				cMsg2server = cMsg2server..xpacket[i+1];-- min		
				cMsg2server = cMsg2server..":";
				cMsg2server = cMsg2server..xpacket[i+2]; --sec
				cMsg2server = cMsg2server.."\"}}";
				break;		-- exit for loop
			end;
			if(i%2 == 0) then		--ID DATA
				cMsg2server = cMsg2server.."\"";
				cMsg2server = cMsg2server..xpacket[i];
				cMsg2server = cMsg2server.."\":";
				if(xpacket[i] == macroJS_TIME) then
					Flag_TimeJS = 1;
				end;
					
			else					-- value DATA
				cMsg2server = cMsg2server.."\"";
				cMsg2server = cMsg2server..xpacket[i];
				cMsg2server = cMsg2server.."\",";
			end;
		end;
		print("---- cSend2server: cMsg2server = ",cMsg2server, "\r\n");
		return cMsg2server;
	end;
	
	-- process for config msg
	if(etype_packet == eRequest) then
		cMsg2server = string.format("{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"\",\"%s\":\"%s\",\"%s\":{\"%s\":\"\"}}"
								,macroJSTYPE_PACKET
								,xpacket[eTypePacketField]
								,macroJSTYPE_DEVICE
								,xpacket[eTypeDeviceField]
								,macroJSID_EP
								,macroJSID_GW
								,cID_GateWay
								,macroJS_DATA
								,macroCONF_RTC);
		print("---- cSend2server: cMsg2server = ",cMsg2server, "\r\n");
		return cMsg2server;
	end;
end;
----------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------
return parser_data_boardsensor;


