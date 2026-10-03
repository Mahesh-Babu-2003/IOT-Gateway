
#include "application.h"

#define NUMBER_OF_WIFI_CONNECTION_ATTEMPTS 5
#define WIFI_CONNECTION_ATTEMPTS_GAP 5000
const char *wifiSettings = "WifiSettings";

// Wi-Fi credentials
const char *ssidAP = "Talk2Cloud";
const char *passwordAP = "powern@solutions";
const char *ssidSTA = "Powern";
const char *passwordSTA = "venus123";
// const char *ssidSTA = "AJ Mobile";
// const char *passwordSTA = "aj@network";
// const char *ssidSTA = "AJ Network";
// const char *passwordSTA = "aj@network";

const char *hostName = "Powern Gateway";

const char *HOSTNAME_KEY = "HOSTNAME_KEY";
const char *AP_SSID_KEY = "apSsid";
const char *AP_PASSWORD_KEY = "apPassword";
const char *WIFI_SSID_KEY = "wifiSsid";
const char *WIFI_PASSWORD_KEY = "wifiPassword";
const char *LAST_WIFI_SSID_KEY = "lastWifiSsid";
const char *LAST_WIFI_PASSWORD_KEY = "lastWifiPassword";
const char *STATIC_ENABLED = "staticEnabled";
const char *STATIC_IP_KEY = "staticIP";
const char *STATIC_SUBNET_KEY = "staticSubnet";
const char *STATIC_GATEWAY_KEY = "staticGateway";
const char *ADMIN_PASSWORD_KEY = "adminPassword";

bool isValidIPAddress(const String &ipAddress)
{
  int p1, p2, p3, p4;

  // Use sscanf to parse the IP address string into four integer values
  int result = sscanf(ipAddress.c_str(), "%d.%d.%d.%d", &p1, &p2, &p3, &p4);

  // Validate if we successfully parsed four integers
  if (result != 4)
  {
    return false; // Invalid format
  }

  // Check if each octet is between 0 and 255
  if (p1 >= 0 && p1 <= 255 && p2 >= 0 && p2 <= 255 && p3 >= 0 && p3 <= 255 && p4 >= 0 && p4 <= 255)
  {
    return true;
  }

  return false;
}

void saveWifiCredentials()
{
  String networkString;
  JsonDocument networkDoc;
  Serial.println("Saving Wi-Fi credentials...");

  networkDoc[HOSTNAME_KEY] = liveObj.hostname;
  networkDoc[AP_SSID_KEY] = liveObj.wifi_AP_Ssid;
  networkDoc[AP_PASSWORD_KEY] = liveObj.wifi_AP_Password;
  networkDoc[WIFI_SSID_KEY] = liveObj.wifiSsid;
  networkDoc[WIFI_PASSWORD_KEY] = liveObj.wifiPassword;

  networkDoc[STATIC_ENABLED] = liveObj.staticEnabled;
  networkDoc[STATIC_IP_KEY] = liveObj.staticIp;
  networkDoc[STATIC_SUBNET_KEY] = liveObj.staticSubnet;
  networkDoc[STATIC_GATEWAY_KEY] = liveObj.staticGateway;


  networkDoc[LAST_WIFI_SSID_KEY] = liveObj.lastWifiSsid;
  networkDoc[LAST_WIFI_PASSWORD_KEY] = liveObj.lastWifiPassword;

  
 
  if(liveObj.adminPassword == nullptr || liveObj.adminPassword.length() == 0){
    liveObj.adminPassword = "admin";
  }
  networkDoc[ADMIN_PASSWORD_KEY] = liveObj.adminPassword;

  serializeJson(networkDoc, networkString);

  preferences.begin(wifiSettings, false);
  preferences.putString(wifiSettings, networkString);
  preferences.end();
}

void loadDefaultNetworkConfiguration(){
  liveObj.hostname = hostName;
    liveObj.wifi_AP_Ssid = ssidAP;
    liveObj.wifi_AP_Password = passwordAP;
    liveObj.wifiSsid = ssidSTA;
    liveObj.wifiPassword = passwordSTA;
    liveObj.staticEnabled = 0;
    liveObj.staticIp = "";
    liveObj.staticSubnet = "";
    liveObj.staticGateway = "";
    liveObj.adminPassword = "admin";

    
    liveObj.lastWifiSsid = "";
    liveObj.lastWifiPassword = "";
}

void loadNetworkConfiguration()
{
  preferences.begin(wifiSettings, false);

  bool isRestored = false;
  if (preferences.isKey(wifiSettings))
  {
    String networkString = preferences.getString(wifiSettings);
    JsonDocument networkDoc;
    DeserializationError error = deserializeJson(networkDoc, networkString.c_str());
    if (!error)
    {
      liveObj.hostname = networkDoc[HOSTNAME_KEY].as<String>();
      liveObj.wifi_AP_Ssid = networkDoc[AP_SSID_KEY].as<String>();
      liveObj.wifi_AP_Password = networkDoc[AP_PASSWORD_KEY].as<String>();
      liveObj.wifiSsid = networkDoc[WIFI_SSID_KEY].as<String>();
      liveObj.wifiPassword = networkDoc[WIFI_PASSWORD_KEY].as<String>();

      liveObj.staticEnabled = networkDoc[STATIC_ENABLED].as<int>();

      liveObj.staticIp = networkDoc[STATIC_IP_KEY].as<String>();
      liveObj.staticSubnet = networkDoc[STATIC_SUBNET_KEY].as<String>();
      liveObj.staticGateway = networkDoc[STATIC_GATEWAY_KEY].as<String>();
 
      liveObj.adminPassword = networkDoc[ADMIN_PASSWORD_KEY].as<String>();
      if(liveObj.adminPassword == nullptr ||  liveObj.adminPassword.length() == 0){
        liveObj.adminPassword = "admin";
      }

      
      liveObj.lastWifiSsid = networkDoc[LAST_WIFI_SSID_KEY].as<String>();
      liveObj.lastWifiPassword = networkDoc[LAST_WIFI_PASSWORD_KEY].as<String>();

      isRestored = true;
    }
  }

  if (isRestored == false)
  {
    loadDefaultNetworkConfiguration();
  }
  // Close Preferences
  preferences.end();
}

bool isSoftAPEnabled()
{
  return WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;
}

bool isSoftAPStarted()
{
  // Check if the SoftAP has a valid IP address
  return WiFi.softAPIP().toString() != "0.0.0.0";
}

void enableWifiAcessPoint()
{
  if (isSoftAPEnabled() == false)
  {
    Serial.println("Starting AP mode...");
    String finalAp = String(ssidAP) + "_" + liveObj.mac;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(finalAp.c_str(), passwordAP);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
}

void disableWifiAccessPoint()
{

  if (isSoftAPEnabled() == true)
  {
    Serial.println("Stopping AP mode...");
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
  }
}

// Function to convert string to IPAddress
IPAddress stringToIP(String ipStr)
{
  int p1, p2, p3, p4;

  // Use sscanf to split the string into 4 integers
  sscanf(ipStr.c_str(), "%d.%d.%d.%d", &p1, &p2, &p3, &p4);

  // Return the IPAddress object
  return IPAddress(p1, p2, p3, p4);
}

void connectToWifi()
{
  Serial.println((String) "Connecting to Wi-Fi..." + liveObj.wifiSsid + " " + liveObj.wifiPassword + " " + liveObj.hostname
  + " " + liveObj.staticIp  + " " +  isValidIPAddress(liveObj.staticIp) 
  + " " + liveObj.staticSubnet  + " " +  isValidIPAddress(liveObj.staticSubnet)
  + " " + liveObj.staticGateway  + " " +  isValidIPAddress(liveObj.staticGateway)
  + " " + liveObj.staticEnabled + " " + liveObj.adminPassword);
  WiFi.disconnect(true);
  
  WiFi.mode(WIFI_STA); // Enable AP + STA mode 
  WiFi.setHostname(liveObj.hostname.c_str());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  if (liveObj.staticEnabled == 1 && isValidIPAddress(liveObj.staticIp) &&
      isValidIPAddress(liveObj.staticSubnet) && isValidIPAddress(liveObj.staticGateway))
  {

     Serial.println((String) "Connecting to Wi-Fi With Static");

    IPAddress local_ip = stringToIP(liveObj.staticIp);
    IPAddress gateway = stringToIP(liveObj.staticGateway);
    IPAddress subnet = stringToIP(liveObj.staticSubnet);
    WiFi.config(local_ip, gateway, subnet);
  }

  WiFi.begin(liveObj.wifiSsid.c_str(), liveObj.wifiPassword.c_str());
  WiFi.waitForConnectResult(1000);
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  liveObj.lastWifiAttemptTime = millis();
}

void scanAvailableWifiNetworks()
{
  // Enable Wi-Fi scanning
  Serial.println("\nScanning for networks...");
  int networks = WiFi.scanNetworks();
  for (int i = 0; i < networks; i++)
  {
    Serial.println((String) "SSID: " + WiFi.SSID(i).c_str() + ", RSSI: " + WiFi.RSSI(i) + " dBm\n");
  }
}

void initializeWifi()
{
  loadNetworkConfiguration(); 
  // scanAvailableWifiNetworks();
  connectToWifi();
} 

void processWifi()
{ 

  if (logObj.wifiStatus != WiFi.isConnected())
  {

    if (WiFi.isConnected() == false)
    {
      ++logObj.wifiDisconnectionCounter;
      saveLogFile();
    }
    logObj.wifiStatus = WiFi.isConnected();
  }

  if(WiFi.isConnected() ){  
  }

  // if (get_abs_diffrence(liveObj.lastWifiAP_AttemptTime, millis()) >= 2000)
  // {
  //   // Serial.println( (String)"Check AP status..." + isSoftAPEnabled() + " " + WiFi.softAPIP().toString()
  //   // + " " + WiFi.isConnected());
  //   if (WiFi.isConnected() == false)
  //   {
  //     enableWifiAcessPoint();
  //   }
  //   else
  //   {
  //     disableWifiAccessPoint();
  //   }
  //   liveObj.lastWifiAP_AttemptTime = millis();
  // }
}
