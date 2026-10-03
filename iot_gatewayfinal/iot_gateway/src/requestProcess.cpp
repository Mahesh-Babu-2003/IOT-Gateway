#include "application.h"
#include "esp_task_wdt.h"

void configureSerialPort(uint32_t baud_rate, uint8_t parity);
extern SemaphoreHandle_t deviceMutex ;

CommunicationResponse response;

CommunicationResponse processAddModbusDeviceParameterBody(uint32_t deviceId, String body)
{
    JsonDocument deviceDoc; // You may adjust the size as needed

    // Parse the JSON string
    DeserializationError error = deserializeJson(deviceDoc, body.c_str());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        response.message = "{\"status\":\"error\",\"message\":\"Invalid body received\"}";
    }
    else
    {
        ApplicationRegister mbRegister;
        mbRegister.id = deviceDoc["id"];
        mbRegister.address = deviceDoc["address"];
        strcpy(mbRegister.name, (const char *)deviceDoc["name"]);
        mbRegister.reg_type = deviceDoc["regType"];
        mbRegister.byte_order = deviceDoc["byteOrder"];
        mbRegister.data_type = deviceDoc["dataType"];
        mbRegister.factor = deviceDoc["factor"];
        addModbusDeviceParameter(deviceId, mbRegister);
        response.message = "{\"status\":\"success\",\"message\":\"Data received\"}";
        response.result = true;
    }

    return response;
}

CommunicationResponse processDeleteModbusDeviceParameterBody(uint32_t deviceId, uint32_t parameterId)
{
    deleteModbusDeviceParameter(deviceId, parameterId);
    response.message = "{\"status\":\"success\",\"message\":\"Data received\"}";
    response.result = true;
    return response;
}

CommunicationResponse processAddModbusDeviceBody(String body)
{
    JsonDocument deviceDoc; // You may adjust the size as needed

    Serial.println("addUpdateDeviceRequest: " + body);
    // Parse the JSON string
    DeserializationError error = deserializeJson(deviceDoc, body.c_str());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        response.message = "{\"status\":\"error\",\"message\":\"Invalid body received\"}";
    }
    else
    {
        Serial.println("parseModbusJsonObject");
        ModbusDevice device = parseModbusJsonObject(deviceDoc.as<JsonObject>());

        response.result = device.address > 0 && device.baud_rate > 0 && addModbusDevice(device);

        if (response.result)
        {
            response.message = "{\"status\":\"success\",\"message\":\"Device Added\"}";
        }
        else
        {
            response.message = "{\"status\":\"error\",\"message\":\"Device Adding failed\"}";
        }
    }

    return response;
}

CommunicationResponse processDeleteModbusDeviceBody(uint32_t deviceId)
{
    response.result = deleteModbusDevice(deviceId);

    if (response.result)
    {
        response.message = "{\"status\":\"success\",\"message\":\"Device deleted\"}";
    }
    else
    {
        response.message = "{\"status\":\"error\",\"message\":\"Device deleting failed\"}";
    }
    return response;
}

CommunicationResponse processModbusDeviceParameterList(bool live, uint32_t deviceId)
{
    JsonDocument jsonDoc;
    JsonArray array = jsonDoc.to<JsonArray>();
    for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
    {
        if (liveObj.modbusDevices[ix].id == deviceId)
        {
            for (uint8_t px = 0; px < liveObj.modbusDevices[ix].noOfRegisters; ++px)
            {
                JsonObject obj = array.add<JsonObject>();
                obj["id"] = liveObj.modbusDevices[ix].mb_register[px].id;
                obj["name"] = liveObj.modbusDevices[ix].mb_register[px].name;
                if (live == false)
                {
                    obj["address"] = liveObj.modbusDevices[ix].mb_register[px].address;
                    obj["regType"] = liveObj.modbusDevices[ix].mb_register[px].reg_type;
                    obj["byteOrder"] = liveObj.modbusDevices[ix].mb_register[px].byte_order;
                    obj["dataType"] = liveObj.modbusDevices[ix].mb_register[px].data_type;
                    obj["factor"] = liveObj.modbusDevices[ix].mb_register[px].factor;
                }
                obj["value"] = liveObj.modbusDevices[ix].mb_register[px].getValue();
                // Serial.println((String)"getDeviceParamList->" + liveObj.modbusDevices[ix].mb_register[px].name
                // + " : " + liveObj.modbusDevices[ix].mb_register[px].getValue());
            }
            break;
        }
    }

    serializeJson(jsonDoc, response.message);
    response.result = true;
    return response;
}

CommunicationResponse processSnmpDeviceParameterList(bool live, uint32_t deviceId)
{
    JsonDocument jsonDoc;
    JsonArray array = jsonDoc.to<JsonArray>();
    for (uint8_t ix = 0; ix < NO_OF_SNMP_DEVICES; ++ix)
    {
        if (liveObj.snmpDevices[ix].id == deviceId)
        {
            for (uint8_t px = 0; px < liveObj.snmpDevices[ix].noOfRegisters; ++px)
            {
                JsonObject obj = array.add<JsonObject>();
                obj["id"] = liveObj.snmpDevices[ix].snmpRegisters[px].id;
                obj["name"] = liveObj.snmpDevices[ix].snmpRegisters[px].name;
                if (live == false)
                {
                    obj["oid"] = liveObj.snmpDevices[ix].snmpRegisters[px].oid;
                    obj["dataType"] = liveObj.snmpDevices[ix].snmpRegisters[px].data_type;
                    obj["factor"] = liveObj.snmpDevices[ix].snmpRegisters[px].factor;
                }
                obj["value"] = liveObj.snmpDevices[ix].snmpRegisters[px].getValue();
                // Serial.println((String)"getDeviceParamList->" + liveObj.modbusDevices[ix].mb_register[px].name
                // + " : " + liveObj.modbusDevices[ix].mb_register[px].getValue());
            }
            break;
        }
    }

    serializeJson(jsonDoc, response.message);
    response.result = true;
    return response;
}

CommunicationResponse processModbusDeviceList()
{
    JsonDocument jsonDoc;

    JsonArray array = jsonDoc.to<JsonArray>();
    for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
    {
        JsonObject obj = array.add<JsonObject>();
        obj["id"] = liveObj.modbusDevices[ix].id;
        obj["address"] = liveObj.modbusDevices[ix].address;
        obj["name"] = liveObj.modbusDevices[ix].name;
        obj["baudrate"] = liveObj.modbusDevices[ix].baud_rate;
        obj["parity"] = liveObj.modbusDevices[ix].parity;
        obj["regByteCount"] = liveObj.modbusDevices[ix].regByteCount;
        obj["timeOut"] = liveObj.modbusDevices[ix].timeOut;
        obj["registerDelay"] = liveObj.modbusDevices[ix].registerDelay;
        obj["deviceDelay"] = liveObj.modbusDevices[ix].deviceDelay;
        obj["syncPeriod"] = liveObj.modbusDevices[ix].syncPeriod;
        obj["connected"] = liveObj.modbusDevices[ix].connected;
    }
    serializeJson(jsonDoc, response.message);
    response.result = true;
    return response;
}
CommunicationResponse processSnmpDeviceList()
{
    JsonDocument jsonDoc;

    JsonArray array = jsonDoc.to<JsonArray>();
    for (uint8_t ix = 0; ix < NO_OF_SNMP_DEVICES; ++ix)
    {
        if (liveObj.snmpDevices[ix].isActive)
        {
            JsonObject obj = array.add<JsonObject>();
            obj["id"] = liveObj.snmpDevices[ix].id;
            obj["address"] = liveObj.snmpDevices[ix].address;
            obj["name"] = liveObj.snmpDevices[ix].name;
            obj["port"] = liveObj.snmpDevices[ix].port;
            obj["readCommunity"] = liveObj.snmpDevices[ix].readCommunity;
            obj["writeCommunity"] = liveObj.snmpDevices[ix].writeCommunity;
            obj["snmpVersion"] = liveObj.snmpDevices[ix].snmpVersion;
            obj["deviceDelay"] = liveObj.snmpDevices[ix].deviceDelay;
            obj["syncPeriod"] = liveObj.snmpDevices[ix].syncPeriod;
            obj["connected"] = liveObj.snmpDevices[ix].connected;
        }
    }
    serializeJson(jsonDoc, response.message);
    response.result = true;
    return response;
}

CommunicationResponse processGatewayInformation()
{
    JsonDocument jsonDoc;

    jsonDoc["ipAddress"] = WiFi.localIP();
    jsonDoc["subnetMask"] = WiFi.subnetMask();
    jsonDoc["gateway"] = WiFi.gatewayIP();
    jsonDoc["macAddress"] = WiFi.macAddress();
    jsonDoc["hostname"] = liveObj.hostname;
    jsonDoc["mac"] = liveObj.mac;
    jsonDoc["wifiConnected"] = WiFi.isConnected();

    jsonDoc["mqttServer"] = liveObj.mqttServer;
    jsonDoc["mqttPort"] = liveObj.mqttPort;
    jsonDoc["brokerConnected"] = mqtt.connected();

    jsonDoc["mqttServer_2"] = liveObj.mqttServer_2;
    jsonDoc["mqttPort_2"] = liveObj.mqttPort_2;
    jsonDoc["brokerConnected_2"] = mqtt_2.connected();

    serializeJson(jsonDoc, response.message);
    response.result = true;
    return response;
}

CommunicationResponse processWifiSetup()
{
    JsonDocument jsonDoc;

    jsonDoc["ssid"] = liveObj.wifiSsid;
    jsonDoc["password"] = liveObj.wifiPassword;
    jsonDoc["hostname"] = liveObj.hostname;

    jsonDoc["mqttServer"] = liveObj.mqttServer;
    jsonDoc["mqttPort"] = liveObj.mqttPort;
    jsonDoc["mqttUsername"] = liveObj.mqttUsername;
    jsonDoc["mqttPassword"] = liveObj.mqttPassword;

    jsonDoc["mqttServer_2"] = liveObj.mqttServer_2;
    jsonDoc["mqttPort_2"] = liveObj.mqttPort_2;
    jsonDoc["mqttUsername_2"] = liveObj.mqttUsername_2;
    jsonDoc["mqttPassword_2"] = liveObj.mqttPassword_2;

    jsonDoc["staticEnabled"] = liveObj.staticEnabled;
    jsonDoc["staticIp"] = liveObj.staticIp == nullptr ? "" : liveObj.staticIp;
    jsonDoc["staticSubnet"] = liveObj.staticSubnet == nullptr ? "" : liveObj.staticSubnet;
    jsonDoc["staticGateway"] = liveObj.staticGateway == nullptr ? "" : liveObj.staticGateway;
    jsonDoc["adminPassword"] = liveObj.adminPassword == nullptr ? "admin" : liveObj.adminPassword;

    serializeJson(jsonDoc, response.message);
    response.result = true;
    return response;
}

CommunicationResponse processConfiguration()
{
    JsonDocument jsonDoc;

    jsonDoc["gatewayMode"] = liveObj.gatewayMode;
    jsonDoc["baudrate"] = liveObj.baud_rate;
    jsonDoc["parity"] = liveObj.parity;
    jsonDoc["port"] = liveObj.serverPort;
    jsonDoc["maxClients"] = MAX_CLIENTS;

    jsonDoc["AI_1_HW"] = liveObj.ai_1_adc;
    jsonDoc["AI_2_HW"] = liveObj.ai_2_adc;

    jsonDoc["AI_1_VALUE"] = liveObj.ai_1_value;
    jsonDoc["AI_2_VALUE"] = liveObj.ai_2_value;
    jsonDoc["DO_1_VALUE"] = liveObj.do_1_value;

    serializeJson(jsonDoc, response.message);
    response.result = true;
    return response;
}

CommunicationResponse processSaveConfiguration(String body)
{
    JsonDocument jsonDoc; // You may adjust the size as needed

    // Parse the JSON string
    DeserializationError error = deserializeJson(jsonDoc, body.c_str());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        response.result = false;
        response.message = "{\"status\":\"error\",\"message\":\"Invalid body received\"}";
    }
    else
    {

        liveObj.gatewayMode = jsonDoc["gatewayMode"].as<int>();
        liveObj.baud_rate = jsonDoc["baudrate"].as<int>();
        liveObj.parity = jsonDoc["parity"].as<int>();
        liveObj.serverPort = jsonDoc["port"].as<int>();

        saveApplicationSettings();
        loadApplicationSettings();
        response = processConfiguration();
        response.result = true;
    }
    return response;
}

CommunicationResponse processSaveWifiSetup(String body)
{
    JsonDocument jsonDoc; // You may adjust the size as needed

    // Parse the JSON string
    DeserializationError error = deserializeJson(jsonDoc, body.c_str());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        response.result = false;
        response.message = "{\"status\":\"error\",\"message\":\"Invalid body received\"}";
    }
    else
    {

        liveObj.wifiSsid = jsonDoc["ssid"].as<String>();
        liveObj.wifiPassword = jsonDoc["password"].as<String>();
        liveObj.hostname = jsonDoc["hostname"].as<String>();

        liveObj.staticEnabled = jsonDoc["staticEnabled"].as<int>();
        liveObj.staticIp = jsonDoc["staticIp"].as<String>();
        liveObj.staticSubnet = jsonDoc["staticSubnet"].as<String>();
        liveObj.staticGateway = jsonDoc["staticGateway"].as<String>();

        liveObj.mqttServer = jsonDoc["mqttServer"].as<String>();
        liveObj.mqttPort = jsonDoc["mqttPort"].as<int>();
        liveObj.mqttUsername = jsonDoc["mqttUsername"].as<String>();
        liveObj.mqttPassword = jsonDoc["mqttPassword"].as<String>();
        // liveObj.mqttServer.trim();

        liveObj.mqttServer_2 = jsonDoc["mqttServer_2"].as<String>();
        liveObj.mqttPort_2 = jsonDoc["mqttPort_2"].as<int>();
        liveObj.mqttUsername_2 = jsonDoc["mqttUsername_2"].as<String>();
        liveObj.mqttPassword_2 = jsonDoc["mqttPassword_2"].as<String>();
        // liveObj.mqttServer_2.trim();

        liveObj.adminPassword = jsonDoc["adminPassword"].as<String>();

        Serial.println("processSaveWifiSetup: " + liveObj.wifiSsid + " " + liveObj.wifiPassword);

        saveWifiCredentials();
        saveMqttCredentials();
        response = processWifiSetup();
        response.result = true;
    }
    return response;
}

String getDeviceRealtimeJson(uint32_t id, uint8_t deviceType)
{
    JsonDocument jsonDoc;
    if (deviceType)
    {
        for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
        {
            if (liveObj.modbusDevices[ix].id == id)
            {
                for (uint8_t px = 0; px < liveObj.modbusDevices[ix].noOfRegisters; ++px)
                {
                    jsonDoc[(String)liveObj.modbusDevices[ix].mb_register[px].id] =
                    liveObj.modbusDevices[ix].mb_register[px].getValue();
                }
            }
        }

        /* ONLY AI1 VALUE */
        jsonDoc["AI1_VALUE"] = liveObj.ai_1_value;
    }
    else
    {
        for (uint8_t ix = 0; ix < NO_OF_SNMP_DEVICES; ++ix)
        {
            if (liveObj.snmpDevices[ix].id == id)
            {
                for (uint8_t px = 0; px < liveObj.snmpDevices[ix].noOfRegisters; ++px)
                {
                    jsonDoc[(String)liveObj.snmpDevices[ix].snmpRegisters[px].id] =
                    liveObj.snmpDevices[ix].snmpRegisters[px].getValue();
                }
            }
        }
    }

    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);
    return jsonResponse;
}

CommunicationResponse processFormulas()
{

    JsonDocument jsonDoc;

    double AI_1_VALUE = liveObj.ai_1_value;
    double AI_2_VALUE = liveObj.ai_2_value;
    double do_1_value = liveObj.do_1_value;

    jsonDoc["AI_1_HW"] = analogRead(ANALOG_1_IP);
    jsonDoc["AI_2_HW"] = analogRead(ANALOG_2_IP);

    jsonDoc["AI_1_VALUE"] = ((long)(liveObj.ai_1_value * 100)) / 100.0;
    jsonDoc["AI_2_VALUE"] = ((long)(liveObj.ai_2_value * 100)) / 100.0;
    jsonDoc["DO_1_VALUE"] = liveObj.do_1_value;

    jsonDoc["ai_1_formula"] = liveObj.ai_1_formula;
    jsonDoc["ai_2_formula"] = liveObj.ai_2_formula;
    jsonDoc["do_1_formula"] = liveObj.do_1_formula;

    jsonDoc["do_1_manual"] = liveObj.do_1_manual;
    jsonDoc["do_1_mode"] = liveObj.do_1_mode;

    jsonDoc["do_1_persistance_off"] = liveObj.do_1_persistance_off;
    jsonDoc["do_1_persistance_on"] = liveObj.do_1_persistance_on;

    serializeJson(jsonDoc, response.message);
    response.result = true;
    return response;
}

CommunicationResponse processConfigureParameter(String body)
{
    JsonDocument jsonDoc; // You may adjust the size as needed

    response.result = false;
    response.message = "{\"message\":\"Invalid body received\"}";

    Serial.println((String) "body: " + body);
    // Parse the JSON string
    DeserializationError error = deserializeJson(jsonDoc, body.c_str());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    }
    else
    {
        bool dataOkay = true;
        String param = jsonDoc["param"].as<String>();
        if (param != NULL)
        {
            param.trim();
        }
        if (param == "ai_1_formula" || param == "ai_2_formula" || param == "do_1_formula")
        {
            String value = jsonDoc["value"].as<String>();
            String error = computeExpression(value, nullptr);
            if (error.length() == 0)
            {
                if (param == "ai_1_formula")
                {
                    liveObj.ai_1_formula = value;
                }
                else if (param == "ai_2_formula")
                {
                    liveObj.ai_2_formula = value;
                }
                else if (param == "do_1_formula")
                {
                    liveObj.do_1_formula = value;
                }
                else
                {
                    dataOkay = false;
                }
            }
        }
        else if (param == "do_1_manual")
        {
            liveObj.do_1_manual = jsonDoc["value"].as<int>();
        }
        else if (param == "do_1_mode")
        {
            liveObj.do_1_mode = jsonDoc["value"].as<int>();
        }
        else if (param == "do_1_persistance_off")
        {
            liveObj.do_1_persistance_off = jsonDoc["value"].as<int>();
        }
        else if (param == "do_1_persistance_on")
        {
            liveObj.do_1_persistance_on = jsonDoc["value"].as<int>();
        }
        else
        {
            dataOkay = false;
        }
        if (dataOkay)
        {

            response.result = true;
            response.message = (String) "{\"status\":\"success\",\"message\":\"saved successfully\"}";

            saveFormulas();
        }
    }
    return response;
}

CommunicationResponse processSaveFormulas(String body)
{

    JsonDocument jsonDoc; // You may adjust the size as needed
    Serial.println((String) "body: " + body);
    // Parse the JSON string
    DeserializationError error = deserializeJson(jsonDoc, body.c_str());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        response.result = false;
        response.message = "{\"message\":\"Invalid body received\"}";
    }
    else
    {

        String ai_1_formula = jsonDoc["ai_1_formula"].as<String>();
        String ai_2_formula = jsonDoc["ai_2_formula"].as<String>();
        String do_1_formula = jsonDoc["do_1_formula"].as<String>();

        String error = computeExpression(ai_1_formula, nullptr);
        if (error.length() == 0)
        {
            error = computeExpression(ai_2_formula, nullptr);
        }
        if (error.length() == 0)
        {
            error = computeExpression(do_1_formula, nullptr);
        }
        response.result = error.length() == 0;
        response.message = (String) "{\"status\":\"error\",\"message\":\"" + error + "\"}";

        if (response.result)
        {
            liveObj.ai_1_formula = ai_1_formula;
            liveObj.ai_2_formula = ai_2_formula;
            liveObj.do_1_formula = do_1_formula;

            liveObj.do_1_manual = jsonDoc["do_1_manual"].as<int>();
            liveObj.do_1_mode = jsonDoc["do_1_mode"].as<int>();
            liveObj.do_1_persistance_off = jsonDoc["do_1_persistance_off"].as<int>();
            liveObj.do_1_persistance_on = jsonDoc["do_1_persistance_on"].as<int>();
            saveFormulas();
        }
        else
        {
        }
    }
    return response;
}

CommunicationResponse processAddSnmpDeviceBody(String body)
{
    // You may adjust the size as needed
    JsonDocument deviceDoc;

    Serial.println("processAddSnmpDeviceBody: " + (String)esp_get_free_heap_size() + " bytes");
    // Parse the JSON string
    DeserializationError error = deserializeJson(deviceDoc, body.c_str());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        response.message = "{\"status\":\"error\",\"message\":\"Invalid body received\"}";
    }
    else
    {
        SNMP_Device device = parseSnmpJsonObject(deviceDoc.as<JsonObject>());

        response.result = device.port > 0 && device.snmpVersion > 0 && addSnmpDevice(device);

        if (response.result)
        {
            response.message = "{\"status\":\"success\",\"message\":\"Device Added\"}";
        }
        else
        {
            response.message = "{\"status\":\"error\",\"message\":\"Device Adding failed\"}";
        }
    }

    return response;
}

CommunicationResponse processDeleteSnmpDeviceBody(uint32_t deviceId)
{
    response.result = deleteSnmpDevice(deviceId);

    if (response.result)
    {
        response.message = "{\"status\":\"success\",\"message\":\"Device deleted\"}";
    }
    else
    {
        response.message = "{\"status\":\"error\",\"message\":\"Device deleting failed\"}";
    }
    return response;
}

CommunicationResponse modbusWriteRequest(String body)
{

    JsonDocument jsonDoc; // You may adjust the size as needed
    Serial.println((String) "body: " + body);
    // Parse the JSON string
    DeserializationError error = deserializeJson(jsonDoc, body.c_str());

    response.result = false;
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        response.message = "{\"status\":\"error\",\"message\":\"Invalid body received\"}";
    }
    else
    {
    esp_task_wdt_reset(); // Enable panic on timeout
        if (xSemaphoreTake(deviceMutex, portMAX_DELAY) == pdTRUE)
        {
            esp_task_wdt_reset(); // Enable panic on timeout
            JsonObject obj = jsonDoc.as<JsonObject>();

            uint8_t address = obj["address"];
            String command_id = (const char *)obj["id"];
            uint8_t fCode = obj["fCode"];
            uint16_t register_address = obj["register_address"];
            String value = (const char *)obj["value"];
            uint8_t dataType = obj["dataType"];
            uint8_t byteOrder = obj["byteOrder"];
            uint8_t parity = obj["parity"];
            uint32_t baudrate = obj["baudrate"];

            Serial.println((String) "address: " + address);
            Serial.println((String) "fCode: " + fCode);
            Serial.println((String) "register_address: " + register_address);
            Serial.println((String) "value: " + value);
            Serial.println((String) "dataType: " + dataType);
            Serial.println((String) "byteOrder: " + byteOrder);
            Serial.println((String) "baudrate: " + baudrate);
            Serial.println((String) "parity: " + parity);

            response.message = "{\"status\":\"error\",\"message\":\"writing failed\", \"id\":\"" + command_id + "\"}";
            if (address == 254)
            {
                if (register_address == 0)
                {
                    liveObj.do_1_mode = value.toInt();
                }
                else if (register_address == 1)
                {
                    liveObj.do_1_manual = value.toInt();
                }
                else if (register_address == 2)
                {
                    liveObj.do_1_persistance_off = value.toInt();
                }
                else if (register_address == 3)
                {
                    liveObj.do_1_persistance_on = value.toInt();
                }
                response.result = true;
            }
            else
            {
                configureSerialPort(baudrate, parity);
                delay(100);
                REGISTER_VALUE register_value;

                switch (dataType)
                {
                case BIT_TYPE:
                case UINT8_TYPE:
                    register_value.u8_value = value.toInt();
                    break;
                case INT8_TYPE:
                    register_value.i8_value = value.toInt();
                    break;
                case INT16_TYPE:
                    register_value.i16_value = value.toInt();
                    break;
                case UINT16_TYPE:
                    register_value.u16_value = value.toInt();
                    break;
                case INT32_TYPE:
                    register_value.i32_value = value.toInt();
                    break;
                case UINT32_TYPE:
                    register_value.u32_value = value.toInt();
                    break;
                case INT64_TYPE:
                    register_value.i64_value = value.toInt();
                    break;
                case UINT64_TYPE:
                    register_value.u64_value = value.toInt();
                    break;
                case FLOAT_TYPE:
                    register_value.f_value = value.toFloat();
                    break;
                case DOUBLE_TYPE:
                    register_value.d_value = value.toDouble();
                    break;
                case STRING_TYPE:
                    break;
                default:
                    break;
                }
                response.result = sendMeterWriteRequest(address, fCode, dataType, byteOrder, register_address, register_value);
            }
            xSemaphoreGive(deviceMutex); // Release the mute
            
            esp_task_wdt_reset(); // Enable panic on timeout
            if (response.result)
            {
                response.message = "{\"status\":\"success\",\"message\":\"writing done\", \"id\":\"" + command_id + "\"}";
            }
        }
    }
    return response;
}