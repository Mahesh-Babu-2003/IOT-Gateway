#include "application.h"
#include <esp_task_wdt.h>
#include <WiFi.h> // ESP32 Core WiFi Library
#include <WiFiUdp.h>

#if(1)
#include <Arduino_SNMP_Manager.h>

WiFiUDP udp;        
SNMPManager snmp = SNMPManager(); // Starts an SNMPManager to listen to replies to get-requests
SNMPGet snmpRequest = SNMPGet("", 0); // Starts an SNMPGet instance to send requests

void initializeSnmpManager(){
    snmp.setUDP(&udp); // give snmp a pointer to the UDP object
    snmp.begin();      // start the SNMP Manager
}

CommandStatus sendSNMPRequest(SNMP_Device *deviceObj)
{              // UDP object used to send and receive packets
    CommandStatus status = COMMAND_OK;
    IPAddress ipAddress;
    // Starts an SNMPGet instance to send requests

    if ((WiFi.status() == WL_CONNECTED || WiFi.getMode() != WIFI_MODE_STA) && deviceObj->noOfRegisters > 0 && ipAddress.fromString(deviceObj->address))
    {
        
        snmp._community = deviceObj->readCommunity;
        // SNMPGet snmpRequest = SNMPGet(deviceObj->readCommunity, deviceObj->snmpVersion == 1?0:1);
        snmpRequest._community = deviceObj->readCommunity;
        snmpRequest._version = deviceObj->snmpVersion == 1?0:1;
        REGISTER_VALUE actualValue;

        actualValue = deviceObj->snmpRegisters[0].value;
        // deviceObj->snmpRegisters[0].value.i8_value = -1; 

        for (uint8_t ix = 0; ix < deviceObj->noOfRegisters; ++ix)
        {
            SnmpRegister *registerObj = &deviceObj->snmpRegisters[ix];
            // registerObj->data_type = 8;
            if (registerObj->data_type <= INT32_TYPE)
            {
                // Serial.println((String)"Snmp Register: " + registerObj->name + " " + (String)registerObj->data_type);
                snmpRequest.addOIDPointer(snmp.addIntegerHandler(ipAddress, registerObj->oid, (int *)registerObj->getValuePointer()));
            }
            else if (registerObj->data_type <= UINT32_TYPE)
            {
                snmpRequest.addOIDPointer(snmp.addCounter32Handler(ipAddress, registerObj->oid, (uint32_t *)registerObj->getValuePointer()));
            }
            else if (registerObj->data_type <= INT64_TYPE)
            {
                snmpRequest.addOIDPointer(snmp.addTimestampHandler(ipAddress, registerObj->oid, (uint32_t *)registerObj->getValuePointer()));
            }
            else if (registerObj->data_type <= UINT64_TYPE)
            {
                snmpRequest.addOIDPointer(snmp.addCounter64Handler(ipAddress, registerObj->oid, (uint64_t *)registerObj->getValuePointer()));
            }
            else if (registerObj->data_type <= FLOAT_TYPE)
            {
                snmpRequest.addOIDPointer(snmp.addFloatHandler(ipAddress, registerObj->oid, (float *)registerObj->getValuePointer()));
            }
        }

        // Serial.println((String) "sendSNMPRequest - target: " + (String)deviceObj->id + " " + deviceObj->address + " " + ipAddress.toString()
        // + "  " + (String)esp_get_free_heap_size() + " bytes");
        snmpRequest.setIP(ipAddress); // IP of the listening MCU
        snmpRequest.setUDP(&udp);
        snmpRequest.setRequestID(rand() % 5555);
        snmpRequest.setPort(deviceObj->port);
        snmpRequest.sendTo(ipAddress);
 
        int waitTime= 0;
        bool status = false;
        while ( waitTime < 1000 && status == false)
        {
            delay(1);
            ++waitTime;
            status = snmp.loop();
            esp_task_wdt_reset();
        } 

        // for (uint8_t ix = 0; ix < deviceObj->noOfRegisters; ++ix){
        //     SnmpRegister *registerObj = &deviceObj->snmpRegisters[ix];
        //     Serial.println( (String)"ipAddress: " + ipAddress.toString() + " " + (String)deviceObj->port 
        //     + (String)" Snmp value: " + registerObj->name +
        //     " " + (String)registerObj->data_type + " " + registerObj->getValue() + " " + registerObj->factor );
        // }
        if(status == false){
            status = COMMAND_COMMUNICATION_ERROR;   
            deviceObj->snmpRegisters[0].value = actualValue;
        }
        snmpRequest.clearOIDList();
    }
    return status;
}
#else
CommandStatus sendSNMPRequest(SNMP_Device *deviceObj)
{
    CommandStatus status = COMMAND_OK;

    return status;
}
#endif

String SNMP_DEVICES_CONFIGURATION_BIN = "/SNMP_FILE";
String BKUP_SNMP_DEVICES_CONFIGURATION_BIN = "/BKUP_SNMP_FILE";

void initializeSnmpDeviceInfo()
{

    Serial.println((String) "Load SNMP Devices " + (String)liveObj.deviceCount);

    for (uint8_t ix = 0; ix < NO_OF_SNMP_DEVICES; ++ix)
    {
        if (liveObj.snmpDevices[ix].isActive)
        {
            liveObj.snmpDevices[ix].reset();
            liveObj.snmpDevices[ix].printInfo();
            for (uint8_t px = 0; px < liveObj.snmpDevices[ix].noOfRegisters; ++px)
            {
                // liveObj.modbusDevices[ix].mb_register[px].printInfo();
                liveObj.snmpDevices[ix].snmpRegisters[px].resetValue();
            }
        }
    }
}

uint8_t saveSnmpDevicesConfiguration()
{
    uint8_t success = false;

    // delay(100);
    // server.end();
    esp_task_wdt_reset(); // Enable panic on timeout
    deleteFile(SPIFFS, BKUP_SNMP_DEVICES_CONFIGURATION_BIN.c_str());
    renameFile(SPIFFS, SNMP_DEVICES_CONFIGURATION_BIN.c_str(), BKUP_SNMP_DEVICES_CONFIGURATION_BIN.c_str());

    uint32_t savedByteCount = 0, attempts = 5;

    do
    {
        savedByteCount = saveFile(SPIFFS, SNMP_DEVICES_CONFIGURATION_BIN.c_str(),
                                  (uint8_t *)liveObj.snmpDevices, sizeof(liveObj.snmpDevices));
        if (attempts > 0 && savedByteCount != sizeof(liveObj.snmpDevices))
        {
            esp_task_wdt_reset(); // Enable panic on timeout
            delay(10);
        }
        --attempts;
    } while (attempts > 0 && savedByteCount != sizeof(liveObj.snmpDevices));

    if (savedByteCount == sizeof(liveObj.snmpDevices))
    {

        success = true;
        Serial.println("Device configuration saved " + (String)savedByteCount);
    }
    else
    {
        deleteFile(SPIFFS, SNMP_DEVICES_CONFIGURATION_BIN.c_str());
        renameFile(SPIFFS, BKUP_SNMP_DEVICES_CONFIGURATION_BIN.c_str(), SNMP_DEVICES_CONFIGURATION_BIN.c_str());
    }

    load_snmp_devices();
    return success;
}

void load_snmp_devices()
{
    bool isDevicesLoaded = true;
    memset(liveObj.snmpDevices, 0, sizeof(liveObj.snmpDevices));
    uint32_t devicesSize = sizeof(liveObj.snmpDevices);
    Serial.println((String) "load_modbus_devices " + (String)devicesSize);
#if (1)
    if (SPIFFS.exists(SNMP_DEVICES_CONFIGURATION_BIN))
    {
        Serial.println((String) "Devices Configuration available");
        File file = SPIFFS.open(SNMP_DEVICES_CONFIGURATION_BIN, "r");
        if (!file)
        {
            Serial.println((String) "File not found ");
            isDevicesLoaded = false;
        }
        else
        {
            if (file.available() == sizeof(liveObj.snmpDevices))
            {
                file.read((uint8_t *)liveObj.snmpDevices, devicesSize);
            }
            else
            {
                isDevicesLoaded = false;
                Serial.println((String) "File MB_VALIDITY_VALUE " + file.size() + " " + sizeof(liveObj.snmpDevices));
            }
            file.close();
        }
    }
    else
    {
        Serial.println("File not exists");
    }
#else
#endif
    initializeSnmpDeviceInfo();
}

SNMP_Device parseSnmpJsonObject(JsonObject obj)
{
    SNMP_Device device;
    memset(&device, 0, sizeof(device));
    device.id = obj["id"];
    strcpy(device.address, (const char *)obj["address"]);
    strcpy(device.name, (const char *)obj["name"]);
    strcpy(device.readCommunity, (const char *)obj["readCommunity"]);
    strcpy(device.writeCommunity, (const char *)obj["writeCommunity"]);
    device.port = obj["port"];
    device.snmpVersion = obj["snmpVersion"];
    device.syncPeriod = obj["syncPeriod"];
    device.deviceDelay = obj["deviceDelay"];
    device.validityBytes = MB_VALIDITY_VALUE;
    device.isActive = 1;
    device.noOfRegisters = 0;

    int noOfRegisters = 0;
    for (JsonObject reg : obj["registers"].as<JsonArray>())
    {
        SnmpRegister mbRegister;
        mbRegister.id = reg["id"];
        strcpy(mbRegister.name, (const char *)reg["name"]);
        strcpy(mbRegister.oid, (const char *)reg["oid"]);
        mbRegister.data_type = reg["dataType"];
        mbRegister.factor = reg["factor"];
        mbRegister.access_type = READ_WRITE_ACCESS;
        device.snmpRegisters[noOfRegisters] = mbRegister;
        ++noOfRegisters;
        if (noOfRegisters >= MAX_MB_REGISTER_COUNT)
        {
            break;
        }
    }
    device.noOfRegisters = noOfRegisters;
    // device.printInfo();
    // device.printRegisterInfo();
    return device;
}

void process_snmp_requests()
{
    if (xSemaphoreTake(snmpMutex, portMAX_DELAY) == pdTRUE)
    {
        if (liveObj.currentSnmpDeviceIx < NO_OF_SNMP_DEVICES)
        {
            uint8_t dx = liveObj.currentSnmpDeviceIx;
            SNMP_Device *deviceObj = &liveObj.snmpDevices[dx];
            CommandStatus status = COMMAND_OK;
            if (deviceObj->isActive)
            {
                if (get_abs_diffrence(millis(), deviceObj->lastFetchTime) >= deviceObj->deviceDelay)
                {
                    deviceObj->lastFetchTime = millis();
                    status = sendSNMPRequest(deviceObj);
                    // Serial.println((String)"Snmp Device" + deviceObj->name + " " + status);
                    deviceObj->isSynced = deviceObj->isSynced || status == COMMAND_OK;
                    deviceObj->connected = status == COMMAND_OK;
                }

                if (get_abs_diffrence(millis(), deviceObj->lastSyncTime) >= deviceObj->syncPeriod)
                {
                    deviceObj->lastSyncTime = millis();
                    if (deviceObj->isSynced)
                    {
                        String deviceData = getDeviceRealtimeJson(deviceObj->id, false);
                        publishMessage(deviceObj->id, "", (const char *)deviceData.c_str(), deviceData.length());
                        deviceObj->isSynced = false;
                        deviceObj->communicationLastCounter = 0;
                    }
                    else
                    {
                        if (deviceObj->communicationLastCounter == 0)
                        {
                            ++liveObj.slaveConnectionCounter[dx];
                            Serial.println((String) "Disconnection RS485-> " + deviceObj->name);
                        }
                        deviceObj->communicationLastCounter++;
                    }
                    JsonDocument jsonDoc;
                    jsonDoc["disconnectionTime"] = (uint32_t)((deviceObj->communicationLastCounter * deviceObj->syncPeriod) / 60000);
                    jsonDoc["disconnectionCounter"] = liveObj.slaveConnectionCounter[dx];

                    String jsonResponse;
                    serializeJson(jsonDoc, jsonResponse);

                    publishMessage(deviceObj->id, "/connection", (const char *)jsonResponse.c_str(), jsonResponse.length());
                }
            }

            liveObj.currentSnmpDeviceIx = (liveObj.currentSnmpDeviceIx + 1) % NO_OF_SNMP_DEVICES;
        }
        else
        {
            liveObj.currentSnmpDeviceIx = 0;
        }
        xSemaphoreGive(snmpMutex); // Release the mute
    }
}