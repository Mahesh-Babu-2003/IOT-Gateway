#ifndef _APPLICATION_H_
#define _APPLICATION_H_
#include "filesystem.h"

#include "lib/common.h"
#include <ArduinoJson.h>
#include <WiFi.h>
// #include <WiFiClientSecure.h> // For secure WiFi connection
#define WATCHDOG_TIMEOUT 20

#define MAX_CLIENTS 8

#include <Ticker.h> // For periodic reconnect attempts
#include <Preferences.h>
#include "PubSubClient.h"

#include <ESPAsyncWebServer.h>
extern AsyncWebServer server;

#include "modbusProcess.h"

#define WAKEUP_SW_IP 13
#define BUZZER_OP 14
#define RELAY_OP 15
#define LED1_OP 23
#define LED2_OP 26

#define ANALOG_1_IP 34
#define ANALOG_2_IP 35

#define CONNECTION_LED_OP LED1_OP
#define PUBLISH_LED_OP LED2_OP

#define RS485_EN_OP 18
#define RXPIN 17
#define TXPIN 16
#define TX_MODE_OP RS485_EN_OP

extern Preferences preferences;
extern SemaphoreHandle_t deviceMutex, snmpMutex;

#define MB_PARAM_NAME_LEN 20
#define MB_DEVICE_NAME_LEN 50
#define MAX_MB_REGISTER_COUNT 25
#define NO_OF_MODBUS_METERS 25

#define MAX_SNMP_REGISTER_COUNT 25
#define NO_OF_SNMP_DEVICES 2

#define MB_VALIDITY_VALUE 0xAAAA

#define MB_MAX_PACKET_SIZE 512
extern String LOG_CONFIGURATION_FILE;

#define LOCAL_MODE 0
#define TRANSPARNET_MODE 1
// typedef enum{
// 	LOCAL_MODE = 0, TRANSPRARENT_MODE
// }DEVICE_MODES;

#define IP_ADDRESS_LENGTH 20
#define COMMUNITY_LENGTH 30
#define SNMP_OID_LENGTH 50

void print_bytes_in_hex(const char *title, uint8_t *raw, uint16_t len);
class CommunicationResponse
{
public:
	bool result;
	String message;
	uint8_t bytes[MB_MAX_PACKET_SIZE];
	uint16_t byteCount;
	CommunicationResponse()
	{
		result = false;
		clear();
	}

	void clear()
	{
		message = "";
		byteCount = 0;
		memset(bytes, 0, sizeof(bytes));
	}

	static uint16_t GetSubarray(uint8_t *dest, int destIx, uint8_t *source, int srcIx, int count)
	{
		uint16_t byteCount = 0;
		while (count > 0)
		{
			dest[destIx] = source[srcIx];
			++destIx;
			++srcIx;
			--count;
			++byteCount;
		}
		return byteCount;
	}
};

class ApplicationRegister
{
public:
	uint16_t id;
	uint16_t address;
	char name[MB_PARAM_NAME_LEN];
	float factor = 1;
	union
	{
		struct
		{
			uint8_t data_type : 6; // 0
			uint8_t reg_type : 4;
			uint8_t byte_order : 2;
			uint8_t access_type : 2;
			uint8_t log : 1;
			uint8_t fetched : 1;
		};
		uint16_t settings;
	};
	REGISTER_VALUE value;
	ApplicationRegister()
	{
		resetValue();
	}

	void resetValue()
	{
		memset(&value, 0, sizeof(value));
	}

	uint8_t getByteCount(uint8_t regByteCount)
	{
		uint8_t byteCount = 0;
		if (regByteCount == 0)
		{
			regByteCount = 2;
		}
		if (reg_type == READ_COILS_ADDRESS || reg_type == READ_DISCRETE_IP_ADDRESS)
		{
			byteCount = 1;
		}
		else if ((regByteCount != 0) && (reg_type == READ_HOLDING_ADDRESS || reg_type == READ_INPUT_ADDRESS))
		{
			byteCount = dataTypeByteCount((DATA_TYPE)data_type) / regByteCount;
			// Serial.println((String)"Register: " + name + " " + data_type + " " + byteCount + regByteCount);
		}
		return byteCount;
	}

	void SetValueFromBuffer(uint8_t *data, uint8_t baseIx, uint8_t length)
	{
		copyBytesToValueAndUpdateByteOrder(
			(uint8_t *)&value, &data[baseIx],
			length, (DATA_TYPE)data_type, (Byte_Order)byte_order);
		// print_bytes_in_hex("src", &data[baseIx], length);
		// print_bytes_in_hex("dest", (uint8_t *)&value, length);
	}

	void setDeviceValue(String value, uint8_t isDevice = true)
	{
		if (isDevice == false && factor != 0 && factor != 1.0f)
		{
			double valueD = value.toDouble() / factor;
			value = valueD;
		}

		switch (data_type)
		{
		case BIT_TYPE:
		case UINT8_TYPE:
			this->value.u8_value = value.toInt();
			break;
		case INT8_TYPE:
			this->value.i8_value = value.toInt();
			break;
		case INT16_TYPE:
			this->value.i16_value = value.toInt();
			break;
		case UINT16_TYPE:
			this->value.u16_value = value.toInt();
			break;
		case INT32_TYPE:
			this->value.i32_value = value.toInt();
			break;
		case UINT32_TYPE:
			this->value.u32_value = value.toInt();
			break;
		case INT64_TYPE:
			this->value.i64_value = value.toInt();
			break;
		case UINT64_TYPE:
			this->value.u64_value = value.toInt();
			break;
		case FLOAT_TYPE:
			this->value.f_value = value.toFloat();
			break;
		case DOUBLE_TYPE:
			this->value.d_value = value.toDouble();
			break;
		case STRING_TYPE:
			break;
		default:
			break;
		}
	}

	void *getValuePointer()
	{
		void *ptr = nullptr;

		switch (data_type)
		{
		case BIT_TYPE:
		case UINT8_TYPE:
			ptr = &this->value.u8_value;
			break;
		case INT8_TYPE:
			ptr = &this->value.i8_value;
			break;
		case INT16_TYPE:
			ptr = &this->value.i16_value;
			break;
		case UINT16_TYPE:
			ptr = &this->value.u16_value;
			break;
		case INT32_TYPE:
			ptr = &this->value.i32_value;
			break;
		case UINT32_TYPE:
			ptr = &this->value.u32_value;
			break;
		case INT64_TYPE:
			ptr = &this->value.i64_value;
			break;
		case UINT64_TYPE:
			ptr = &this->value.u64_value;
			break;
		case FLOAT_TYPE:
			ptr = &this->value.f_value;
			break;
		case DOUBLE_TYPE:
			ptr = &this->value.d_value;
			break;
		case STRING_TYPE:
			break;
		default:
			break;
		}
		return ptr;
	}

	String getValue()
	{
		String value;
		switch (data_type)
		{
		case BIT_TYPE:
		case UINT8_TYPE:
			if (factor == 1)
			{
				value = this->value.u8_value;
			}
			else
			{
				value = this->value.u8_value * factor;
			}
			break;
		case INT8_TYPE:
			if (factor == 1)
			{
				value = this->value.i8_value;
			}
			else
			{
				value = this->value.i8_value * factor;
			}
			break;
		case INT16_TYPE:
			if (factor == 1)
			{
				value = this->value.i16_value;
			}
			else
			{
				value = this->value.i16_value * factor;
			}
			break;
		case UINT16_TYPE:
			if (factor == 1)
			{
				value = this->value.u16_value;
			}
			else
			{
				value = this->value.u16_value * factor;
			}
			break;
		case INT32_TYPE:
			if (factor == 1)
			{
				value = this->value.i32_value;
			}
			else
			{
				value = this->value.i32_value * factor;
			}
			break;
		case UINT32_TYPE:
			if (factor == 1)
			{
				value = this->value.u32_value;
			}
			else
			{
				value = this->value.u32_value * factor;
			}
			break;
		case INT64_TYPE:
			if (factor == 1)
			{
				value = this->value.i64_value;
			}
			else
			{
				value = this->value.i64_value * factor;
			}
			break;
		case UINT64_TYPE:
			if (factor == 1)
			{
				value = this->value.u64_value;
			}
			else
			{
				value = this->value.u64_value * factor;
			}
			break;
		case FLOAT_TYPE: 
			if (factor == 1){ 
				value = this->value.f_value * factor;
			}else{
				value = this->value.f_value;
			}
			break;
		case DOUBLE_TYPE:
			if (factor == 1){ 
				value = this->value.d_value * factor;
			}else{
				value = this->value.d_value;
			}
			break;
		case STRING_TYPE:
			break;
		default:
			break;
		}
		return value;
	}

	void printInfo()
	{
		Serial.println("##################################################################################");
		Serial.println((String) "Param Info: " + name + ", " + (String) "id: " + (String)id + ", " + (String) "address: " + (String)address + ", " + (String) "factor: " + (String)factor + ", " + (String) "data_type: " + (String)data_type + ", " + (String) "reg_type: " + (String)reg_type + ", " + (String) "byte_order: " + (String)byte_order + ", " + (String) "access_type: " + (String)access_type + ", " + (String) "log: " + (String)log);
		Serial.println("##################################################################################");
	}
};


class SnmpRegister: public ApplicationRegister{
	public:
	char oid[SNMP_OID_LENGTH]; 

};

class ApplicationDevice
{
public:
	uint64_t id;
	char name[MB_DEVICE_NAME_LEN];

	uint32_t syncPeriod;
	uint8_t isActive;
	uint8_t connected;
	uint32_t lastFetchTime; // fetch from device

	uint16_t deviceDelay;
	uint8_t isSynced;
	uint32_t lastSyncTime; // push to server
	uint32_t communicationLastCounter;
	int64_t configurationSyncTime;

	uint16_t validityBytes;
	void reset()
	{
		communicationLastCounter = 0;
	}
};

class SNMP_Device : public ApplicationDevice
{
public:
	char address[IP_ADDRESS_LENGTH];
	uint16_t port;
	char readCommunity[COMMUNITY_LENGTH];
	char writeCommunity[COMMUNITY_LENGTH];
	int snmpVersion;

	SnmpRegister snmpRegisters[MAX_SNMP_REGISTER_COUNT];
	uint8_t noOfRegisters;


	void printRegisterInfo(){
		for(uint8_t ix = 0; ix < noOfRegisters; ++ix){
			snmpRegisters[ix].printInfo();
		}
	}
	
	void printInfo()
	{
		Serial.println("*********************************************************************************");
		Serial.println((String) "Device Info: " + (String)name + ", " + (String) "id: " + (String)id + ", " + (String) "isActive: " + (String)isActive + ", " + (String) "noOfRegisters: " + (String)noOfRegisters + ", " + (String) "connected: " + (String)connected + ", " + (String) "syncPeriod: " + (String)syncPeriod + ", " + (String) "validityBytes: " + (String)validityBytes + ", " + (String) "ipaddress: " + (String)address + ", " + (String) "Port: " + (String)port + ", " + (String) "readCommunity: " + (String)readCommunity + ", " + (String) "writeCommunity: " + (String)writeCommunity + ", " + (String) "snmpVersion: " + (String)snmpVersion + ", " + (String) "noOfRegisters: " + (String)noOfRegisters);
		Serial.println("*********************************************************************************");
	}
};

class ModbusDevice : public ApplicationDevice
{
public:
	uint8_t address;
	uint32_t baud_rate;
	uint8_t data_bits;
	uint8_t parity;
	uint8_t stop_bits;
	uint8_t regByteCount;
	uint16_t timeOut;
	uint16_t registerDelay;
	ApplicationRegister mb_register[MAX_MB_REGISTER_COUNT];
	uint8_t noOfRegisters;

	void printRegisterInfo(){
		for(uint8_t ix = 0; ix < noOfRegisters; ++ix){
			mb_register[ix].printInfo();
		}
	}
	void printInfo()
	{
		Serial.println("*********************************************************************************");
		Serial.println((String) "Device Info: " + (String)name + ", " + (String) "id: " + (String)id + ", " + (String) "address: " + (String)address + ", " + (String) "baud_rate: " + (String)baud_rate + ", " + (String) "data_bits: " + (String)data_bits + ", " + (String) "parity: " + (String)parity + ", " + (String) "stop_bits: " + (String)stop_bits + ", " + (String) "regByteCount: " + (String)regByteCount + ", " + (String) "isActive: " + (String)isActive + ", " + (String) "noOfRegisters: " + (String)noOfRegisters + ", " + (String) "connected: " + (String)connected + ", " + (String) "timeOut: " + (String)timeOut + ", " + (String) "registerDelay: " + (String)registerDelay + ", " + (String) "deviceDelay: " + (String)deviceDelay + ", " + (String) "syncPeriod: " + (String)syncPeriod + ", " + (String) "validityBytes: " + (String)validityBytes);
		Serial.println("*********************************************************************************");
	}
};

class ApplicationState
{
public:
	char mac[16];

	ModbusDevice modbusDevices[NO_OF_MODBUS_METERS];
	short deviceCount;
	short currentModbusDeviceIx;
	short currentRegisterIx;

	SNMP_Device snmpDevices[NO_OF_SNMP_DEVICES];
	short currentSnmpDeviceIx;

	String hostname;
	String wifiSsid;
	String wifiPassword;

	
	String lastWifiSsid;
	String lastWifiPassword;

	String wifi_AP_Ssid;
	String wifi_AP_Password;

	int staticEnabled;
	String staticIp;
	String staticSubnet;
	String staticGateway;

	String adminPassword;

	String mqttServer;
	int mqttPort;
	String mqttUsername;
	String mqttPassword;

	
	String mqttServer_2;
	int mqttPort_2;
	String mqttUsername_2;
	String mqttPassword_2;

	uint32_t lastWifiAttemptTime;
	uint32_t lastMqttAttemptTime[2];
	uint32_t lastWifiAP_AttemptTime;

	uint32_t onTimer;

	uint32_t publishLedTime;
	uint32_t connectionBlinkTime;
	uint32_t slaveConnectionCounter[NO_OF_MODBUS_METERS];
	bool isSaveDeviceCOnfiguration;

	uint8_t gatewayMode;
	uint32_t baud_rate;
	uint8_t data_bits;
	uint8_t parity;
	uint8_t stop_bits;
	uint16_t serverPort;

	bool rebootRequire;
	uint32_t rebootDelay;

	bool isPressed;
	uint32_t lastPressedTime;

	uint32_t periodicDataTransmissionTime;

	uint16_t do_1_mode;
	uint16_t do_1_manual;

	uint16_t do_1_persistance_off;
	uint16_t do_1_persistance_on;
	uint64_t do_1_persistance_time;


	float ai_1_adc;
	float ai_2_adc;

	float ai_1_value;
	float ai_2_value;
	float do_1_value;

	String ai_1_formula;
	String ai_2_formula;
	String do_1_formula;

	uint8_t powerOnSwitch;
	
	ApplicationState()
	{
		memset(slaveConnectionCounter, 0, sizeof(slaveConnectionCounter));
	}
};

class ApplicationLog
{
public:
	uint32_t switchOnCounter;

	uint32_t slaveConnectionCounter[NO_OF_MODBUS_METERS];

	uint8_t wifiStatus;
	uint32_t wifiDisconnectionCounter;

	uint8_t mqttStatus[2];
	uint32_t mqttDisconnectionCounter[2];
	ApplicationLog()
	{
		wifiStatus = false;
		mqttStatus[0] = false;
		mqttStatus[1] = false;
	}
};
extern ApplicationState liveObj;
extern ApplicationLog logObj;

extern PubSubClient mqtt;
extern PubSubClient mqtt_2;

void initializeWifi();
void processWifi();
void saveWifiCredentials();

void loadDefaultNetworkConfiguration();

void load_application_log();

void initializeMqttTopics();
void load_modbus_devices();
void printAllDevicesInfo();
uint8_t saveModbusDevicesConfiguration();
void process_modbus_requests();

void mqttInit();
void saveMqttCredentials();

uint8_t addModbusDevice(ModbusDevice device);
uint8_t clearAllDevices();
uint8_t addModbusDeviceParameter(int deviceId, ApplicationRegister mbRegister);
uint8_t deleteModbusDevice(short deviceId);
uint8_t deleteModbusDeviceParameter(short deviceId, short paramId);

void initializeWebserver();
void processWebserver();

CommunicationResponse ProcessRegisterOrCoilReading(ModbusDevice *device, uint8_t address, uint8_t regByteCount, uint8_t functionCode, int startAddress, int noOfRegisters, bool crcRequire, bool tcpHeader, uint16_t transactionId);

void print_bytes_in_hex(const char *title, uint8_t *raw, uint16_t len);
void processLoops();

void application_delay(uint32_t delay);
String getDeviceRealtimeJson(uint32_t id, uint8_t deviceType);
void publishMessage(uint32_t deviceId, String sub_topic, const char *data, uint32_t count);
void publishGatewayMessage(String sub_topic, const char *data, uint32_t count);
void connectToWifi();
void prcoessMqttLoop();
void reconnectMQTT(PubSubClient* mqttClient, String server, int mqttPort, String username, String password);
ModbusDevice parseModbusJsonObject(JsonObject obj);
CommunicationResponse processFormulas();
CommunicationResponse processSaveFormulas(String body);
CommunicationResponse processConfigureParameter(String body);

void saveLogFile();

CommunicationResponse processAddModbusDeviceParameterBody(uint32_t deviceId, String body);
CommunicationResponse processDeleteModbusDeviceParameterBody(uint32_t deviceId, uint32_t parameterId);
CommunicationResponse processAddModbusDeviceBody(String body);
CommunicationResponse processDeleteModbusDeviceBody(uint32_t deviceId);
CommunicationResponse processModbusDeviceList();
CommunicationResponse processModbusDeviceParameterList(bool live, uint32_t deviceId);
CommunicationResponse processGatewayInformation();
CommunicationResponse processWifiSetup();
CommunicationResponse processSaveWifiSetup(String body);

void loadApplicationSettings();
void saveApplicationSettings();

bool isSoftAPEnabled();
bool isSoftAPStarted();

void enableWifiAcessPoint();
void disableWifiAccessPoint();
void connectToWifi();

void loadApplicationSettings();
void saveApplicationSettings();

void processTransparentLoop();
void terminateTransparentServer();
void initializeTransaparentServer();

CommunicationResponse processConfiguration();
CommunicationResponse processSaveConfiguration(String body);

void processAnalogAndDigital();

void saveFormulas();
void loadformulas();
void procesFormulas();
String computeExpression(String expression, double *reuslt);
void configureBleCommunication();

uint8_t saveSnmpDevicesConfiguration();
void load_snmp_devices();


uint8_t deleteSnmpDevice(short deviceId);
uint8_t addSnmpDevice(SNMP_Device device);
SNMP_Device parseSnmpJsonObject(JsonObject obj);
CommunicationResponse processAddSnmpDeviceBody(String body);
CommunicationResponse processDeleteSnmpDeviceBody(uint32_t deviceId);
CommunicationResponse processSnmpDeviceList();
CommunicationResponse processSnmpDeviceParameterList(bool live, uint32_t deviceId);
void process_snmp_requests();
CommunicationResponse modbusWriteRequest(String body);


bool sendMeterWriteRequest(uint8_t modbus_address, 
		uint8_t fcode,
		uint8_t data_type, uint8_t byte_order, uint16_t register_address,
		REGISTER_VALUE value);
#endif