#include "application.h"

AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket webssocket_("/ws");

const char *http_username = "admin";
const char *http_password = "admin";
const char APPLICATION_JSON_TYPE[] = {"application/json"};
const char APPLICATION_TEXT_PLAIN_TYPE[] = {"text/plain"};
const char BODY_PARAM[] = {"body"}; // x-www-form-urlencoded
void serveFileFromFilesystem(AsyncWebServerRequest *request, const char *filename, const char *contentType);

void sendResponseToClient(AsyncWebServerRequest *request, int code, const char *content_type, const char *content)
{
  // request->send(code, content_type, content);

  AsyncResponseStream *response = request->beginResponseStream(content_type);
  response->addHeader("Access-Control-Allow-Origin", "*");                                // Allow all origins
  response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"); // Allow methods
  response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");     // Allow headers
  response->setCode(code);                                                                // Set appropriate HTTP code (404 for not found)
  response->print(content);
  request->send(response);
}
String getContentType(String filename)
{
  if (filename.endsWith(".html"))
    return "text/html";
  if (filename.endsWith(".css"))
    return "text/css";
  if (filename.endsWith(".js"))
    return "application/javascript";
  if (filename.endsWith(".png"))
    return "image/png";
  if (filename.endsWith(".jpg"))
    return "image/jpeg";
  if (filename.endsWith(".txt"))
    return "text/plain";
  return "application/octet-stream";
}

void serveFileFromFilesystem(AsyncWebServerRequest *request, const char *filename, const char *contentType)
{
  Serial.println((String) "Handling: " + filename + ", request: " + contentType);
  String gzFilename = String(filename) + ".gz";

  if (SPIFFS.exists(filename))
  {
    File file = SPIFFS.open(filename, "r");
    if (!file)
    {
      Serial.println("File not found!");
      sendResponseToClient(request, 404, APPLICATION_TEXT_PLAIN_TYPE, "File Not Found");
    }
    else
    {
      Serial.println((String) "File size: " + filename + ", " + String(file.size()));
      // AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/style.css", "text/css");
      // response->addHeader("Cache-Control", "max-age=86400"); // Cache for 24 hours

      // // request->setContentLength(file.size());
      // request->setContentLength(CONTENT_LENGTH_UNKNOWN); // CONTENT_LENGTH_UNKNOWN -1
      // // sendResponseToClientHeader("Content-Encoding", "gzip");
      // // sendResponseToClientHeader("Content-Type", contentType);
      // request->streamFile(file, contentType);
      request->send(file, contentType, true);
      file.close();

      Serial.println("file served successfully.");
    }
  }
  else if (SPIFFS.exists(gzFilename))
  {
    File file = SPIFFS.open(gzFilename, "r");
    if (!file)
    {
      Serial.println("File not found!");
      sendResponseToClient(request, 404, APPLICATION_TEXT_PLAIN_TYPE, "File Not Found");
    }
    else
    {
      Serial.println((String) "File size: " + gzFilename + ", " + String(file.size()));
      // // request->setContentLength(file.size());
      // request->setContentLength(CONTENT_LENGTH_UNKNOWN); // CONTENT_LENGTH_UNKNOWN -1
      // // sendResponseToClientHeader("Content-Encoding", "gzip");
      // // sendResponseToClientHeader("Content-Type", contentType);
      // request->streamFile(file, contentType);
      request->send(file, contentType, true);
      file.close();

      Serial.println("file served successfully.");
    }
  }
  else
  {
    sendResponseToClient(request, 404, APPLICATION_TEXT_PLAIN_TYPE, "File Not Found");
  }
}

void loadFileFromServer(AsyncWebServerRequest *request, const char *filename, bool isZipped = false)
{
  String path = String(filename);
  if (path.startsWith("/") == false)
  {
    path = "/" + path;
  }

  if (path.indexOf(".css") > 0)
  {
    serveFileFromFilesystem(request, path.c_str(), "text/css");
  }
  else if (path.indexOf(".js") > 0)
  {
    serveFileFromFilesystem(request, path.c_str(), "application/javascript");
  }
  else if (path.indexOf(".woff2") > 0)
  {
    serveFileFromFilesystem(request, path.c_str(), "application/octet-stream");
  }
  else if (path.indexOf(".png") > 0)
  {
    serveFileFromFilesystem(request, path.c_str(), "image/png");
  }
  else
  {
    serveFileFromFilesystem(request, path.c_str(), "text/html");
  }
}

void handleNotFound(AsyncWebServerRequest *request)
{
  String message = "404 Not Found\n\n";
  message += "URI: " + request->url() + "\n";
  message += (String) "Method: " + (request->method() == HTTP_GET ? "GET" : "POST") + "\n";
  message += "Arguments: " + String(request->args()) + "\n";
  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  sendResponseToClient(request, 404, APPLICATION_TEXT_PLAIN_TYPE, message.c_str());
}

void getModbusDeviceList(AsyncWebServerRequest *request)
{
  CommunicationResponse response = processModbusDeviceList();
  sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
}

void getSnmpDeviceList(AsyncWebServerRequest *request)
{
  CommunicationResponse response = processSnmpDeviceList();
  sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
}

void getModbusDeviceParamList(AsyncWebServerRequest *request, bool live)
{
  if (request->hasArg("id"))
  {
    int deviceId = request->arg("id").toInt(); // Get the 'value' parameter
    CommunicationResponse response = processModbusDeviceParameterList(live, deviceId);
    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
  }
  else
  {
    sendResponseToClient(request, 400, APPLICATION_TEXT_PLAIN_TYPE, "device id not found");
  }
}

void getSnmpDeviceParamList(AsyncWebServerRequest *request, bool live)
{
  if (request->hasArg("id"))
  {
    int deviceId = request->arg("id").toInt(); // Get the 'value' parameter
    CommunicationResponse response = processSnmpDeviceParameterList(live, deviceId);
    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
  }
  else
  {
    sendResponseToClient(request, 400, APPLICATION_TEXT_PLAIN_TYPE, "device id not found");
  }
}

void addUpdateDeviceRequest(AsyncWebServerRequest *request, uint8_t isModbus)
{
  if (request->hasParam(BODY_PARAM, true))
  {
    String body = request->getParam(BODY_PARAM, true)->value(); // Get the raw body of the POST request

    CommunicationResponse response;
    if(isModbus){ 
     response = processAddModbusDeviceBody(body);
    }else{
      response = processAddSnmpDeviceBody(body);
    }
    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
  }
  else
  {
    // No body in the request
    sendResponseToClient(request, 400, APPLICATION_JSON_TYPE, "{\"status\":\"error\",\"message\":\"No body received\"}");
  }
}

void printRequestData(AsyncWebServerRequest *request)
{
  int params = request->params();
  for (int i = 0; i < params; i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isFile())
    { // Handle file uploads
      Serial.printf("FILE[%s]: %s, size: %u bytes\n", p->name().c_str(), p->value().c_str(), p->size());
    }
    else if (p->isPost())
    { // Handle POST parameters
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
    else
    { // Handle GET parameters
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
}
void updateFormulas(AsyncWebServerRequest *request)
{
  Serial.println((String) "request->" + (String)request->contentLength());
  printRequestData(request);
  if (request->hasParam(BODY_PARAM, true))
  {
    String body = request->getParam(BODY_PARAM, true)->value(); // Get the raw body of the POST request

    CommunicationResponse response = processSaveFormulas(body);
    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
  }
  else
  {

    // No body in the request
    sendResponseToClient(request, 400, APPLICATION_JSON_TYPE, "{\"status\":\"error\",\"message\":\"No body received\"}");
  }
}

void addUpdateDeviceParameterRequest(AsyncWebServerRequest *request)
{
  if (request->hasArg("id") && request->hasParam(BODY_PARAM, true))
  {

    String body = request->getParam(BODY_PARAM, true)->value();
    int deviceId = request->arg("id").toInt();

    CommunicationResponse response = processAddModbusDeviceParameterBody(deviceId, body);
    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
  }
  else
  {
    // No body in the request
    sendResponseToClient(request, 400, APPLICATION_JSON_TYPE, "{\"status\":\"error\",\"message\":\"No body received\"}");
  }
}

void deleteModbusDeviceRequest(AsyncWebServerRequest *request, uint8_t isModbus)
{
  if (request->hasArg("id"))
  { 
    
    CommunicationResponse response;
    if(isModbus){ 
     response = processDeleteModbusDeviceBody(request->arg("id").toInt());
    }else{
      response = processDeleteSnmpDeviceBody(request->arg("id").toInt());
    }

    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
  }
  else
  {
    sendResponseToClient(request, 400, APPLICATION_TEXT_PLAIN_TYPE, "device id not found");
  }
}

void deleteModbusDeviceParameterRequest(AsyncWebServerRequest *request)
{
  if (request->hasArg("id") && request->hasArg("param_id"))
  {
    CommunicationResponse response = processDeleteModbusDeviceParameterBody(request->arg("id").toInt(), request->arg("param_id").toInt());
    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
  }
  else
  {
    sendResponseToClient(request, 400, APPLICATION_TEXT_PLAIN_TYPE, "device id and parameter id not found");
  }
}

void getRealTimeData(AsyncWebServerRequest *request)
{
  JsonDocument jsonDoc;
  for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
  {
    liveObj.modbusDevices[ix].printInfo();
    for (uint8_t px = 0; px < liveObj.modbusDevices[ix].noOfRegisters; ++px)
    {
      jsonDoc[(String)liveObj.modbusDevices[ix].mb_register[px].id] = liveObj.modbusDevices[ix].mb_register[px].getValue();
    }
  }
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  // Send JSON response
  sendResponseToClient(request, 200, APPLICATION_JSON_TYPE, jsonResponse.c_str());
}

void processAndSendResponse(AsyncWebServerRequest *request, CommunicationResponse response)
{
  sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
}

void sendWifiSetup(AsyncWebServerRequest *request)
{
  CommunicationResponse response = processWifiSetup();
  sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
}
void listAllParameters(AsyncWebServerRequest *request)
{
  String responseMessage = "Received Parameters: \n";
  for (uint8_t i = 0; i < request->params(); i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    String paramName = p->name();
    String paramValue = p->value();
    responseMessage += paramName + ": " + paramValue + "\n";
    Serial.println(paramName + ": " + paramValue);
  }
  Serial.print("responseMessage:" + responseMessage);
}

void saveWifiSetup(AsyncWebServerRequest *request)
{
  // Serial.println((String) "saveWifiSetup " + request->hasParam(BODY_PARAM, true) + " " + request->hasParam(BODY_PARAM) + " ");
  // Serial.println((String) "saveWifiSetup " + request->hasArg(BODY_PARAM));

  // listAllParameters(request);

  if (request->hasParam(BODY_PARAM, true))
  {
    String body = request->getParam(BODY_PARAM, true)->value(); // Get the raw body of the POST request
    Serial.println("saveWifiSetup body: " + body);

    CommunicationResponse response = processSaveWifiSetup(body);
    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());

    liveObj.rebootDelay = millis();
    liveObj.rebootRequire = true;
  }
  else
  {
    sendResponseToClient(request, 400, APPLICATION_TEXT_PLAIN_TYPE, "body not found");
  }
}

void saveConfigurationRequest(AsyncWebServerRequest *request)
{
  if (request->hasParam(BODY_PARAM, true))
  {
    String body = request->getParam(BODY_PARAM, true)->value(); // Get the raw body of the POST request
    Serial.println("saveConfigurationRequest body: " + body);

    CommunicationResponse response = processSaveConfiguration(body);
    sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
    // application_delay(2000);
    liveObj.rebootDelay = millis();
    liveObj.rebootRequire = true;
  }
  else
  {
    sendResponseToClient(request, 400, APPLICATION_TEXT_PLAIN_TYPE, "body not found");
  }
}

void handleAllGetRequests(AsyncWebServerRequest *request)
{
  String path = request->url(); // Get the base URL
  WebRequestMethodComposite method = request->method();
  String param = request->arg("action"); // Get query parameter `action`

  Serial.println((String) "Request: " + path + ", method: " + method + ", param: " + param + ", RAM: " + ESP.getFreeHeap());

  if (path.indexOf(".html") > 0)
  {
    loadFileFromServer(request, path.c_str());
  }
  else if (path.indexOf("assets/") > 0)
  {
    loadFileFromServer(request, path.c_str(), true);
  }
  else if (path.indexOf("fonts/") > 0)
  {
    loadFileFromServer(request, path.c_str(), true);
  }
  else if (path == "/")
  {
    serveFileFromFilesystem(request, "/index.html", "text/html");
  }
  else if (path == "/realtime")
  {
    getRealTimeData(request);
  }
  else if (path == "/information")
  {
    processAndSendResponse(request, processGatewayInformation());
  }
  else if (path == "/configuration")
  {
    processAndSendResponse(request, processConfiguration());
  }
  else if (path == "/loadFormulas")
  {
    processAndSendResponse(request, processFormulas());
  }
  else if (path == "/wifi_setup")
  {
    sendWifiSetup(request);
  }
  else if (path == "/modbus_devices")
  {
    getModbusDeviceList(request);
  }
  else if (path.startsWith("/live_modbus_device"))
  {
    getModbusDeviceParamList(request, true);
  }
  else if (path.startsWith("/modbus_registers"))
  {
    getModbusDeviceParamList(request, false);
  }
  else if (path == "/snmp_devices")
  {
    getSnmpDeviceList(request);
  }
  else if (path.startsWith("/live_snmp_device"))
  {
    getSnmpDeviceParamList(request, true);
  }
  else if (path.startsWith("/snmp_registers"))
  {
    getSnmpDeviceParamList(request, false);
  }
  else
  {
    handleNotFound(request);
  }
}

void handleAllPostRequests(AsyncWebServerRequest *request)
{
  String path = request->url(); // Get the base URL
  WebRequestMethodComposite method = request->method();
  String param = request->arg("action"); // Get query parameter `action`

  Serial.println((String) "Request: " + path + ", method: " + method + ", param: " + param + ", RAM: " + ESP.getFreeHeap());

  if (path == "/wifi_setup")
  {
    saveWifiSetup(request);
  }
  else if (path == "/configuration")
  {
    saveConfigurationRequest(request);
  }
  else if (path == "/add_modbus_device")
  {
    addUpdateDeviceRequest(request, 1);
  }
  else if (path.startsWith("/delete_modbus_device"))
  {
    deleteModbusDeviceRequest(request, 1);
  }
  else if (path == "/add_snmp_device")
  {
    addUpdateDeviceRequest(request, 0);
  }
  else if (path.startsWith("/delete_snmp_device"))
  {
    deleteModbusDeviceRequest(request, 0);
  }
  else if (path.startsWith("/save_formulas"))
  {
    updateFormulas(request);
  }
  else if (path.startsWith("/reboot"))
  {
    ESP.restart();
  }
  else if (path.startsWith("/format"))
  {
    if (SPIFFS.format())
    {
      Serial.println("SPIFFS erased successfully.");
    }
    else
    {
      Serial.println("Failed to erase SPIFFS.");
    }
    if (SPIFFS.begin(true))
    {
      Serial.println("SPIFFS mounted again.");
    }
    else
    {
      Serial.println("Failed to mount SPIFFS after erasing.");
    }
    sendResponseToClient(request, 400, APPLICATION_JSON_TYPE, "{\"status\":\"success\",\"message\":\"Data received\"}");
  }
  else
  {
    handleNotFound(request);
  }
}

// Callback to handle WebSocket events
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    Serial.printf("Client connected: ID %u\n", client->id());
    client->text("Welcome to ESP32 WebSocket Server!");
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.printf("Client disconnected: ID %u\n", client->id());
  }
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->opcode == WS_TEXT)
    {
      String message = "";
      for (size_t i = 0; i < len; i++)
      {
        message += (char)data[i];
      }
      Serial.printf("Received from client: %s\n", message.c_str());
      // Echo the message back to the client
      client->text("Echo: " + message);
    }
  }
}

void initializeWebserver()
{
  //  DefaultHeaders::Instance().addHeader("Connection", "close");
  // server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setCacheControl("max-age=86400"); // 86400
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html")
      // .setCacheControl("max-age=3600")
      .setAuthentication(http_username, liveObj.adminPassword.c_str()); // 86400

  server.on("/save_formulas", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              // Nothing here, body is processed separately
            },
            NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    static String body = ""; // Temporary buffer for the body

    // Append the incoming chunk of data to the body
    body += String((char*)data);
  // Serial.println("body: " + body);
    // If the full body is received (index + len == total), process it
    if (index + len == total) {      
      CommunicationResponse response = processSaveFormulas(body);
      sendResponseToClient(request, response.result ? 200 : 400, APPLICATION_JSON_TYPE, response.message.c_str());
      body = ""; // Clear buffer for next request
    } });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { 
        if (request->method() == HTTP_GET) {
            handleAllGetRequests(request);
        } else if (request->method() == HTTP_POST) {
            handleAllPostRequests(request);
        } else {
            request->send(405, "text/plain", "Method Not Allowed");
        } });

  // Attach WebSocket events
  webssocket_.onEvent(onWebSocketEvent);
  server.addHandler(&webssocket_);

  // webssocket_.textAll()

  // Start the server
  server.begin();
  Serial.println("Web server started...");
}

void processWebserver()
{
}
