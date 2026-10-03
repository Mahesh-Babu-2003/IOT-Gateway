// #include <WiFi.h>
// #include <WebServer.h>
// #include <SPIFFS.h>

// const char* ssid = "your-SSID";
// const char* password = "your-PASSWORD";

// WebServer server(80);

// // Serve gzipped CSS file
// void handleCSS() {
//     File file = SPIFFS.open("/styles.css.gz", "r");
//     if (!file) {
//         server.send(404, "text/plain", "File Not Found");
//         return;
//     }

//     // Set content encoding and type
//     server.setContentLength(file.size());
//     server.sendHeader("Content-Encoding", "gzip"); // Notify browser of gzip compression
//     server.sendHeader("Content-Type", "text/css");
//     server.streamFile(file, "text/css");
//     file.close();
// }

// void setup() {
//     Serial.begin(115200);

//     // Connect to WiFi
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED) {
//         delay(1000);
//         Serial.println("Connecting to WiFi...");
//     }
//     Serial.println("Connected to WiFi: " + WiFi.localIP().toString());

//     // Mount SPIFFS
//     if (!SPIFFS.begin(true)) {
//         Serial.println("SPIFFS Mount Failed");
//         return;
//     }

//     // Define routes
//     server.on("/styles.css", handleCSS);
//     server.begin();
//     Serial.println("Server started");
// }

// void loop() {
//     server.handleClient();
// }
