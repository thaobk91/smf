1. Update OTA
2. Update bản tin Reset 3G
{"TypePacket":"CONF","TypeDevice":"SMF_SENSN","DeviceIDEp":"12","DeviceIDGw":"12345678E","Data":{"RESET"}}
3. pub/sub default: 
MQTT_TOPIC_PUBLISH    =   "smartfarm/data/";
MQTT_TOPIC_SUBSCRIBE  =   "smartfarm/command/";

MQTT_USER       =   "SMF_TEST";
MQTT_PWD        =   "12348765";
MQTT_ADDR_SV    =   "m13.cloudmqtt.com";
MQTT_PORT       =   "10622";

================update 18 Oct 2018=============
1. sua loi ban tin data co 1 truong--> ban tin data add time vao la OK

2. ban tin ctrl--> Oke

3. them delay trc khi send--> oke

4. Get ID lien tuc --> oke

5. ban tin request so du-->ban tin CUSD

6. thay doi baurate --> oke