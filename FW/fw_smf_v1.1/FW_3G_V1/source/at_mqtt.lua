---------------------------AT_MQTT----------------------------------------------
--Date: 27.06.2017
--Author: Giangnv
--Project: AQI V1.3
-- Last Update: 02-Oct-2017
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--macroMQTT_START = "AT+CMQTTSTART\r\n"
local at_mqtt =  {}

-------------------------------------------------------------------------------
--Open the network
function at_mqtt.open()
	--mqtt start
	sio.send("AT+CMQTTSTART\r\n");
	vmsleep(200);
end;
-------------------------------------------------------------------------------
----stop the network
function at_mqtt.stop()
	--mqtt stop
	sio.send("AT+CMQTTSTOP\r\n");
	vmsleep(200);
end;
-------------------------------------------------------------------------------
-- client_index : int
-- strclientID: string
function at_mqtt.acquireClien(iClient_index, strClientID)
	local cmd = "";
	cmd = string.format("AT+CMQTTACCQ=%d,\"%s\"\r\n", iClient_index, strClientID);
	print("---- AT MQTT AcquireClien: ", cmd, "\r\n");
	sio.send(cmd);
	vmsleep(200);
end;
-------------------------------------------------------------------------------
--Connect to the MQTT server
function at_mqtt.connect(iClient_index, strServer_addr, strPort, strUser, strPwd)
	local cmd = "";
	iKeepalive_time = 60;  -- 60s
	iClean_session = 0;	   -- 0,1
	
	cmd = string.format("AT+CMQTTCONNECT=%d,\"tcp://%s:%s\",%d,%d,\"%s\",\"%s\"\r\n"
						,iClient_index
						,strServer_addr
						,strPort
						,iKeepalive_time
						,iClean_session
						,strUser
						,strPwd);
	print("---- AT MQTT Connect: ",cmd,"\r\n");
	print("---- AT MQTT Connect to ", strServer_addr, ":",strPort,"\r\n");
	sio.send(cmd);
	vmsleep(3000);
end;
-------------------------------------------------------------------------------
--iClient_index: int
--iTimeout: int, unit(sec)
function at_mqtt.disconnect(iClient_index, iTimeout)
	local cmd = "";
	cmd = string.format("AT+CMQTTDISC=%d,%d\r\n",iClient_index, iTimeout);
	print("---- AT MQTT Disconect: ", cmd, "\r\n");
	sio.send(cmd);
	vmsleep(100);
end;
-------------------------------------------------------------------------------
--iClient_index: int
--strTopicSubscribe: string
function at_mqtt.subscribe(iClient_index, strTopicSubscribe)
	local cmd = "";
	local iQoS = 1;
	iLenghTopic = string.len(strTopicSubscribe);
	cmd = string.format("AT+CMQTTSUB=%d,%d,%d\r\n",iClient_index,iLenghTopic,iQoS);
	sio.send(cmd);
	vmsleep(100);
	sio.send(strTopicSubscribe);
	vmsleep(500);
end;
-------------------------------------------------------------------------------
--iClient_index: int
--strTopicPublish: string
--strData: string
function at_mqtt.publish(iClient_index, strTopicPublish, strData)
	local iQoS = 1;
	local publish_interal = 60;		-- timeout when publish msg, unit(sec)
	--strData = strData.."\r\n";
	--set a topic for publish
	local cmd = "";
	iLenghTopic = string.len(strTopicPublish);
	cmd = string.format("AT+CMQTTTOPIC=%d,%d\r\n",iClient_index,iLenghTopic);
	sio.send(cmd);
	vmsleep(50);
	sio.send(strTopicPublish);
	vmsleep(50);
	--set payload of data for publish
	iLengthData = string.len(strData);
	cmd = "";
	cmd = string.format("AT+CMQTTPAYLOAD=%d,%d\r\n",iClient_index,iLengthData);
	sio.send(cmd);
	vmsleep(50);
	sio.send(strData);
	vmsleep(50);
	--publish msg
	cmd = "";
	cmd = string.format("AT+CMQTTPUB=%d,%d,%d\r\n",iClient_index, iQoS,publish_interal);
	sio.send(cmd);
	vmsleep(50);
end;
-------------------------------------------------------------------------------
--iClient_index: int
--response : OK
function at_mqtt.releaseclient(iClient_index)
	local cmd = "";
	cmd = string.format("AT+CMQTTREL=%d\r\n",iClient_index);
	sio.send(cmd);
	vmsleep(50);
end;
-------------------------------------------------------------------------------
--mqtt check connection
function at_mqtt.checkconnect(iClient_index)
	local cmd = "";
	local iTimeout = 120;	--120s
	cmd = string.format("AT+CMQTTDISC=%d,%d\r\n",iClient_index,iTimeout);
	sio.send(cmd);
	vmsleep(50);
end;
-------------------------------------------------------------------------------
function at_mqtt.getIPaddress(cDomainName)
	local cmd = "";
	cmd = string.format("AT+CDNSGIP= \"%s\"\r\n",cDomainName);
	print("---- getIPaddress: ",cmd);
	sio.send(cmd);
	vmsleep(50);
end;
-------------------------------------------------------------------------------

return at_mqtt




