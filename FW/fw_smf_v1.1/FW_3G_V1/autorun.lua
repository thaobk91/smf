--------------------------------------------------------------------------------
-- Last Update 04-Aug-2018
--------------------------------------------------------------------------------
FILE_DATA     = "c:\\databackup.txt";
--------------------------------------------------------------------------------
--supported port for atctl.setport(...)
ATCTL_UART_PORT  = 1
ATCTL_MODEM_PORT = 2
ATCTL_USBAT_PORT = 3
--  -1 is used to release the port
ATCTL_INVALID_PORT = -1
-------------------------------------------------------------------------------
--*******************************my function************************************
-------------------------------------------------------------------------------
function vReset_system()
  sio.send("AT+REBOOT\r\n");
end;
-------------------------------------------------------------------------------
--
-------------------------------------------------------------------------------
function cGetIMEIConn(strATI)
    local strIMEI = "";
    --return start pos and end pos a string
    i,j = string.find(strATI, sio_rpt_hdr[SIO_ATI_RECVED]);
    --sub string to get IMEI
    if(i == nil and j == nil) then 
        print("---- cGetIMEIConn: dont get IMEI \r\n");
        return strIMEI;
    end;
    strIMEI = string.sub(strATI,j+2,j+16);
    return strIMEI;
end;
------------------------------------------------------------------------
--@Function: Get IP address
--@Para: string that rev after send cmd query IP (+CDNSGIP: 1,"domain","IP address")
--@Return: IP address
-------------------------------------------------------------------------------
function cGetIPAddressSV(cStrIPAddress)
  local struct_IPAddr ={};
  local strIPAddr = "";
  struct_IPAddr = user_board.Parser_msg(cStrIPAddress, '%s*["]%s*');
  strIPAddr = struct_IPAddr[4];
  print("---- cGetIPAddressSV: IP = ",strIPAddr,"\r\n");
  return strIPAddr;
end;
-------------------------------------------------------------------------------
--@Function: manage MQTT protocol
--@parameter: none
--@return: none
-------------------------------------------------------------------------------
function vMQTT_Process()
  print("---- vMQTT_Process.........\r\n");
  if(mqtt_state == MQTT_STATE_INIT) then
    print("---- Step 1: Openning network.........\r\n");
    status_network = 0;
    next_state = false;
    user_mqtt.open();
  elseif(mqtt_state == MQTT_SATE_QUERYIP) then
    print("---- Quering IP address server.....\r\n");
    next_state = false;
    user_mqtt.getIPaddress(cServerADDR);
  elseif(mqtt_state == MQTT_STATE_OPENNETWORK) then
    print("---- Step 2: Acquire a Client.........\r\n");
    if(string.len(cID_Conntivity) > 0) then
      next_state = false;
      user_mqtt.acquireClien(MQTT_INDEX_CLIENT,cID_Conntivity);
    else
      print("---- Step 2: Acquire a Client failed cause dont get ID connectivity\r\n");
      next_state = true;
    end;
  elseif(mqtt_state == MQTT_STATE_ACQUIRE_CLIENT) then
    print("---- Step 3: MQTT Connecting..........\r\n");
    next_state = false;
    user_mqtt.connect(MQTT_INDEX_CLIENT, g_strIP, cPortSV, cUserMqtt, cPassMqtt); ---xxx
    return;
  elseif(mqtt_state == MQTT_STATE_CONNECTED) then
    next_state = false;
    user_mqtt.subscribe(MQTT_INDEX_CLIENT, MQTT_TOPIC_SUBSCRIBE);
    print("---- Step 4: MQTT Subscribing.........\r\n");
  elseif(mqtt_state == MQTT_STATE_DISCONNECT) then
    status_network = 0;
    next_state = false;
    print("---- MQTT Disconnected to server\r\n");
    user_mqtt.disconnect(MQTT_INDEX_CLIENT, 120);
  elseif(mqtt_state == MQTT_STATE_SUBSCRIBE) then
    mqtt_state = MQTT_STATE_IDE;
    next_state = true;
  elseif(mqtt_state == MQTT_STATE_RELEASECLIENT) then
    print("---- MQTT Release a Client............\r\n");
    next_state = false;
    user_mqtt.releaseclient(MQTT_INDEX_CLIENT);
  elseif(mqtt_state == MQTT_STATE_PUBLISH) then
    next_state = false;
    user_mqtt.publish(MQTT_INDEX_CLIENT, MQTT_TOPIC_PUBLISH, cData_publish);  -- rev data from usb --> publish
    flag_publish = 1; -- check trang thai publish
    print("---- MQTT Publishing..............\r\n");
  elseif(mqtt_state == MQTT_STATE_STOPNETWORK) then
    print("---- MQTT: Stoping Network.........\r\n");
    status_network = 0;
    next_state = false;
    user_mqtt.stop();
  elseif(mqtt_state == MQTT_STATE_PROCESSDATA) then
    print("---- MQTT: Processing Data.........\r\n");
    -- call function process_data_server in here
    vHandle_cmdfromserver(cCmd_from_server);
    mqtt_state = MQTT_STATE_IDE;
    next_state = true;
    cCmd_from_server= "";
  elseif(mqtt_state == MQTT_STATE_IDE) then
    print("---- IDLE STATE.........\r\n");
    next_state = false;
  else
    -- Nothing 
    next_state = false;
  end;
  
end;
-------------------------------------------------------------------------------
--@function: process responses after send cmd
--@parameter: none
--@return: none
-------------------------------------------------------------------------------
function handle_received_sio_data(hdr_idx)
  if(hdr_idx == SIO_ATI_RECVED) then
        print("---- handle_received_sio_data: getting cID_Conntivity....\r\n");
        cID_Conntivity = cGetIMEIConn(sio_recved_string);
        print("---- cID_Conntivity: ",cID_Conntivity,"\r\n");
        return;
  elseif(hdr_idx == SIO_REAL_TIME) then
    print("---- handle_received_sio_data: Getting time....\r\n");
    --call function get time
    get_time_local = user_board.Get_time(sio_recved_string);
    print("---- handle_received_sio_data: send to board \r\n");
    print(get_time_local,"\r\n");
    atctl.send(get_time_local);
    get_time_local= "";
    return;
  elseif(hdr_idx == SIO_NEW_MSG) then
    print("---- handle_received_sio_data: Has new message...\r\n");
    iIndexSms = user_sms.iGetIndexSms(sio_recved_string);
    return;
  elseif(hdr_idx == SIO_OK_RECVED) then           -- received OK
    if(mqtt_state == MQTT_STATE_OPENNETWORK) then
      print("---- Step 2: Acquire a Client done!\r\n");
      mqtt_state = MQTT_STATE_ACQUIRE_CLIENT;
      next_state = true;
    elseif(mqtt_state == MQTT_STATE_RELEASECLIENT) then
      print("---- MQTT Release a Client done!\r\n");
      mqtt_state = MQTT_STATE_STOPNETWORK;
      next_state = true;
    else
      -- nothing..
    end;
    return;
  elseif(hdr_idx == SIO_SIM_FAIL) then
    print("---- SIMCARD Failed, waiting reinitialize simcard....\r\n");
    flag_start = 0;
    --sio.send("AT+CRFSIM\r\n");                -- reset simcard
  elseif(hdr_idx == SIO_SIM_READY) then
    print("---- SIMCARD READY, MQTT Process Start...\r\n");
    mqtt_state = MQTT_STATE_INIT;
    next_state = true;
    flag_start = 1;                     -- start program when simcard ready
  elseif(hdr_idx == SIO_QUERYIP) then             -- received sio get IP
    g_strIP = "";
    g_strIP = cGetIPAddressSV(sio_recved_string);
    print("---- Get IP server done: ",g_strIP,"\r\n");
    if(string.len(g_strIP) > 0) then
      mqtt_state = MQTT_STATE_OPENNETWORK;
      next_state = true;
    else
      -- coding: get 3 lan khong dc thi reset....
      print("---- Dont get IP server, Get IP server again\r\n");
      next_state = true;
    end;
    return;
  elseif (hdr_idx == SIO_OK_OPEN_NETWORK) then      --received +CMQTTSTART: 0
    print("---- Open Network done.....\r\n");
    mqtt_state = MQTT_SATE_QUERYIP;
    next_state = true;
    return;
   elseif (hdr_idx == SIO_MQTT_STOPNETWORK) then      -- received +CMQTTSTOP: 0
    print("---- MQTT Close Network done!\r\n");
    mqtt_state = MQTT_STATE_INIT;
    next_state = true;
    return;
  elseif(hdr_idx == SIO_MQTT_CONNECTED) then        -- received +CMQTTCONNECT: 0,0
    mqtt_state = MQTT_STATE_CONNECTED;
    next_state = true;
    status_network = 1;
    print("---- MQTT Connected......\r\n");
    return;
  elseif (hdr_idx == SIO_MQTT_DISCONNECTED) then      -- received +CMQTTDISC: 0,0
    mqtt_state = MQTT_STATE_RELEASECLIENT;
    next_state = true;
    status_network = 0;
    return;
  elseif(hdr_idx == SIO_MQTT_SUBSCRIBE) then        -- received +CMQTTSUB: 0,0
    mqtt_state = MQTT_STATE_SUBSCRIBE;
    flag_subscribe = 1;
    next_state = true;
    print("---- Step 4: MQTT subscribe topic: ", MQTT_TOPIC_SUBSCRIBE, "\r\n");
    return;
  elseif(hdr_idx == SIO_MQTT_RECVED) then         -- received +CMQTTRXPAYLOAD:
    print("---- handle_received_sio_data: Has data from ", MQTT_TOPIC_SUBSCRIBE, "\r\n");
    --process data from sv
    cCmd_from_server = cGetPacketFromServer(sio_recved_string);
    --print("---- handle_received_sio_data: Cmd from Server:",cCmd_from_server,"\r\n");
    return;
  elseif(hdr_idx == SIO_MQTT_CONNLOST) then       -- received +CMQTTCONNLOST: 0,1
    print("---- MQTT connection lost.....\r\n");
    mqtt_state = MQTT_STATE_RELEASECLIENT;
    next_state = true;
    status_network = 0;
    return;
  elseif(hdr_idx == SIO_MQTT_PUBLISH) then        -- received +CMQTTPUB: 0,0" --> pub done
    -- code send msg reponse to ensure msg has publish
    flag_publish = 0;
    mqtt_state = MQTT_STATE_IDE;
    icnt_state_publish = 0; -- rst cnt
    next_state = true;
    cData_publish = "";                 -- reset cData_publish
    print("---- MQTT Published to ", MQTT_TOPIC_PUBLISH, " done..\r\n");
    if(flag_rev_data == 1) then  -- response to GW
        user_board.HandleMessageResponse(macroRESP_OK);
        flag_rev_data = 0;
    end;
    return;
  elseif (hdr_idx == SIO_ERROR_RECVED) then       -- received ERROR
    --if( (mqtt_state == MQTT_STATE_INIT) or (mqtt_state == MQTT_STATE_ACQUIRE_CLIENT) ) then
    if(mqtt_state == MQTT_STATE_INIT) then
      mqtt_state = MQTT_STATE_RELEASECLIENT;
      next_state = true;
      return;
    elseif(mqtt_state == MQTT_STATE_RELEASECLIENT) then
      mqtt_state = MQTT_STATE_DISCONNECT;
      next_state = true;
      return;
    elseif( (mqtt_state == MQTT_STATE_OPENNETWORK) or 
        (mqtt_state == MQTT_STATE_CONNECTED) or
        (mqtt_state == MQTT_STATE_SUBSCRIBE) or
        (mqtt_state == MQTT_SATE_QUERYIP) or
        (mqtt_state == MQTT_STATE_STOPNETWORK) or
        (mqtt_state == MQTT_STATE_DISCONNECT) ) then
        print("---- rev ERROR, reset system\r\n");
        vReset_system();
    elseif(mqtt_state == MQTT_STATE_PUBLISH) then
      -- step1: check whether addr is empty
      if(user_fs.iCheck_data(FILE_DATA,addr_data1) == addr_data1) then    -- if hasnt data
        -- save in file data
        user_fs.vWritefile(FILE_DATA, cData_publish, addr_data1);
        vmsleep(50);
        vReset_system();
      else
        print("---- handle_received_sio_data: memory for data backup is full\r\n ");
        vReset_system();
      end;
    else
      return;
    end;
  else                          --handle other sio string...
    print("---- handle_received_sio_data: data dont process: ",sio_recved_string);
    return string.len(sio_recved_string);
  end;
  return 0;
end;
----------------------------------------------------------------------------
--@func: process data from GateWay
--@para: cbuff_uart: string that received from board
--@return: None
----------------------------------------------------------------------------
function vProcessDataUart(cbuff_uart)
  --- process case for msg WAMI
  local l_token = '%s*[,:!]%s*';
  local l_cWAMItoGW = "";
                     
  if(cbuff_uart == g_cWHO_AM_I) then
    if(status_network == 1) then
        l_cWAMItoGW = string.format("WAMI,%s,%s!", macroCONNECTIVITY_3G,macroRESP_OK);
    else
        l_cWAMItoGW = string.format("WAMI,%s,%s!", macroCONNECTIVITY_3G,macroRESP_NOK);
    end;
    atctl.send(l_cWAMItoGW);
    print("---- Send to GW: ",l_cWAMItoGW,"\r\n");
    return;
  end;
  
  --check packet, parser 
  if(user_board.ucCheckPacket(cbuff_uart) == eSuccessed) then
    cbuff_uart = string.gsub(cbuff_uart,"!","");  -- find '!' and replace ""
    packet_t = user_board.Parser_msg(cbuff_uart,l_token);
    -- Check type_packet
    if(packet_t[eTypePacketField] == macroTYPE_PACKET_DATA) then
      -- turn on flag rev data success
      flag_rev_data = 1;
      cData_publish = user_board.HandleMessageData(packet_t);
      return;
    elseif(packet_t[eTypePacketField] == macroTYPE_PACKET_CONF) then
      user_board.HandleMessageConfig(packet_t); -- process msg config ID GW
      return;
    elseif(packet_t[eTypePacketField] == macroTYPE_PACKET_REQS) then
      cData_publish = user_board.HandleMessageRequest(packet_t);
      return;
    elseif(packet_t[eTypePacketField] == macroTYPE_PACKET_RESP) then
      --user_board.HandleMessageResponse();
      return;
    else
      print("--- vProcessDataUart: TypePacket Failed\r\n");
    end;
    packet_t = {};  -- memset packet_t
  else
    print("--- vProcessDataUart: Failed ucCheckPacket\r\n");
  end;
  
end;
----------------------------------------------------------------------------
--@function: process data from server
--@para: cPacketFromSV: string after filter and parser {"TypePacket":.....}}
--@return: Null
----------------------------------------------------------------------------
function vHandle_cmdfromserver(message_from_sv)
  local l_token = '%s*[,:!]%s*';
  local cSmsDebug = "";
  local sv_xpacket={};  -- struct after parse msg from server
  -- step 1: check packet
  print("---- vHandle_cmdfromserver: Msg from server is", message_from_sv,"\r\n");
  
  if(user_server.ucCheckPacket(message_from_sv) == eSuccessed) then
    print("---- vHandle_cmdfromserver: Passed check  lenght packet\r\n");
    sv_xpacket = user_server.ucParsePacket(message_from_sv);
    
    -- Check type_packet vs type device
    if(user_server.ucCheckFieldPacket(sv_xpacket) == eSuccessed) then
      -- if pass, goi ham xu ly ung voi cac ban tin do
      if(sv_xpacket[eTypePacketField_sv+1] == macroTYPE_PACKET_CONF) then
        user_server.HandleMsgConfigFromSV(sv_xpacket);
      elseif(sv_xpacket[eTypePacketField_sv+1] == macroTYPE_PACKET_CTRL) then
        user_server.HandleMsgControlFromSV(sv_xpacket);
      elseif(sv_xpacket[eTypePacketField_sv+1] == macroTYPE_PACKET_REQS) then
        user_server.HandleMsgRequestFromSV(sv_xpacket);
      else
        -- nothing
        print("---- vHandle_cmdfromserver: typepacket not found...\r\n");
      end;
    else
        return;
    end; 
  else
    print("---- vHandle_cmdfromserver: Failed checkpacket\r\n");
    return;
  end;
  
end;
----------------------------------------------------------------------------
--@function: filter and parse to get content of msg from server
--@para: string from server
--@return: string packet after filter
--msg sio.rev
-- +CMQTTRXSTART: 0,13,122
-- +CMQTTRXTOPIC: 0,13
-- sub_mqtt_mc60
-- +CMQTTRXPAYLOAD: 0,122
-- {"TypePacket":.................}}
-- +CMQTTRXEND: 0
----------------------------------------------------------------------------
function cGetPacketFromServer(cDataFromServer)
  local l_cPacketFromServer= "";
  --delete space and \r,\n
  cDataFromServer = string.gsub(cDataFromServer,"%s+","");
  cDataFromServer = string.gsub(cDataFromServer,"\r","");
  cDataFromServer = string.gsub(cDataFromServer,"\n","");
  ---print("---- cGetPacketFromServer: after del: ",cDataFromServer,"\r\n");
  ------------Get payload----------------
  -- find string +CMQTTRXPAYLOAD:
  i,j = string.find(cDataFromServer,"+CMQTTRXPAYLOAD:");
  -- code process this case in here
  if(j == nil) then 
    print("---- cGetPacketFromServer Failed \r\n");
    return l_cPacketFromServer;   
  end;
  --find start pos of msg
  start_msg,n = string.find(cDataFromServer,"{\"TypePacket\"");
  -- get payload
  if(start_msg == nil) then 
    print("---- cGetPacketFromServer: Msg dont need process!\r\n");
    return l_cPacketFromServer;   
  end;
  cPayload = string.sub(cDataFromServer,j+1,start_msg-1);
  xtemp = user_board.Split(cPayload, ',');
  cPayload = "";
  cPayload= xtemp[2];
  print("---- PayLoad =",cPayload,"\r\n");
  ---------Get content msg---------------
  -- find end pos of msg
  end_msg,i = string.find(cDataFromServer, "+CMQTTRXEND:");
  if(end_msg == nil) then 
    print("---- cGetPacketFromServer: MQTT Rev failed!\r\n");
    return l_cPacketFromServer;   
  end;
  l_cPacketFromServer = string.sub(cDataFromServer,start_msg,end_msg-1);
  return l_cPacketFromServer;
end;
----------------------------------------------------------------------------
-- Interface between GW to 3G
----------------------------------------------------------------------------
function vTaskUart()
  print("---- Task uart called!\r\n");
  while( true ) do
  local evt, evt_param = waitevt(15000);
  if (evt >= 0) then
    if ( evt == ATCTL_EVENT ) then
      local atctl_found_matched_hdr_idx = -1;
      --step 1: rev data from uart
      local strdata_uart = "";
      strdata_uart = atctl.recv(5000);              -- rev data for uart
      if(strdata_uart) then
        local atctl_rev_data = "";
        atctl_rev_data = atctl_rev_data..strdata_uart;          
        print("---------------------------------------------\r\n");
        print("---- vTaskUart: ATCTL Rev string ",atctl_rev_data,"\r\n");
        print("---------------------------------------------\r\n");
        if(string.find(atctl_rev_data,'!') ~= nil) then
          vProcessDataUart(atctl_rev_data);
        else 
          atctl_rev_data = "";    -- fomat buff uart
          print("---- vTaskUart: MSG dont need process\r\n");
        end;
        atctl_rev_data = "";
      end;
    end;
  end;
  end;
end;
----------------------------------------------------------------------------
function vTaskTimer()
  vmstarttimer(TIMER_ID_DATA,800);  --800ms
  vmstarttimer(TIMER_ID_MQTT,500);
  vmstarttimer(TIMER_ID_GETIDGW,2000);  -- 2s
  vmstarttimer(TIMER_ID_CHECKFLAG,200);
  vmstarttimer(TIMER_ID_STATE_PUBLISH,600);
  vmstarttimer(TIMER_ID_GETIDCONNTIVITY,500);
  print("---- Task Timer called!\r\n");
  while (true) do
    local evt, evt_param = thread.waitevt(80000);  -- timeout 80s
    if(evt == TIMER_EVENT) then
      if(evt_param == TIMER_ID_GETIDCONNTIVITY) then
        if(string.len(cID_Conntivity) > 0) then -- ensure has got ID Conntivity
            vmstoptimer(TIMER_ID_GETIDCONNTIVITY);
            print("---- vTaskTimer: ID Connectivity get done!\n");
        else
            -- send AT cmd get IMEI 5360
            sio.send("ATI\r\n");
        end;
      elseif(evt_param == TIMER_ID_CHECKFLAG) then
        if(flag_mqtt_cfg == 1) then       -- has msg config mqtt: server, port, user, pass.
          mqtt_state = MQTT_STATE_DISCONNECT;
          next_state = true;
          flag_mqtt_cfg = 0;
        elseif((status_network == 1) and (flag_republish == 0)) then    -- ensure network and subscribe are done
          -- step 1: check if has data in file data backup
          if(user_fs.iCheck_data(FILE_DATA,addr_data1) == 0) then     -- addr 1 has data
            print("---- vTaskTimer: has data in file data....\r\n");
            local cData_republish = user_fs.cReadfile(FILE_DATA,512,addr_data1); -- read data at addr 1
            if ((cData_republish ~= eError) and (string.len(cData_republish) > 0))then
              -- read done --> republish
              print("---- vTaskTimer: Republising data........\r\n");
              user_mqtt.publish(MQTT_INDEX_CLIENT, MQTT_TOPIC_PUBLISH, cData_republish);  -- rev data from usb --> publish
              -- fomat data in file data
              user_fs.vEraser_segment(FILE_DATA,addr_data1,512);
            else
              flag_republish = 1;   -- khong co data republish
            end;
            vmsleep(2000);
          else      -- full slot to store data
            print("---- vTaskTimer: No data republish\r\n");
            flag_republish = 1;
          end;
        elseif(iIndexSms >= 0) then       -- has sms--> get id--> read sms
--          local sms_command = user_sms.cRead(iIndexSms);
--          -- process data...
--          -- find sms comamnd need process
--          local found_matched_hdr_idx = -1;
--          for idx, val in pairs(sio_sms_hdr) do   -- sio_sms_hdr
--            if(string.find(sms_command,val) ~= nil) then
--              found_matched_hdr_idx = idx;
--            break;
--            end;
--          end;
--          -- if has sms command need to process
--          if (found_matched_hdr_idx > 0) then
--            user_sms.vHandleSmsCommand(found_matched_hdr_idx,sms_command);
--          else
--            print("---- vTaskTimer: No sms command need to process... \r\n");
--          end;
          user_sms.vDeleteSms();
          iIndexSms = -1;
          sms_command = "";
        end;
      elseif(evt_param == TIMER_ID_GETIDGW) then
        -- send msg get id dev to board
        if(string.len(cID_GateWay) > 0) then
          print("---- vTaskTimer: ID GW get done!\r\n");
          vmstoptimer(TIMER_ID_GETIDGW);
          MQTT_TOPIC_PUBLISH = MQTT_TOPIC_PUBLISH..cID_GateWay;
          MQTT_TOPIC_SUBSCRIBE = MQTT_TOPIC_SUBSCRIBE..cID_GateWay;
          print("---- vTaskTimer: Create cMqttTopicPublish: ",MQTT_TOPIC_PUBLISH,"\r\n");
          print("---- vTaskTimer: Create cMqttTopicSubscribe: ",MQTT_TOPIC_SUBSCRIBE,"\r\n");
          flag_get_idgw = 1;
        else
          flag_cntgetid= flag_cntgetid+1;
          local msg_reqs_idgw = "";
          print("---- vTaskTimer: Getting ID GW:",flag_cntgetid,"\r\n");
          msg_reqs_idgw = string.format("REQS,%s,%s!", macroTYPE_DEVICE_GW, macroREQS_IDGW); --msg request id_device to board
          atctl.send(msg_reqs_idgw);
          if(flag_cntgetid > 10) then
            print("---- vTaskTimer: dont get ID GW, Connect to Publish and Subscribe default....\r\n");
            flag_get_idgw = 1;
            flag_cntgetid = 0;
            vmstoptimer(TIMER_ID_GETIDGW);
          end;
        end;
      elseif(evt_param == TIMER_ID_MQTT) then               -- manage  MQTT connect, sub
        if((next_state == true) and (flag_start == 1) and (flag_get_idgw == 1)) then  -- ensure get id done, sim ready
          vMQTT_Process();
        end;
      elseif(evt_param == TIMER_ID_STATE_PUBLISH) then
        if(flag_publish == 1) then                  -- publishing....
          icnt_state_publish = icnt_state_publish +1;
          if(icnt_state_publish == 40) then   -- publish failed
            -- step1: check whether addr is empty
            if(user_fs.iCheck_data(FILE_DATA,addr_data1) == addr_data1) then    -- if hasnt data
              -- save in file data
              print("---- write to file data \r\n");
              user_fs.vWritefile(FILE_DATA, cData_publish, addr_data1);
            else
              print("---- memory for data backup is full\r\n ");
            end;
              vmsleep(100);
              vReset_system();
          end;
        --else : tuong ung vs publish done 
        end;
      elseif(evt_param == TIMER_ID_DATA) then                   -- for publish msg
        if(flag_rst > 2) then
            print("---- Main: reset system\r\n");
            vmsleep(100);
            vReset_system();
        end;
        
        if(mqtt_state == MQTT_STATE_IDE) then
          count=0;
          flag_rst = 0;
          if(string.len(cData_publish) > 0) then
            print("---- vTaskTimer: Has data publish....\r\n");
            mqtt_state = MQTT_STATE_PUBLISH;
            next_state = true;  -- turn on mqtt process for publish
          end;
          if(string.len(cCmd_from_server) > 0) then
            print("---- vTaskTimer: Has data from server\r\n");
            mqtt_state = MQTT_STATE_PROCESSDATA;
            next_state = true;
          end;
        else
          if(count > 29) then 
            mqtt_state = MQTT_STATE_INIT;
            next_state = true;
            if(flag_start == 0) then                -- cause simcard failed
              sio.send("AT+CRFSIM\r\n");              -- reset simcard
            end;
            count = 0;
            flag_rst = flag_rst +1;                 -- for reset system
          else
            count = count+1;
          end;
         -- print("---- MAIN: MQTT: count = ", count, "\r\n");
        end;
      else
        -- nothing....
      end;  -- end if (evt_param)
    end;  --end if(evt == TIMER_EVENT)
  end;  --end while
end;
----------------------------------------------------------------------------
-- Interface between GSM to 3G sim
----------------------------------------------------------------------------
function vTaskSIO_DATA()  
  print("---- vTaskSIO_DATA called!\r\n");
  while (true) do
    local evt, evt_param = thread.waitevt(15000);
    if(evt == SIO_RCVD_EVENT) then
      local found_matched_hdr_idx = -1;
      local rsp = "";
      rsp = sio.recv(5000);                   --rev data from sio
      if (rsp) then
        print("---------------------------------------------\r\n");
        sio_recved_string = sio_recved_string..rsp;           -- noi string
        print("---- vTaskSIO_DATA: Rev SIO string ",sio_recved_string,"\r\n");
        print("---------------------------------------------\r\n");
        --handle all the received string
        while (true) do
          found_matched_hdr_idx = -1;
          if (string.len(sio_recved_string) > 0) then
          for idx, val in pairs(sio_rpt_hdr) do
            if(string.find(sio_recved_string,val) ~= nil) then
            found_matched_hdr_idx = idx;
            break;
            end;
          end;
          end;
          --handle the found hdr
          if (found_matched_hdr_idx > 0) then
          handle_received_sio_data(found_matched_hdr_idx);
          sio_recved_string = "";
          else
          print("---- vTaskSIO_DATA: MSG dont need process\r\n");
          sio_recved_string = "";
          break;
          end;
        end;  -- end while
      else
        print("---- vTaskSIO_DATA: Hasnt data from sio..\r\n");
      end;  --end of handling the received string
    end;
  end;
end;            
----------------------------------------------------------------------------
--------------------------------My Define-----------------------------------
----------------------------------------------------------------------------
--const values for event id, these are defined in the lua task on the module.
GPIO_EVENT    = 0;
UART_EVENT    = 1;
KEYPAD_EVENT  = 2;
USB_EVENT     = 3;
AUDIO_EVENT   = 4;
SMS_EVENT   = 26;
TIMER_EVENT   = 28;
SIO_RCVD_EVENT  = 29;
ATCTL_EVENT   = 30;
VOICECALL_EVENT = 21;

------------------------------------------------------------------------------
--all possible sio report data headers
--enum value
SIO_OK_OPEN_NETWORK           = 1;
SIO_MQTT_CONNECTED          = 2;
SIO_MQTT_DISCONNECTED       = 3;
SIO_MQTT_SUBSCRIBE          = 4;
SIO_MQTT_PUBLISH          = 5;
SIO_ERROR_PUBLISH         = 6;
SIO_REAL_TIME           = 7;
SIO_ATI_RECVED            = 8;
SIO_MQTT_RECVED           = 9;
SIO_MQTT_STOPNETWORK        = 10;
SIO_MQTT_CONNLOST         = 11;
SIO_QUERYIP             = 12; 
SIO_SIM_FAIL            = 13;
SIO_SIM_READY           = 14;
SIO_MISS_CALL           = 15;
SIO_NEW_MSG             = 16; 
SIO_FROM_GSM            = 17;           -- data gps from gsm
SIO_FROM_GPS            = 18;           -- data gps from gps
SIO_ERROR_RECVED                    = 19;           -- remember put OK in end pos
SIO_OK_RECVED                       = SIO_ERROR_RECVED +1;    -- remember put OK in end pos

------------------------------------------------------------------------------
-- declare for sio array
------------------------------------------------------------------------------
sio_rpt_hdr = {};     --declare an array
sio_rpt_hdr[SIO_OK_RECVED]                        = "OK";
sio_rpt_hdr[SIO_ERROR_RECVED]                     = "ERROR";
sio_rpt_hdr[SIO_OK_OPEN_NETWORK]                    = "+CMQTTSTART: 0";
sio_rpt_hdr[SIO_MQTT_CONNECTED]                   = "+CMQTTCONNECT: 0,0";
sio_rpt_hdr[SIO_MQTT_DISCONNECTED]          = "+CMQTTDISC: 0,0";
sio_rpt_hdr[SIO_MQTT_SUBSCRIBE]                   = "+CMQTTSUB: 0,0";
sio_rpt_hdr[SIO_MQTT_CONNLOST]                    = "+CMQTTCONNLOST:";
sio_rpt_hdr[SIO_MQTT_PUBLISH]                     = "+CMQTTPUB: 0,0";
sio_rpt_hdr[SIO_ERROR_PUBLISH]                    = "\r\nERROR\r\nNO CARRIER\r\n";  -- error publish
sio_rpt_hdr[SIO_REAL_TIME]              = "+CCLK:";
sio_rpt_hdr[SIO_ATI_RECVED]             = "IMEI:";              -- msg infomation sim 5360
sio_rpt_hdr[SIO_MQTT_RECVED]            = "+CMQTTRXPAYLOAD:";       -- has data from server
sio_rpt_hdr[SIO_MQTT_STOPNETWORK]         = "+CMQTTSTOP: 0" ;
sio_rpt_hdr[SIO_QUERYIP]              = "+CDNSGIP: 1" ;         -- query IP address of domain server
sio_rpt_hdr[SIO_SIM_FAIL]             = "+SIMCARD: NOT AVAILABLE";    -- simcard failed
sio_rpt_hdr[SIO_SIM_READY]              = "SMS DONE";           -- simcard ready
sio_rpt_hdr[SIO_NEW_MSG]              = "+CMTI";              -- has new msg
sio_rpt_hdr[SIO_FROM_GPS]             = "CGPSINFO";           -- data gps from gsm
sio_rpt_hdr[SIO_FROM_GSM]             = "CASSISTLOC";           -- data gps from gps
sio_rpt_hdr[SIO_MISS_CALL]              = "MISSED_CALL:"          -- has a miss call
------------------------------------------------------------------------------
------------------------------------------------------------------------------
--define state MQTT
MQTT_STATE_INIT             = 0;
MQTT_STATE_OPENNETWORK      = 1;
MQTT_STATE_ACQUIRE_CLIENT   = 2;
MQTT_STATE_CONNECTED        = 3;
MQTT_STATE_SUBSCRIBE        = 4;
MQTT_STATE_PUBLISH          = 5;
MQTT_STATE_DISCONNECT       = 6;
MQTT_STATE_STOPNETWORK      = 7;
MQTT_STATE_RELEASECLIENT    = 8;
MQTT_STATE_PROCESSDATA      = 9;
MQTT_SATE_QUERYIP           = 10;
MQTT_STATE_IDE              = 11;
STATE_GETGPS                = 12;
-------------------------------------------------------------------------------
--MQTT DEFINE
--MQTT_TOPIC_PUBLISH    =   "smartfarm/data/";
--MQTT_TOPIC_SUBSCRIBE  =   "smartfarm/command/";

--user for first config
--MQTT_USER       =   "SMF_TEST";
--MQTT_PWD        =   "12348765";
--MQTT_ADDR_SV    =   "m13.cloudmqtt.com";
--MQTT_PORT       =   "10622";
-------------------------------------------------------------------------------
--MQTT DEFINE
MQTT_TOPIC_PUBLISH    =   "smartfarm/data/";
MQTT_TOPIC_SUBSCRIBE    =   "smartfarm/command/";
----
------user for first config
MQTT_USER       =   "admin";
MQTT_PWD        =   "20051926";
MQTT_ADDR_SV      =   "broker.hosco.com.vn";
MQTT_PORT       =   "1883";
------------------------------------------
MQTT_INDEX_CLIENT = 0;
----------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
--DATA STATE DEFINE
DATA_STATE_CLOSE        = 1;
DATA_STATE_OPEN         = 2;
DATA_STATE_DISCONNECT   = 3;
DATA_STATE_CONNECT      = 4;
DATA_STATE_SUBSCRIBE    = 5;
DATA_STATE_PUBLISH      = 6;
DATA_STATE_IDLE         = 7;
-------------------------------------------------------------------------------
-- TIMER_ID define
TIMER_ID_MQTT               = 0;
TIMER_ID_DATA               = 1;
TIMER_ID_GETIDGW            = 2;
TIMER_ID_GETIDCONNTIVITY    = 3;
TIMER_ID_CHECKFLAG          = 4;
TIMER_ID_STATE_PUBLISH      = 5;
--------------------------------------------------------------------------------
-- define priority thread
LOW_PRIORITY = 1;
MEDIUM_PRIORITY = 2;
HIGH_PRIORITY = 3;
---------------------------------------------------------------------------------
--*************user declare  global variable************************************
--------------------------------------------------------------------------------
sio_recved_string = "";     -- data rev from sio
atctl_rev_data= "";       -- store data from uart
mqtt_state = MQTT_STATE_INIT; -- init state MQTT
count = 0;            -- manage MQTT state
next_state = true;        -- switch  MQTT state

number_getgps = 0;        -- count get gps, max = 10;
icnt_state_publish = 0;     -- count publish number, if icnt_state_publish = 20 without publish done --> failed

cID_GateWay = "";       -- store ID device of MCU
cID_Conntivity = "";      -- store ID Conntivity -- IMEI 5360
status_network  = 0;      -- network connect --> status_network = 1;
                -- network disconnect --> status_network = 0;
                

flag_mqtt_cfg   = 0;
flag_ota        = 0;
flag_start      = -1;
flag_publish    = 0;
flag_rst        = 0;      -- for reset system
flag_cntgetid   = 0;
flag_republish  = 0;
flag_subscribe  = 0;      -- for republish
flag_get_idgw   = 0;
flag_rev_data   = 0;       -- for rev DATA msg

Packet_t = {};          -- init packet_t = nil.
--Packet_t = {type_packet, id_device, type_device, number_field, arr_data}; -- struct packet from core board
-- Packet_t: table(struct)
-- Packet_t[0-4]: type_packet, iddevice, type_device, leng_data_field
-- Packet_t[5..]: data_field
cIPAdress = "";         -- query IP from domain server
g_strIP   = "";         -- IP server.
cData_publish = "";       -- store data for publish to server
cPayload = "";          -- string store length data received from server
cCmd_from_server= "";     -- cmd from server send to connectivity

-- for index sms
iIndexSms   = -1;   -- for read sms 

-- for msg WAMI
g_cWHO_AM_I = "WAMI!";
----------------------------------------------------------------------------------------------------------------
--*******************************Start Program***********************************
-------------------------------------------------------------------------------------------------------------------
printdir(1);    -- turn of debug via print function.
--ATTENTION: at+catr and atctl.setport SHOULDN'T be set to the same port, or else the unsolidated result may be sent to the ATCTL port.
atctl.setport(ATCTL_UART_PORT);
print("\r\n-------------------3G_GW Version 1.0 Last Update 12-Aug-2018-----------------------\r\n");
sio.send("ATE0\r\n");   -- echo off
vmsleep(200);
sio.send("AT+CTZU=1\r\n");  -- turn on auto update time via NITZ
vmsleep(200);
------------------------------------------------------------------------------
-- include file
user_mqtt = require ("at_mqtt");  
user_board = require ("parser_data_boardsensor");
user_server = require ("process_data_server");
user_ota = require ("ota");
user_fs = require ("filesystem");
user_sms = require ("smslib");
-------------------------------------------------------------------------------
-- write or load config parameters
user_fs.vConfigFile(FILE_CONFIG);
vmsleep(1000);
-- create file data backup
filehl = io.open(FILE_DATA,"w");
assert(filehl);
filehl:close();
vmsleep(1000);
-- config sms
-- del sms
--user_sms.vConfigSms();
--user_sms.vDeleteSms();
--vmsleep(1000);
--user_sms.eSend("0972156484","SmartFarm");
--vmsleep(1000);
-------------------------------------------------------------------------------
-- create timer to manage tasks
-- the minimum value of the second parameter for vmstarttimer(...) is 20ms
-- the supported maximum 30 timers
ID_vTaskUart = thread.create(vTaskUart);
ID_vTaskSIO_DATA = thread.create(vTaskSIO_DATA);
ID_vTaskTimer = thread.create(vTaskTimer);
---------------------------
-- set priority
thread.setpri(ID_vTaskUart, MEDIUM_PRIORITY);
thread.setpri(ID_vTaskSIO_DATA, HIGH_PRIORITY);
thread.setpri(ID_vTaskTimer, MEDIUM_PRIORITY);
----------------------------

thread.run(ID_vTaskTimer);
thread.run(ID_vTaskUart);
thread.run(ID_vTaskSIO_DATA);

while ( true ) do
  local evt, evt_param = waitevt(15000);
  if (evt >= 0) then
    if (evt == TIMER_EVENT ) then
    print("--- Main Task: Go to timer task \r\n");
    thread.setevt(ID_vTaskTimer,TIMER_EVENT);
    elseif (evt == ATCTL_EVENT) then                  -- uart evt, not relate to timer evt
    print("--- Main Task: Go to uart task \r\n");
    thread.setevt(ID_vTaskUart,ATCTL_EVENT);
    elseif(evt == SIO_RCVD_EVENT) then                  -- print out port usb at  
    print("--- Main Task: Go to sio task \r\n");
    thread.setevt(ID_vTaskSIO_DATA,SIO_RCVD_EVENT);
  elseif (evt == VOICECALL_EVENT) then
    print("---- Main Task: A call is coming.....\r\n");
    voice_call.hangup(nil);       -- try hangup all call
    --user_sms.vDeleteSms();
  else
    end;  --end if event        
  end;    --end if event > 0
end;      --end while
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
--**********************************end code***********************************
-------------------------------------------------------------------------------