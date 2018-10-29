-------------------------------------------------------------------------------
-- Getfile and Putfile FTP server 3G simcom 5360
-- Author: giangnv
-- Date: 06-Sep-2017
-- Project: Enviroment AQI V1.3
-- Last update: 09-NOV-2017
-------------------------------------------------------------------------------
local fota =  {};
-------------------------------------------------------------------------------
-- DEFINE FOR SERVER FTP
macroOTA_PDPAPN = "cmnet";
macroOTA_ADDRSERVER = "ftp.drivehq.com";
macroOTA_PORT = 21;
macroOTA_USER = "giang0972156";
macroOTA_PASS = "123456789_";
macroOTA_PATHFILE = "/3G/fota.lua";			-- Path file in server
macroOTA_LOCALFILE = "c:\\autorun.lua";		-- Path and file name save into efs
macroOTA_PASSIVE = 1;

-- for upload file
macroUplocal_file = "c:\\test.txt";
macroOTA_PATHFILEUPLOAD = "/test_SangVV/test.txt";
-------------------------------------------------------------------------------
--@Func: 	get file from FTP server
--@Para:	cPathfile: locate of file that need download "/3G/filename.lua"
--@para:	cLocalfile: locate of file after download, store in EFS of 3G "c:\\filename.lua"
--@Return:	eSuccessed if download successed!
--			eFailed if download failed!
------------------------------------------------------
-- CHu y: Voi ban AQI V1.3 dang trien khai o PQ, can chuyen ham nay ra autorun file roi OTA. sau do moi download ddc cac file khac ve
-------------------------------------------------------------------------------
function fota.ucUpdateOTA(cPathfile, cLocalfile)
	local ret = -1;
	local cmd = "";
	-- Step 1: send cmd to set PDP APN
	cmd = string.format("AT+CGSOCKCONT=1,\"IP\",\"%s\"\r\n",macroOTA_PDPAPN);
	print("---- ucUpdateOTA: Set active PDP context's..\r\n");
	print(cmd);
	sio.send(cmd);
	vmsleep(1000);
	-- Step 2: update file
	print("---- ucUpdateOTA: Start get file from server \r\n");
	--ret = ftp.simpget(macroOTA_ADDRSERVER, macroOTA_PORT, macroOTA_USER, macroOTA_PASS, macroOTA_PATHFILE, macroOTA_LOCALFILE, macroOTA_PASSIVE);
	ret = ftp.simpget(macroOTA_ADDRSERVER, macroOTA_PORT, macroOTA_USER, macroOTA_PASS, cPathfile, cLocalfile, macroOTA_PASSIVE);
	print("---- MYDEBUG: ret =",ret,"\r\n");
	if(ret == 0) then
      print("---- ucUpdateOTA: Update Successed!\r\n");
	  return eSuccessed;
    else
      print("---- ucUpdateOTA: Update failed! \r\n");
      return eFailed;
    end;
end;
--------------------------------------------------------------------
function fota.putfile(cFileput)
	local ret = -1;
	cmd = "";
	-- Step 1: send cmd to set PDP APN
	cmd = string.format("AT+CGSOCKCONT=1,\"IP\",\"%s\"\r\n",macroOTA_PDPAPN);
	print("---- ucUpdateOTA: Set active PDP context's..\r\n");
	print(cmd);
	sio.send(cmd);
	vmsleep(1000);
	-- Step 2: put file to ftp server
	print("---- fota.putfile: Start put file to ftp server \r\n");
	ret = ftp.simpput(macroOTA_ADDRSERVER,macroOTA_PORT,macroOTA_USER,macroOTA_PASS,cFileput,macroUplocal_file,macroOTA_PASSIVE);
	if(ret == 0) then
      print("---- ucUpdateOTA: Send Successed!\r\n");
	  return eSuccessed;
    else
      print("---- ucUpdateOTA: Send failed! \r\n");
      return eFailed;
    end;
end;
--------------------------------------------------------------------
return fota;












