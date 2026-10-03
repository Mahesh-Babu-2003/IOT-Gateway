#include "application.h"
#include <esp_task_wdt.h>
#include <iostream>
#include "MuParser/muParser.h"
#include <esp_bt.h>
#include <esp_wifi.h>

ApplicationState liveObj;
ApplicationLog logObj;
Preferences preferences;

int testMuParser();
// Define timeout period in seconds

void initializeHardware()
{
  // put your setup code here, to run once:
  pinMode(WAKEUP_SW_IP, INPUT);
  pinMode(ANALOG_1_IP, INPUT); // Set the pin as input
  pinMode(ANALOG_2_IP, INPUT); // Set the pin as input

  pinMode(BUZZER_OP, OUTPUT);
  pinMode(RELAY_OP, OUTPUT);
  pinMode(RS485_EN_OP, OUTPUT);
  pinMode(LED1_OP, OUTPUT);
  pinMode(LED2_OP, OUTPUT);

  digitalWrite(BUZZER_OP, LOW);
  digitalWrite(RELAY_OP, LOW);
  digitalWrite(RS485_EN_OP, LOW);

  // Initialize RS485 mode control pin
  pinMode(RS485_EN_OP, OUTPUT);
  digitalWrite(RS485_EN_OP, LOW); // Start in receive mode (LOW for RX, HIGH for TX)

  Serial1.begin(9600, SERIAL_8N1, RXPIN, TXPIN);

  Serial.println("modbus serial initialized.");
}
void initializeSnmpManager();
void snmp_loop();
void snmp_setup();

void mqttLoopTask(void* args){
  while(1){
    prcoessMqttLoop();
  }
}

void setup()
{
  esp_log_level_set("*", ESP_LOG_NONE);
  // esp_bt_mem_release(ESP_BT_MODE_CLASSIC_BT); // Disable classic Bluetooth
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Minimal power save  

  delay(500);
  // Serial1.begin(9600, SERIAL_8N1,RXPIN, TXPIN);
  uint64_t chipid = ESP.getEfuseMac();
  memset(liveObj.mac, 0, sizeof(liveObj.mac));
  sprintf(liveObj.mac, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);

  // btStop();  // Disable Bluetooth

  Serial.begin(115200);
  if (!SPIFFS.begin(false))
  {
    Serial.println("SPIFFS Mount Failed");
    ESP.restart();
    return;
  } 
  Serial.println((String)"MALLOC_CAP_SPIRAM: " + heap_caps_get_free_size(MALLOC_CAP_SPIRAM)); // PSRAM size
  listDir(SPIFFS, "/", 0);
  Serial.print("Total space: ");
  Serial.println(SPIFFS.totalBytes());
  Serial.print("Used space: ");
  Serial.println(SPIFFS.usedBytes()); 

  load_application_log(); 
  logObj.switchOnCounter++;
  saveLogFile(); 

  initializeHardware(); 

  loadApplicationSettings(); 
  load_modbus_devices(); 
  load_snmp_devices();
  loadformulas(); 
  procesFormulas();
  Serial.println((String) "procesFormulas free heap size: " + (String)esp_get_free_heap_size() + " bytes");

  initializeWifi();
  // enableWifiAcessPoint();
  Serial.println((String) "initializeWifi free heap size: " + (String)esp_get_free_heap_size() + " bytes");
  
  initializeWebserver();
  Serial.println((String) "initializeWebserver free heap size: " + (String)esp_get_free_heap_size() + " bytes");

  mqttInit();
  Serial.println((String) "mqttInit free heap size: " + (String)esp_get_free_heap_size() + " bytes");

  if (liveObj.gatewayMode != LOCAL_MODE)
  {
    initializeTransaparentServer();
  }                  // Add to current task
  // configureBleCommunication();
  // snmp_setup();
  Serial.println((String) "Current free heap size: " + (String)esp_get_free_heap_size() + " bytes");
  
  // Initialize Task Watchdog Timer
  esp_task_wdt_init(WATCHDOG_TIMEOUT, true); // Enable panic on timeout
  esp_task_wdt_add(NULL);  
  
  initializeSnmpManager();

  
  xTaskCreateUniversal(mqttLoopTask, "mqttLoopTask", 16384, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  liveObj.powerOnSwitch = digitalRead(WAKEUP_SW_IP) == 0;
  Serial.println((String)"liveObj.powerOnSwitch: " + (String)liveObj.powerOnSwitch);
}

void processLoops()
{

  // Feed the watchdog to prevent a timeout
  esp_task_wdt_reset();
 
  // processWebserver();
  // prcoessMqttLoop();
  procesFormulas();

  if (get_abs_diffrence(liveObj.publishLedTime, millis()) < 1000)
  {
    digitalWrite(PUBLISH_LED_OP, HIGH);
  }
  else
  {
    digitalWrite(PUBLISH_LED_OP, LOW);
  }

  if (WiFi.isConnected() && mqtt.connected())
  {
    digitalWrite(CONNECTION_LED_OP, HIGH);
    liveObj.connectionBlinkTime = 0;
  }
  else if (WiFi.isConnected())
  {
    if (get_abs_diffrence(liveObj.connectionBlinkTime, millis()) > 1000)
    {
      if (digitalRead(CONNECTION_LED_OP) == LOW)
      {
        digitalWrite(CONNECTION_LED_OP, HIGH);
      }
      else
      {
        digitalWrite(CONNECTION_LED_OP, LOW);
      }
      liveObj.connectionBlinkTime = millis();
    }
    else
    {
    }
  }
  else
  {
    liveObj.connectionBlinkTime = 0;
    digitalWrite(CONNECTION_LED_OP, LOW);
  }

  if (!digitalRead(WAKEUP_SW_IP) && liveObj.isPressed == false)
  {
    liveObj.isPressed = true;
    liveObj.lastPressedTime = millis();
  }
  else if (digitalRead(WAKEUP_SW_IP))
  {
    if (liveObj.isPressed && get_abs_diffrence(liveObj.lastPressedTime, millis()) > 1000 && get_abs_diffrence(liveObj.lastPressedTime, millis()) < 20000)
    {
      if(liveObj.powerOnSwitch && get_abs_diffrence(liveObj.lastPressedTime, millis()) >= 4000){

        for(uint8_t ix = 0; ix < 10; ++ix){

          esp_task_wdt_reset();

          digitalWrite(CONNECTION_LED_OP, HIGH);
          delay(500);
          esp_task_wdt_reset();
          digitalWrite(CONNECTION_LED_OP, LOW);
          delay(500);
          esp_task_wdt_reset();
          loadDefaultNetworkConfiguration();
          saveWifiCredentials();
          esp_task_wdt_reset();
          delay(500);
          ESP.restart();
        }

        
        liveObj.powerOnSwitch = false;

      }else{ 
        server.end();
        mqtt.disconnect();
        mqtt_2.disconnect();
        if (isSoftAPEnabled())
        {
          disableWifiAccessPoint();
          connectToWifi();
        }
        else
        {
          enableWifiAcessPoint();
        }
        initializeWebserver();
        mqttInit();
      }
    }
    liveObj.isPressed = false;
  }

  if (liveObj.rebootRequire)
  {
    if (get_abs_diffrence(liveObj.rebootDelay, millis()) > 2000)
    {
      ESP.restart();
    }
  }
  
}

void application_delay(uint32_t delay)
{
  uint32_t timestamp = millis();
  while (get_abs_diffrence(millis(), timestamp) < delay)
  {
    processLoops();
  }
}


void loop()
{
  #if(1)
    if (liveObj.gatewayMode == LOCAL_MODE)
    {
      process_modbus_requests();
    }
    else
    {
      processTransparentLoop();
    }
  #else
    // process_modbus_requests();
      // processTransparentLoop();
  #endif
  process_snmp_requests();
  processWifi();
  processLoops();

if(get_abs_diffrence(liveObj.periodicDataTransmissionTime, millis()) >= 5000){
  if(liveObj.ai_1_formula.length() != 0 && liveObj.ai_1_value != -1){
        digitalWrite(RS485_EN_OP, HIGH);
        delay(1);
      Serial1.printf("@ADC_VAL1:%.2f\r\n", liveObj.ai_1_value);
        Serial1.flush(); 
        delay(1);
        digitalWrite(RS485_EN_OP, LOW);
  } 
  if(liveObj.ai_2_formula.length() != 0 && liveObj.ai_2_value != -1){
        digitalWrite(RS485_EN_OP, HIGH);
        delay(1);
        Serial1.printf("@ADC_VAL2:%.2f\r\n", liveObj.ai_2_value);
        Serial1.flush(); 
        delay(1);
        digitalWrite(RS485_EN_OP, LOW);
  }
  liveObj.periodicDataTransmissionTime =  millis();
}

  // snmp_loop();

  // if (isSoftAPEnabled())
  // {
  //   Serial.println("Soft AP Started");
  //   delay(100);
  // }

  // Optionally, you can monitor RAM usage periodically
  // delay(2000);
}

void print_bytes_in_hex(const char *title, uint8_t *raw, uint16_t len)
{
  String rawStr = "";
  for (int i = 0; i < len; i++)
  {
    char txt[4] = {0};
    sprintf(txt, "%02X ", raw[i]);
    rawStr += String(txt);
  }
  Serial.println((String)title + ": " + rawStr);
  // Serial.flush();
} 