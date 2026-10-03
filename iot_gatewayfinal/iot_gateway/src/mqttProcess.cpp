
#include "application.h"

// WiFiClientSecure wifiClient;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
WiFiClient wifiClient_2;
PubSubClient mqtt_2(wifiClient_2);
const char *mqttSettings = "mqttSettings";
// Wi-Fi credentials
const char *mqttServer_2 = "124.123.109.126";
// const char *mqttServer = "192.168.1.4";
const char *mqttServer = "122.175.39.25";
const int mqttPort = 1883; // 8883
const char *mqttUsername = "";
const char *mqttPassword = "";
const char *MQTT_SERVER_KEY = "MQTT_SERVER_KEY";
const char *MQTT_SERVER_PORT_KEY = "MQTT_SERVER_PORT_KEY";
const char *MQTT_USERNAME_KEY = "MQTT_USERNAME_KEY";
const char *MQTT_PASSWORD_KEY = "MQTT_PASSWORD_KEY";

const char *MQTT_SERVER_KEY_2 = "MQTT_SERVER_KEY_2";
const char *MQTT_SERVER_PORT_KEY_2 = "MQTT_SERVER_PORT_KEY_2";
const char *MQTT_USERNAME_KEY_2 = "MQTT_USERNAME_KEY_2";
const char *MQTT_PASSWORD_KEY_2 = "MQTT_PASSWORD_KEY_2";

String BaseDeviceReq = "/gateway/";
String BaseDeviceRaw = "/raw_gateway/";
String BasicServerTopic = "/server/";
String BasicServerRawTopic = "/raw_server/";
String deviceReq, deviceRaw, serverTopic, serverRawTopic;

#define MQTT_CONNECTION_ATTEMPTS_GAP 5000

void processRecievedBytes(PubSubClient* mqttClient, char *topicBytes, uint8_t *payloadBytes, unsigned int len){

  if (len >= 0)
  {
    payloadBytes[len] = 0;
    String topic = (char *)topicBytes;
    String payload = (char *)payloadBytes;
    
    Serial.println((String) "mqtt topic: " + topicBytes + ", " + (char *)payloadBytes);

    CommunicationResponse response;
    if (topic.endsWith("add_modbus_device"))
    {
      response = processAddModbusDeviceBody(payload);
    }
    else if (topic.endsWith("delete_modbus_device"))
    {
      response = processDeleteModbusDeviceBody(payload.toInt());
    }
    else if (topic.endsWith("add_snmp_device"))
    {
      response = processAddSnmpDeviceBody(payload);
    }
    else if (topic.endsWith("delete_snmp_device"))
    {
      response = processDeleteSnmpDeviceBody(payload.toInt());
    }
    else if (topic.endsWith("modbus_device_list"))
    {
      response = processModbusDeviceList();
    }
    else if (topic.endsWith("snmp_device_list"))
    {
      response = processSnmpDeviceList();
    }
    else if (topic.endsWith("modbus_parameter_list"))
    {
      response = processModbusDeviceParameterList(false, payload.toInt());
    }
    else if (topic.endsWith("snmp_parameter_list"))
    {
      response = processSnmpDeviceParameterList(false, payload.toInt());
    }
    else if (topic.endsWith("gateway_information"))
    {
      response = processGatewayInformation();
    }
    else if (topic.endsWith("get_wifi"))
    {
      response = processWifiSetup();
    }
    else if (topic.endsWith("set_wifi"))
    {
      response = processSaveWifiSetup(payload);
    }
    else if (topic.endsWith("save_formulas"))
    {
      response = processSaveFormulas(payload);
    }
    else if (topic.endsWith("configure_parameter"))
    {
      response = processConfigureParameter(payload);
    }    
    else if (topic.endsWith("load_formulas"))
    {
      response = processFormulas();
    }
    else if (topic.endsWith("modbus_write_request"))
    {
      response = modbusWriteRequest(payload);
    }  

    if (mqttClient->connected())
    {
      topic.replace(BaseDeviceReq, BasicServerTopic);
      if (response.message.length() > 0)
      {
        Serial.println((String) "mqtt response topic: " + topic + ", " + response.message);
        mqttClient->publish(topic.c_str(), (uint8_t *)response.message.c_str(), response.message.length(), false);
      }
      else if (response.byteCount > 0)
      {
        Serial.println((String) "mqtt response topic: ");
        print_bytes_in_hex(topic.c_str(), response.bytes, response.byteCount);
        mqttClient->publish(topic.c_str(), (uint8_t *)response.bytes, response.byteCount, false);
      }
    }

    if (topic.endsWith("set_wifi"))
    {
      application_delay(2000);
      connectToWifi();
    }
  }
}

void mqtt_1_callback(char *topicBytes, uint8_t *payloadBytes, unsigned int len)
{
  processRecievedBytes(&mqtt, topicBytes, payloadBytes, len);
}

void mqtt_2_callback(char *topicBytes, uint8_t *payloadBytes, unsigned int len)
{
  processRecievedBytes(&mqtt_2, topicBytes, payloadBytes, len);
}

void saveMqttCredentials()
{
  String mqttString;
  JsonDocument mqttDoc;
  Serial.println("Saving Mqtt credentials...");

  mqttDoc[MQTT_SERVER_KEY] = liveObj.mqttServer;
  mqttDoc[MQTT_SERVER_PORT_KEY] = liveObj.mqttPort;
  mqttDoc[MQTT_USERNAME_KEY] = liveObj.mqttUsername;
  mqttDoc[MQTT_PASSWORD_KEY] = liveObj.mqttPassword;


  mqttDoc[MQTT_SERVER_KEY_2] = liveObj.mqttServer_2;
  mqttDoc[MQTT_SERVER_PORT_KEY_2] = liveObj.mqttPort_2;
  mqttDoc[MQTT_USERNAME_KEY_2] = liveObj.mqttUsername_2;
  mqttDoc[MQTT_PASSWORD_KEY_2] = liveObj.mqttPassword_2;

  serializeJson(mqttDoc, mqttString);

  preferences.begin(mqttSettings, false);
  preferences.putString(mqttSettings, mqttString);
  preferences.end();
}

void loadMqttCredentials()
{

  preferences.begin(mqttSettings, false);
  bool isRestored = false;

  if (preferences.isKey(mqttSettings))
  {
    String mqttString = preferences.getString(mqttSettings);
    JsonDocument mqttDoc;
    DeserializationError error = deserializeJson(mqttDoc, mqttString.c_str());
    if (!error)
    {
      liveObj.mqttServer = mqttDoc[MQTT_SERVER_KEY].as<String>();
      liveObj.mqttPort = mqttDoc[MQTT_SERVER_PORT_KEY].as<int32_t>();
      liveObj.mqttUsername = mqttDoc[MQTT_USERNAME_KEY].as<String>();
      liveObj.mqttPassword = mqttDoc[MQTT_PASSWORD_KEY].as<String>();

      liveObj.mqttServer_2 = mqttDoc[MQTT_SERVER_KEY_2].as<String>();
      liveObj.mqttPort_2 = mqttDoc[MQTT_SERVER_PORT_KEY_2].as<int32_t>();
      liveObj.mqttUsername_2 = mqttDoc[MQTT_USERNAME_KEY_2].as<String>();
      liveObj.mqttPassword_2 = mqttDoc[MQTT_PASSWORD_KEY_2].as<String>();
      isRestored = true;
    }
  }

  if (isRestored == false)
  {
    liveObj.mqttServer = mqttServer;
    liveObj.mqttPort = mqttPort;
    liveObj.mqttUsername = mqttUsername;
    liveObj.mqttPassword = mqttPassword;
    
    liveObj.mqttServer_2 = mqttServer_2;
    liveObj.mqttPort_2 = mqttPort;
    liveObj.mqttUsername_2 = mqttUsername;
    liveObj.mqttPassword_2 = mqttPassword;
  }

  // Close Preferences
  preferences.end();
}

// Function to reconnect to MQTT broker
void reconnectMQTT(PubSubClient* mqttClient, String server, int mqttPort, String username, String password)
{ 
  int mqttIndex = 0;
  if(mqttClient == &mqtt_2)
  {
    mqttIndex = 1;
  } 
  // server.trim();
    if ( server.length() > 0 
    && get_abs_diffrence(liveObj.lastMqttAttemptTime[mqttIndex], millis()) >= MQTT_CONNECTION_ATTEMPTS_GAP && !mqttClient->connected()) 
  {
      String clientId = (String)liveObj.mac;
      if(mqttClient == &mqtt_2)
      {
        clientId = clientId + "_";
      } 

      if (mqttClient->connect(clientId.c_str(), username.c_str(), password.c_str()))
      {
        // Serial.println("Connected to MQTT broker! " + deviceReq);
        mqttClient->subscribe(deviceReq.c_str()); 
        Serial.println("Connected to MQTT broker! " + deviceReq);
      }
      else
      {
        Serial.println("Failed to connect to MQTT broker, retrying...");
      }
      {
        if(mqttClient == &mqtt)
        {
          mqttClient->setCallback(mqtt_1_callback);
        }
        else
        {
          mqttClient->setCallback(mqtt_2_callback);
        }
        
        mqttClient->subscribe(deviceReq.c_str());
        Serial.println("Connected to MQTT broker! " + deviceReq);
      }
      liveObj.lastMqttAttemptTime[mqttIndex] = millis();
    }
    if (logObj.mqttStatus[mqttIndex] != mqttClient->connected())
    {

      if (mqttClient->connected() == false)
      {
        ++logObj.mqttDisconnectionCounter[mqttIndex];
        saveLogFile();
      }

      logObj.mqttStatus[mqttIndex] = mqttClient->connected();
    }
}

void initializeMqtt()
{
  loadMqttCredentials();
  deviceReq = BaseDeviceReq + liveObj.mac + "/#";
  deviceRaw = BaseDeviceRaw + liveObj.mac + "/#";

  serverTopic = BasicServerTopic + liveObj.mac;
  serverRawTopic = BaseDeviceRaw + liveObj.mac;
}
void mqttInit()
{
  initializeMqtt();
  // Configure WiFiClientSecure to skip certificate validation
  // wifiClient.setInsecure();
  mqtt.setBufferSize(1024 * 10U);
  mqtt.setServer(liveObj.mqttServer.c_str(), liveObj.mqttPort);
  mqtt.setCallback(mqtt_1_callback);
  
  mqtt_2.setBufferSize(1024 * 10U);
  mqtt_2.setServer(liveObj.mqttServer_2.c_str(), liveObj.mqttPort_2);
  mqtt_2.setCallback(mqtt_2_callback);

  reconnectMQTT(&mqtt, liveObj.mqttServer, liveObj.mqttPort, liveObj.mqttUsername, liveObj.mqttPassword);
  reconnectMQTT(&mqtt_2, liveObj.mqttServer_2, liveObj.mqttPort_2, liveObj.mqttUsername_2.c_str(), liveObj.mqttPassword_2.c_str());
}

void publisheMessageToMqtt(PubSubClient* mqttClient, String topic, const char *data, uint32_t count){
  if (mqttClient->connected())
    { 
      liveObj.publishLedTime = millis(); 

      // Serial.println( (String)"Publish message: " + totalTopic);
      mqttClient->publish(topic.c_str(), (uint8_t *)data, count, false);
    }
}

void publishMessage(uint32_t deviceId, String sub_topic, const char *data, uint32_t count)
{

  {
  reconnectMQTT(&mqtt, liveObj.mqttServer, liveObj.mqttPort, liveObj.mqttUsername, liveObj.mqttPassword);
  reconnectMQTT(&mqtt_2, liveObj.mqttServer_2, liveObj.mqttPort_2, liveObj.mqttUsername_2.c_str(), liveObj.mqttPassword_2.c_str());
    String totalTopic = serverTopic + "/" + (String)deviceId + sub_topic;
    publisheMessageToMqtt(&mqtt, totalTopic, data, count);
    publisheMessageToMqtt(&mqtt_2, totalTopic, data, count);
  }
}

void publishGatewayMessage(String sub_topic, const char *data, uint32_t count)
{
  {
  reconnectMQTT(&mqtt, liveObj.mqttServer, liveObj.mqttPort, liveObj.mqttUsername, liveObj.mqttPassword);
  reconnectMQTT(&mqtt_2, liveObj.mqttServer_2, liveObj.mqttPort_2, liveObj.mqttUsername_2.c_str(), liveObj.mqttPassword_2.c_str());
    String totalTopic = serverTopic + sub_topic;
    publisheMessageToMqtt(&mqtt, totalTopic, data, count);
    publisheMessageToMqtt(&mqtt_2, totalTopic, data, count);
  }
}

void prcoessMqttLoop()
{

  {
  reconnectMQTT(&mqtt, liveObj.mqttServer, liveObj.mqttPort, liveObj.mqttUsername, liveObj.mqttPassword);
    mqtt.loop();
    
  reconnectMQTT(&mqtt_2, liveObj.mqttServer_2, liveObj.mqttPort_2, liveObj.mqttUsername_2.c_str(), liveObj.mqttPassword_2.c_str());
    mqtt_2.loop();
  }
}