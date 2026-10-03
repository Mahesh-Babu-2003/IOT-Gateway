
#if(0)
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin("your-SSID", "your-PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Serve the HTML file
  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Serve JavaScript or other files
  server.on("/script.js", HTTP_GET, []() {
    File file = SPIFFS.open("/script.js", "r");
    server.streamFile(file, "application/javascript");
    file.close();
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
#endif