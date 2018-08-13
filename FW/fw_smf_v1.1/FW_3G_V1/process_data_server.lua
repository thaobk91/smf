-------------------------------------------------------------------------------
--file name: process_data_server.lua
--Description: -- check msg from server
			   -- parser msg
			   -- process data received from server
			   -- process some type msg: GPS,OTA...
			   -- handle msg
-- date: 04/08/2018
-- Last Update: 04-Aug-2018
-------------------------------------------------------------------------------
-- define typepacke enum for server
sv_eData               = 0
sv_eResponse           = 1
sv_eConfig             = 2
sv_eRequest            = 3
sv_eControl            = 4
------------------------------------------------------------

sv_Response_OK			= "OK";
sv_Response_NOK			= "NOK";

--for MQTT config
macroNOTCHANGE			= "";

--------------------------------------------------
--define type enum for fields in server packet_t
--------------------------------------------------
eTypePacketField_sv 		    = 1
eTypeDevField_sv			    = 3
eIDEndDevice_sv                 = 5
eIDGW_sv                        = 7
eIDDataField_sv                 = 9

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
local process_data_server =  {}
user_board = require("parser_data_boardsensor");
user_ota = require("ota");
user_mqtt = require ("at_mqtt");
user_fs = require ("filesystem");
user_gps = require ("gpslib");
-------------------------------------------------------------------------------
--@function: check packet, only check length and end character
--@para: cPacket: packet has format {"Typepacket":.........}}
--@return: eSuccessed: if passed
--		   eFailed	 : if failed
-------------------------------------------------------------------------------
function process_data_server.ucCheckPacket(cPacket)
	local ret = eFailed;
	---------check length------------------
	if(tonumber(cPayload) == nil) then				--cPayload declare in main.lua
		return eFailed;
	elseif(string.len(cPacket) == tonumber(cPayload)) then
		print("---- ucCheckPacketsv: Passed check len\r\n");
		ret = eSuccessed;
	end;
	---------check end character "}}"------------
	i,j = string.find(cPacket, "}}");
	if(j == string.len(cPacket)) then
		print("---- ucCheckPacketsv: Passed check end packet\r\n");
		ret = eSuccessed;
	else
		return eFailed;
	end;
	return ret;
end;
-------------------------------------------------------------------------------
--@function: parser packet
--@para: cPacket: packet has format {"Typepacket":.........}}
--@return: xpacket: struct packet
-- packet[1]=JSTypepacket
-- packet [2] =TypePacket
-- packet [3] = JSTypeDevice
-- packet [4] = TypeDevice 
-------------------------------------------------------------------------------
function process_data_server.ucParsePacket(cPacket)
	local token = '%s*[:,]%s*';
	-- step1:delete ",{,}
	cPacket = string.gsub(cPacket,"\"","");
	cPacket = string.gsub(cPacket,"}","");
	cPacket = string.gsub(cPacket,"{","");
	
	--print("---- ucParsePacket: cPacket after del:",cPacket,"\r\n");
	--step2: parser
	struct_packet = user_board.Parser_msg(cPacket,token);
	return struct_packet;
end;
-------------------------------------------------------------------------------
-- @function: check each type_field
-- @para: sv_packet_t: packet server after parser
-- @return: eFailed if check failed
-- 			eSuccessed if check done
-- sv_packet_t[1]: "TypePacket"
-- sv_packet_t[2]: <TypePacket>: DATA, HAQI, DAQI....
-- sv_packet_t[3]: "DeviceIDEp"
-- sv_packet_t[4]: <DeviceIDEp>
-- sv_packet_t[5]: "TypeDevice"
-- sv_packet_t[6]: <TypeDevice>: Env_AQI, Env_WQI
-- sv_packet_t[7]: "Data"
-- sv_packet_t[8]: "name sensor"
-- sv_packet_t[..]: <value>
-------------------------------------------------------------------------------
function process_data_server.ucCheckFieldPacket(sv_packet_t)
	local ret = eFailed;
	
  -- check TypeDevice
  if(sv_packet_t[eTypeDevField_sv]== macroJSTYPE_DEVICE) then
    if((sv_packet_t[eTypeDevField_sv+1] == macroTYPE_DEVICE_NODESS) or
       (sv_packet_t[eTypeDevField_sv+1] == macroTYPE_DEVICE_GW)) then
        ret = eSuccessed;
    else
      print("---- ucCheckFieldPacket: TypeDevice failed!\r\n");
      return eFailed;
    end;
  else
    print("---- ucCheckFieldPacket: TypeDevice failed!\r\n");
    return eFailed;
  end;
  
	--check Typepacket
	if(sv_packet_t[eTypePacketField_sv]== macroJSTYPE_PACKET) then
		if((sv_packet_t[eTypePacketField_sv+1] == macroTYPE_PACKET_DATA) or
		   (sv_packet_t[eTypePacketField_sv+1] == macroTYPE_PACKET_CONF) or
		   (sv_packet_t[eTypePacketField_sv+1] == macroTYPE_PACKET_REQS) or
		   (sv_packet_t[eTypePacketField_sv+1] == macroTYPE_PACKET_CTRL) ) then
		   ret = eSuccessed;
		else
			print("---- ucCheckFieldPacket: Typepacket failed!\r\n");
			return eFailed;
		end;
	else
    print("---- ucCheckFieldPacket: TypeDevice failed!\r\n");
    return eFailed;
	end;
return ret;
end;


-------------------------------------------------------------------------------
-- HandleMsgConfigFromSV
-------------------------------------------------------------------------------
function process_data_server.HandleMsgConfigFromSV(sv_packet)
    --process config MQTT
    print("HandleMsgControlFromSV: Msg is Config\r\n");
    if(sv_packet[eIDDataField_sv+1] == macroCONF_MQTT) then
        local token = '%s*[/]%s*';
        local struct_MQTT = user_board.Parser_msg(sv_packet[eIDDataField_sv+2],token);
        cServerADDR     = struct_MQTT[1];
        cPortSV         = struct_MQTT[2];
        cUserMqtt       = struct_MQTT[3];
        cPassMqtt       = struct_MQTT[4];
        
        print("---- cServerADDR:",cServerADDR,"\r\n");
        print("---- cPortSV: ",cPortSV,"\r\n");
        print("---- cUserMqtt: ",cUserMqtt,"\r\n");
        print("---- cPassMqtt: ",cPassMqtt,"\r\n");
        
        -- write to file.....
        -- for addr mqtt
        if(string.len(cServerADDR) > 0) then    -- ensure addr server not empty
            user_fs.vEraser_segment(FILE_CONFIG,addr_svaddr,SIZE_ADDR_SV);
            user_fs.vWritefile(FILE_CONFIG,cServerADDR,addr_svaddr);
        else
            print("---- vSend2board: addr server not right!\r\n");
            return;
        end;
        -- for port port
        if(string.len(cPortSV) > 0) and (tonumber(cPortSV) ~= nil) then -- ensure cPortSV not empty and cPortSV is number
            user_fs.vEraser_segment(FILE_CONFIG,addr_svport,SIZE_PORT);
            user_fs.vWritefile(FILE_CONFIG,cPortSV,addr_svport);
        else
            print("---- vSend2board: cPortSV not right!\r\n");
            return;
        end;
        -- for user mqtt
        if(cUserMqtt ~= macroNOTCHANGE) or (cUserMqtt == "") then --string empty ("") not compare in Lua so have to separate??
            print("---- User mqtt: ",cUserMqtt);
            user_fs.vEraser_segment(FILE_CONFIG,addr_mqtt_user,SIZE_MQTT_USER_PASS);
            user_fs.vWritefile(FILE_CONFIG,cUserMqtt,addr_mqtt_user);
        else
            print("---- vSend2board: cUserMqtt not change!\r\n");   
        end;
        -- for pass mqtt
        if(cPassMqtt ~= macroNOTCHANGE) or (cPassMqtt == "") then
            user_fs.vEraser_segment(FILE_CONFIG,addr_mqtt_pass,SIZE_MQTT_USER_PASS);
            user_fs.vWritefile(FILE_CONFIG,cPassMqtt,addr_mqtt_pass);
        else
            print("---- vSend2board: cPassMqtt not change!\r\n");
        end;
        -- turn on flag mqtt config and go to autorun.lua
        flag_mqtt_cfg = 1;
        return;
    else         
        process_data_server.vSendToGW(sv_packet, sv_eConfig);
    end; -- end if config mqtt

end;


-------------------------------------------------------------------------------
-- HandleMsgControlFromSV
-------------------------------------------------------------------------------
function process_data_server.HandleMsgControlFromSV(sv_packet)
        process_data_server.vSendToGW(sv_packet, sv_eControl);
end;

-------------------------------------------------------------------------------
-- HandleMsgRequestFromSV
-------------------------------------------------------------------------------
function process_data_server.HandleMsgRequestFromSV(sv_packet)
        --process_data_server.vSendToGW(sv_packet, sv_eRequest);
    local cSendToGW = string.format("%s,%s,%s,%s!",sv_packet[eTypePacketField_sv+1]
                                                ,sv_packet[eTypeDevField_sv+1]
                                                ,sv_packet[eIDEndDevice_sv+1]
                                                ,macroREQS_ALL);                                                     
    atctl.send(cSendToGW);
    print("---- vSendToGW:",cSendToGW,"\r\n"); 
end;

-------------------------------------------------------------------------------
--@function: package and send 2 board
--@para: sv_packet_t: packet after parser
--@return: null

--p/s: if dont want fix numberfield, pls add code get numberfield
-------------------------------------------------------------------------------
function process_data_server.vSendToGW(sv_packet_t, sv_eTypePacket)
	cSendToGW = "";		--string send to main board
	cArr_data = "";
	--------------------------------------
	-- process data for msg OTA, get file
	if ((sv_packet_t[2] == macroTYPE_PACKET_CONF) and 
		(sv_packet_t[10] == macroCONF_OTA) and 
		(sv_packet_t[11] ~= nil) and      -- filename.lua
		(string.find(sv_packet_t[11], ".lua")) > 0) then
			print("---- vSend2board: update file name ",sv_packet_t[11],"\r\n");		-- file name that need download
			local sv_pathfile = string.format("/SMF/%s",sv_packet_t[11]);
			local sv_localfile = string.format("c:\\\\%s",sv_packet_t[11]);
			print("---- sv_pathfile: ",sv_pathfile,"\r\n");
			print("---- sv_localfile: ",sv_localfile, "\r\n");
		if(user_ota.ucUpdateOTA(sv_pathfile,sv_localfile) == eSuccessed) then
			print("---- vSend2board: Reset Connectivity....\r\n");
			sio.send("AT+REBOOT\r\n");
		else
			process_data_server.SendtoServer(sv_eResponse, sv_Response_NOK);
			print("---- vSend2board: Update OTA again \r\n");
		end;
		return;
	end;
    -- for reset connectivity
    if ((sv_packet_t[2] == macroTYPE_PACKET_CONF) and 
        (sv_packet_t[10] == macroCONF_RST) ) then
            print("---- vSend2board: Msg reset network\r\n");
            sio.send("AT+REBOOT\r\n");
        return;
    end;
	--------------------------------------------------------------------------
	--------------------------------------------------------------------------
	--dong goi phan data field
	----------------------------------------
	for i=10,#sv_packet_t do               -- see format packet
        if(i == #sv_packet_t) then                  
            cArr_data = cArr_data..sv_packet_t[i];
        else
            if(i%2 == 0) then                           --name sensor
                cArr_data = cArr_data .. sv_packet_t[i];
                cArr_data = cArr_data .. ":";
            else                                        --value sensor
                cArr_data = cArr_data .. sv_packet_t[i];
                cArr_data = cArr_data .. ",";
            end;
        end;
    end;
    print("---- cArr_data = ", cArr_data, "\r\n");
	------- for msg config----------------
    --tat ca cac ban tin config,control,request(tru MQTT,RTC)                      
	if( (sv_eTypePacket == sv_eConfig) or 
	    (sv_eTypePacket == sv_eControl) ) then
        if(sv_packet_t[eIDDataField_sv+1] == macroCONF_RTC ) then
            cSendToGW = string.format("%s,%s,%s!",sv_packet_t[eTypePacketField_sv+1]
                                                 ,sv_packet_t[eTypeDevField_sv+1]
                                                 ,cArr_data);
        else
            cSendToGW = string.format("%s,%s,%s,%s!",sv_packet_t[eTypePacketField_sv+1]
                                                ,sv_packet_t[eTypeDevField_sv+1]
                                                ,sv_packet_t[eIDEndDevice_sv+1]
                                                ,cArr_data);
        end;
        atctl.send(cSendToGW);
        print("---- vSendToGW:",cSendToGW,"\r\n");
                                                   
	end;
	
	
end;
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
return process_data_server;