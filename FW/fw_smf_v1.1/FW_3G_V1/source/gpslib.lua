-------------------------------------------------------------------------------
-- File name: gpslib.lua
-- Description: get gps from GPS/GSM...
-- Author: giangnv
-- Date: 15-Oct-2017
-- Project: AQI V1.3
-- Last Update: 15-Oct-2017
-------------------------------------------------------------------------------
eClose		= 0;
eStart		= 1;

-------------------------------------------------------------------------------
_eLat		= 0;
_eLong		= 1;
-------------------------------------------------------------------------------
local gps_function = {}
-------------------------------------------------------------------------------
function gps_function.eSetstate_gps(eState_gps)
	if(eState_gps == eStart) then		-- start 
		l_start = gps.gpsstart(1);		-- start hot gps
		if(l_start == true) then		-- start success
			print("---- Setstate_gps: GPS hot start success!\r\n");
			return eSuccessed;
		else
			print("---- Setstate_gps: GPS hot start failed!\r\n");
			return eFailed;
		end;
	elseif(eState_gps == eClose) then	-- close
		l_close = gps.gpsclose()
		if(l_close == true) then
			print("---- Setstate_gps: GPS close success!\r\n");
			return eSuccessed;
		else
			print("---- Setstate_gps: GPS close failed!\r\n");
			return eFailed;
		end;
	else
		return eFailed;
	end;
end;
-------------------------------------------------------------------------------
--@function: get GPS from GPS
--@
-------------------------------------------------------------------------------
function gps_function.cGetGPS()
	-- local cData_gps="";
	-- cData_gps = gps.gpsinfo();
	-- print("---- cGetGPS = ",cData_gps,"\r\n");
	sio.send("AT+CGPSINFO\r\n");
	--vmsleep(60000);
end;
-------------------------------------------------------------------------------
function gps_function.cGetGPSFromGSM()
	--local cmd = "";
	sio.send("AT+CASSISTLOC=1,1,\"en_GB\"\r\n");
	--vmsleep(15000);
end;
-------------------------------------------------------------------------------
-- lat: ddmm.mmmmmm
-- long: dddmm.mmmmmm
-------------------------------------------------------------------------------
function gps_function.convert_toDegree(cData_gps, _ePos)
	local strdegree1 = "";
	local strdegree1 = "";
	if(_ePos == _eLat) then
		strdegree1 = string.sub(cData_gps,1,2);
		strdegree2 = string.sub(cData_gps,3,string.len(cData_gps));
		local _iLatDegree = tonumber(strdegree1) + (tonumber(strdegree2)/60);
		print("---- convert_toDegree: strdegree1:",strdegree1,"\r\n");
		print("---- convert_toDegree: strdegree2:",strdegree2,"\r\n");
		print("---- convert_toDegree: _iLatDegree:",_iLatDegree,"\r\n");
		local cLat = "";
		cLat = string.format("%.6f",_iLatDegree);
		print("---- convert_toDegree: cLat:",cLat,"\r\n");
		return cLat;
	elseif(_ePos == _eLong) then
		strdegree1 = string.sub(cData_gps,1,3);
		strdegree2 = string.sub(cData_gps,4,string.len(cData_gps));
		local _iLongDegree = tonumber(strdegree1) + (tonumber(strdegree2)/60);
		print("---- convert_toDegree: strdegree1:",strdegree1,"\r\n");
		print("---- convert_toDegree: strdegree2:",strdegree2,"\r\n");
		print("---- convert_toDegree: _iLongDegree:",_iLongDegree,"\r\n");
		local cLong = "";
		cLong = string.format("%.6f",_iLongDegree);
		print("---- convert_toDegree: cLong:",cLong,"\r\n");
		return cLong;
	end;
end;
-------------------------------------------------------------------------------
return gps_function
-------------------------------------------------------------------------------