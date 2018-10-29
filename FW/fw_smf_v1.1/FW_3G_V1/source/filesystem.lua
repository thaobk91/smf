-------------------------------------------------------------------------------
-- File name: filesystem.lua
-- Description: store para config about server, mqtt topic, data,...
-- Author: giangnv
-- Date: 21/09/2017
-- Project: AQI V1.3
-- Last Update: 09-Oct-2017
-------------------------------------------------------------------------------
MAX_BYTE_READ = 512;
eError        = "error";
-------------------------------------------------------------------------------
---------store config parameter for MQTT---------------------------------------
cTimeUD 		=	"";								
cUserMqtt		=	"";									
cPassMqtt		=	"";									
cPhone_1		=	"";								
cPhone_2		=	"";								
cPhone_3		=	"";								
cPhone_4		=	"";								
cPhone_5		=	"";								
cServerADDR		=	"";							
cPortSV			=	"";									
-------------------------------------------------------------------------------
SIZE_TIME				= 2 ;
SIZE_PASS				= 32;
SIZE_USER				= 32;
SIZE_PHONE				= 22;
SIZE_MQTT_USER_PASS 	= 32;
SIZE_MQTT_PUB_SUB		= 64;
SIZE_ADDR_SV			= 64;
SIZE_PORT				= 7	;										
																						
-------------------------------------------------------------------------------
HAS_UPDATE		=	"1";
NO_UPDATE		=	"2";
USER			=	"admin";
PASS			= 	"12345"
FILE_CONFIG 	=	"c:\\config.txt";
-------------------------------------------------------------------------------
--define addr for file config 
addr_time =0;								--0  + len_time = 0 + 2 --> 0 to 2
addr_user = (addr_time +2 +1);				--2+1  + len_user = 3 +32 =35--> 3 to 35
addr_pass = (addr_user + 32 +1);			--35+1 + len_pass = 36 +32=68 --> 36 to 68
addr_phone_1 = 69;							--68+1 +len_addr_phone = 69 +22=91--> 69 to 91
addr_phone_2 = (addr_phone_1 +22 +1);		--91+1 +len_addr_phone = 92 +22=114-->92 to 114
addr_phone_3 = (addr_phone_2 +22 +1);		--115 to 137
addr_phone_4 = (addr_phone_3 +22 +1);		--138 to 160
addr_phone_5 = (addr_phone_4 +22 +1);		--161 to 183
addr_svaddr  = (addr_phone_5 +22 +1);		--len addr_addrsv = 64--> 184 to 248
addr_svport	 = (addr_svaddr  +64 +1);		--len port =6--> 249 to 255
addr_mqtt_user = (addr_svport + 6 +1);		--len mqtt_user = 32---> 256 to 288
addr_mqtt_pass = (addr_mqtt_user + 32 +1);	--len mqtt_pass = 32---> 289 to 321
addr_mqtt_sub= (addr_mqtt_pass + 32 +1); 	--len mqtt_sub	= 64---> 322 to 386
addr_mqtt_pub= (addr_mqtt_sub + 64 +1);		--len mqtt_pub = 64--> 387 to 409

--for file data log
addr_data1 = 1;							--define for backup data file
addr_data2 = addr_data1 + 512 + 1;      --
addr_data3 = addr_data2 + 512 +1;
-------------------------------------------------------------------------------
local filesystem = {}

-------------------------------------------------------------------------------
-- Function: read data from a filesystem
-- Para:	cFilename: name file
--			iReadByte: number of bytes is read, iReadByte = 5 ~ 5bytes
--			iAddress: address of data in filesystem. address is define by user
-- Return: a string contains data that is read.
-------------------------------------------------------------------------------
function filesystem.cReadfile(cFilename,iReadByte,iAddress)
	l_cReadcontent = "";
	l_cTemp = "";
	l_iIndex = 0;
	--Step 1: open file
	file = io.open(cFilename,"r");
	assert(file);
	--Step 2: Go to address need to read, from start of file
	file: seek("set",iAddress);
	--Step 3: read iReadByte and store in l_cReadcontent
	l_cTemp = file:read(iReadByte);
	if(l_cTemp == nil) then 
		print("---- no data to read\r\n");
		return eError;
	end;
	--Step 4: close file
	file:close();
	--Separate l_cTemp and get content. Because l_cTemp = data######--> token '#' is end data. 
	-- find first appear '#'
	i = string.find(l_cTemp,"#");
	if(i == nil) then				-- if dont find token end data(token '#')
		return l_cReadcontent;
	end;
	--sub string to get data
	l_cReadcontent = string.sub(l_cTemp,0,i-1);
	return l_cReadcontent;
end;
-------------------------------------------------------------------------------
-- Function: write data to  filesystem
-- Para:	cFilename: name file
--			cWriteData: data need to write.
--			iAddress: address in filesystem. address is define by user
-- Return: None
-------------------------------------------------------------------------------
function filesystem.vWritefile(cFilename,cWriteData,iAddress)
	
	cWriteData = cWriteData.."#";	-- add end data
	print("---- vWritefile: data write: ",cWriteData,"\r\n");
	--Step 1: open file
	file = io.open(cFilename,"w");
	assert(file);
	--Step 2: Go to address need to write, from start of file
	file: seek("set",iAddress);
	--Step 3: write data to filesystem
	l_cTemp = file:write(cWriteData);
	--Step 4: close file
	file:close();
end;
-------------------------------------------------------------------------------
-- Function: format a segment in filesystem
-- Para:	cFilename: name file
--			iAddress: address in filesystem. address is define by user
--			iLength: len of segment need to format
-- Return: None
-------------------------------------------------------------------------------
function filesystem.vEraser_segment(cFilename,iAddress,iLength)
	-- create a string to eraser, "########...."
	print("---- vEraser_segment: formating data......");
	l_cTemp = "";
	l_cTemp = string.rep("#",iLength);
	filesystem.vWritefile(cFilename,l_cTemp,iAddress);
end;
-------------------------------------------------------------------------------
-- Function: check data at addr in FS
-- Para:	cFilename: name file
--			iAddress: address in filesystem. address is define by user

-- Return: 0: if the addr has data in FS
--		  iAddress: if the addr hasnt data in FS
-- Note: only use for store data lost
-------------------------------------------------------------------------------
function filesystem.iCheck_data(cFilename,iAddress)
	iRet=0; 
	local l_cTemp = filesystem.cReadfile(cFilename,MAX_BYTE_READ,iAddress);
	print("---- read content: ",l_cTemp,"\r\n");
	if((string.len(l_cTemp) == 0) or (l_cTemp == eError)) then
		print("---- iCheck_data: this addr = ",iAddress," hasnt data\r\n");
		iRet = iAddress;
	else
		print("---- iCheck_data: This addr = ",iAddress," has data\r\n");
		iRet = 0;
	end;
	return iRet;
end;
-------------------------------------------------------------------------------
-- Function: store or load para config into variable respectively
-- Para:	cFilename: name file
-- Return: None
--------------------------------------------------------------------------------
function filesystem.vConfigFile(cFilename)
	-- Check file, whether the file exists or not
	file = io.open(cFilename,"r");
	if(file == nil) then 	-- if not exists
		print("---- vConfig: file not exists!\r\n");
		-- create and write data into File
		-- filesystem.vWritefile(cFilename,HAS_UPDATE,addr_time);
		-- vmsleep(50);
		cUserMqtt 		= MQTT_USER;
		cPassMqtt 		= MQTT_PWD;
		cServerADDR     = MQTT_ADDR_SV;
		cPortSV		    = MQTT_PORT;
		print("---- vConfig: cUserMqtt = ",cUserMqtt,"\r\n");
		print("---- vConfig: cPassMqtt = ",cPassMqtt,"\r\n");
		print("---- vConfig: cServerADDR = ",cServerADDR,"\r\n");
		print("---- vConfig: cPortSV = ",cPortSV,"\r\n");
		
		filesystem.vWritefile(cFilename,MQTT_USER,addr_user);
		vmsleep(50);
		filesystem.vWritefile(cFilename,MQTT_PWD,addr_pass);
		vmsleep(50);
		filesystem.vWritefile(cFilename,MQTT_ADDR_SV,addr_svaddr);
		vmsleep(50);
		filesystem.vWritefile(cFilename,MQTT_PORT,addr_svport);
		vmsleep(50);
		return;
	else
		file:close();
		-- load config parameter in buffer respectively
		print("---- vConfig: file has existed!\r\n");
		-- memset  parameter of mqtt
		cUserMqtt         = "";
		cPassMqtt         = "";
		cServerADDR       = "";
		cPortSV		      = "";
		
		cUserMqtt = filesystem.cReadfile(cFilename,SIZE_USER,addr_user);
		cPassMqtt = filesystem.cReadfile(cFilename,SIZE_PASS,addr_pass);
		cServerADDR = filesystem.cReadfile(cFilename,SIZE_ADDR_SV,addr_svaddr);
		cPortSV = filesystem.cReadfile(cFilename,SIZE_PORT,addr_svport);
		--print
		--print("---- vConfig: cTimeUD = ",cTimeUD,"\r\n");
		print("---- vConfig: cUserMqtt = ",cUserMqtt,"\r\n");
		print("---- vConfig: cPassMqtt = ",cPassMqtt,"\r\n");
		print("---- vConfig: cServerADDR = ",cServerADDR,"\r\n");
		print("---- vConfig: cPortSV = ",cPortSV,"\r\n");
	end;
end;
return filesystem