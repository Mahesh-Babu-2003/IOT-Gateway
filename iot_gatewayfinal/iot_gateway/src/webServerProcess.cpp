// #include "application.h"
 
//  #include <WebServer.h>
// // Web server on port 80
// WebServer server(80);

// // Function prototypes
// int sensorValue = 0;
// String valueFromGet = "";
// String valueFromPost = "";

// const char APPLICATION_JSON_TYPE[] = {"application/json"};
// const char APPLICATION_TEXT_PLAIN_TYPE[] = {"text/plain"};
 
// void serveFileFromFilesystem(const char *filename, const char *contentType);

// void sendResponseToClient(int code, const char* content_type, const char* content){
//       server.send(code, content_type, content); 
// }

// void serveFileFromFilesystem(const char *filename, const char *contentType)
// {
//     Serial.println((String) "Handling " + filename + "request..." + contentType);
//   if (SPIFFS.exists(filename))
//   {
//     File file = SPIFFS.open(filename, "r");
//     if (!file)
//     {
//       Serial.println("File not found!");
//       sendResponseToClient(404, APPLICATION_TEXT_PLAIN_TYPE, "File Not Found");
//       return;
//     }

//     Serial.println((String) "File size: " + String(file.size()));

//     // server.setContentLength(file.size());
//     server.setContentLength(CONTENT_LENGTH_UNKNOWN); // CONTENT_LENGTH_UNKNOWN -1
//     // sendResponseToClientHeader("Content-Encoding", "gzip");
//     // sendResponseToClientHeader("Content-Type", contentType);
//     server.streamFile(file, contentType);
//     file.close();

//     Serial.println("CSS file served successfully.");
//   }
//   else
//   {
//     sendResponseToClient(404, APPLICATION_TEXT_PLAIN_TYPE, "File Not Found");
//   }
// }

// void loadFileFromServer(const char *filename, bool isZipped = false){
//   String path = String(filename);
//   if(path.startsWith("/") == false){
//     path = "/" + path;
//   }
  
//   if(path.indexOf(".css") > 0){
//     serveFileFromFilesystem(path.c_str(), "text/css");    
//   }else if(path.indexOf(".js") > 0){
//     serveFileFromFilesystem(path.c_str(), "application/javascript");     
//   }else if(path.indexOf(".woff2") > 0){
//     serveFileFromFilesystem(path.c_str(), "application/octet-stream"); 
//   }else if(path.indexOf(".png") > 0){
//     serveFileFromFilesystem(path.c_str(), "image/png");     
//   }else{
//     serveFileFromFilesystem(path.c_str(), "text/html");   
//   }
// }

// void handleNotFound()
// {
//   String message = "404 Not Found\n\n";
//   message += "URI: " + server.uri() + "\n";
//   message += (String) "Method: " + (server.method() == HTTP_GET ? "GET" : "POST") + "\n";
//   message += "Arguments: " + String(server.args()) + "\n";
//   for (uint8_t i = 0; i < server.args(); i++)
//   {
//     message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
//   }
//   sendResponseToClient(404, APPLICATION_TEXT_PLAIN_TYPE, message.c_str());
// }

// void getDeviceList()
// { 
//     CommunicationResponse response = processModbusDeviceList();  
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
// }

// void getDeviceParamList(bool live)
// {
//   if (server.hasArg("id"))
//   {
//     int deviceId = server.arg("id").toInt(); // Get the 'value' parameter    
//     CommunicationResponse response = processModbusDeviceParameterList(live, deviceId);  
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
//   }
//   else
//   {
//     sendResponseToClient(400, APPLICATION_TEXT_PLAIN_TYPE, "device id not found");
//   }
// }

// void addUpdateDeviceRequest()
// {
//   if (server.hasArg("plain"))
//   {
//     String body = server.arg("plain"); // Get the raw body of the POST request
    
//     CommunicationResponse response = processAddModbusDeviceBody(body);  
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
//   }
//   else
//   {
//     // No body in the request
//     sendResponseToClient(400, APPLICATION_JSON_TYPE, "{\"status\":\"error\",\"message\":\"No body received\"}");
//   }
// }


// void addUpdateDeviceParameterRequest()
// {
//   if (server.hasArg("id") && server.hasArg("plain"))
//   {
//     String body = server.arg("plain"); // Get the raw body of the POST request
//     int deviceId = server.arg("id").toInt();

//     CommunicationResponse response = processAddModbusDeviceParameterBody(deviceId, body);  
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
     
//   }
//   else
//   {
//     // No body in the request
//     sendResponseToClient(400, APPLICATION_JSON_TYPE, "{\"status\":\"error\",\"message\":\"No body received\"}");
//   }
// }

// void deleteModbusDeviceRequest()
// {
//   if (server.hasArg("id"))
//   {
//     CommunicationResponse response = processDeleteModbusDeviceBody(server.arg("id").toInt());
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
//   }
//   else
//   {
//     sendResponseToClient(400, APPLICATION_TEXT_PLAIN_TYPE, "device id not found");
//   }
// }

// void deleteModbusDeviceParameterRequest()
// {
//   if (server.hasArg("id") && server.hasArg("param_id"))
//   { 
//     CommunicationResponse response = processDeleteModbusDeviceParameterBody(server.arg("id").toInt(), server.arg("param_id").toInt());
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
//   }
//   else
//   {
//     sendResponseToClient(400, APPLICATION_TEXT_PLAIN_TYPE, "device id and parameter id not found");
//   }
// }


// void getRealTimeData()
// {
//   JsonDocument jsonDoc;
//   for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
//   {
//     liveObj.modbusDevices[ix].printInfo();
//     for (uint8_t px = 0; px < liveObj.modbusDevices[ix].noOfRegisters; ++px)
//     {
//       jsonDoc[(String)liveObj.modbusDevices[ix].mb_register[px].id] = liveObj.modbusDevices[ix].mb_register[px].getValue();
//     }
//   }
//   String jsonResponse;
//   serializeJson(jsonDoc, jsonResponse);

//   // Send JSON response
//   sendResponseToClient(200, APPLICATION_JSON_TYPE, jsonResponse.c_str());
// }

// void sendGatewayInformation(){ 
//     CommunicationResponse response = processGatewayInformation();
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
// }

// void sendWifiSetup(){ 
//     CommunicationResponse response = processWifiSetup();
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
// }

// void saveWifiSetup()
// {
//   if (server.hasArg("plain")){ 
//     String body = server.arg("plain"); // Get the raw body of the POST request
//     Serial.println("saveWifiSetup body: " + body); 
    
//     CommunicationResponse response = processSaveWifiSetup(body);
//     sendResponseToClient(response.result?200:400, APPLICATION_JSON_TYPE, response.message.c_str());
//     connectToWifi();

//   }else{
//     sendResponseToClient(400, APPLICATION_TEXT_PLAIN_TYPE, "body not found"); 
//   }
// }

// void handleAllRequests()
// {
//   String path = server.uri(); // Get the base URL
//   HTTPMethod method = server.method();
//   String param = server.arg("action"); // Get query parameter `action`

//   Serial.println((String) "Request ->" + path + ", param " + param + " RAM " + ESP.getFreeHeap());

//   if (path.indexOf(".html") > 0 && method == HTTP_GET)
//   {
//     loadFileFromServer(path.c_str());
//   }
//   else if (path.indexOf("assets/") > 0 && method == HTTP_GET)
//   {
//     loadFileFromServer(path.c_str(), true);
//   }
//   else if (path.indexOf("fonts/") > 0 && method == HTTP_GET)
//   {
//     loadFileFromServer(path.c_str(), true);
//   }
//   else if (path == "/" && method == HTTP_GET)
//   {
//     serveFileFromFilesystem("/index.html", "text/html");
//   }
//   else if (path == "/realtime" && method == HTTP_GET)
//   {
//     getRealTimeData();
//   }
//   else if(path == "/information" && method == HTTP_GET){
//     sendGatewayInformation();
//   }
//   else if(path == "/wifi_setup" && method == HTTP_GET){
//     sendWifiSetup();
//   }
//   else if(path == "/wifi_setup" && method == HTTP_POST){
//     saveWifiSetup();
//   }
//   else if (path == "/devices" && method == HTTP_GET)
//   {
//     getDeviceList();
//   }
//   else if (path.startsWith("/live_device") && method == HTTP_GET)
//   {
//     getDeviceParamList(true);
//   }
//   else if (path.startsWith("/registers") && method == HTTP_GET)
//   {
//     getDeviceParamList(false);
//   }
//   else if (path == "/add_device" && method == HTTP_POST)
//   {
//     addUpdateDeviceRequest();
//   } 
//   else if (path.startsWith("/delete_device") && method == HTTP_POST)
//   {
//     deleteModbusDeviceRequest();
//   } 
  
//   else
//   {
//     handleNotFound();
//   }
// } 
// void initializeWebserver()
// { 
//   server.on("/", handleAllRequests); // Single handler for all routes
//   server.onNotFound(handleAllRequests);

//   server.begin();
//   Serial.println("Web server started..."); 
// }

// void processWebserver()
// {
 
//   server.handleClient(); // Handle incoming HTTP requests 
// }
