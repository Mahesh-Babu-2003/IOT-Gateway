// #include "application.h"
// #include <NimBLEDevice.h>

// // UUIDs for the service and characteristics
// #define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
// #define CHARACTERISTIC_READ "abcd0001-1234-1234-1234-123456789abc"
// #define CHARACTERISTIC_WRITE "abcd0002-1234-1234-1234-123456789abc"
// #define CHARACTERISTIC_NOTIFY "abcd0003-1234-1234-1234-123456789abc"

// NimBLECharacteristic *notifyCharacteristic;
// NimBLECharacteristic *writeCharacteristic;
// NimBLECharacteristic *readCharacteristic;
// bool deviceConnected = false;
// NimBLEServer *pServer;
// NimBLEService *pService;

// // Server Callbacks
// class MyServerCallbacks : public NimBLEServerCallbacks {

//     void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
//         deviceConnected = true;
//         Serial.println("Client connected!");
//     }

//     void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
//         deviceConnected = false;
//         Serial.println("Client disconnected!");


//         // Restart advertising after disconnect
//         NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
//         pAdvertising->start();
//         Serial.println("Advertising restarted...");
//     }
// };

// // Write Callback
// class MyWriteCallbacks : public NimBLECharacteristicCallbacks {

//     void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override {
//         std::string value = pCharacteristic->getValue();
//         readCharacteristic->setValue(value);
//         notifyCharacteristic->setValue(value);
//         notifyCharacteristic->notify();
//         if (value.length() > 0) {
//             Serial.print("Write Received: ");
//             Serial.println(value.c_str());
//         }
//     }
// };

// void configureBleCommunication() {
//     Serial.println("Starting BLE Server...");

//     // Initialize NimBLE
//     NimBLEDevice::init("POWERN");
//     NimBLEDevice::setMTU(255);
//     NimBLEDevice::setDeviceName(liveObj.mac);
//     NimBLEDevice::setPower(9);

//     // Create BLE Server
//     pServer = NimBLEDevice::createServer();
//     pServer->setCallbacks(new MyServerCallbacks());

//     // Create BLE Service
//     pService = pServer->createService(SERVICE_UUID);

//     // Create Read Characteristic
//     readCharacteristic = pService->createCharacteristic(
//         CHARACTERISTIC_READ,
//         NIMBLE_PROPERTY::READ
//     );
//     readCharacteristic->setValue("");

//     // Create Write Characteristic
//     writeCharacteristic = pService->createCharacteristic(
//         CHARACTERISTIC_WRITE,
//         NIMBLE_PROPERTY::WRITE
//     );
//     writeCharacteristic->setCallbacks(new MyWriteCallbacks());

//     // Create Notify Characteristic
//     notifyCharacteristic = pService->createCharacteristic(
//         CHARACTERISTIC_NOTIFY,
//         NIMBLE_PROPERTY::NOTIFY
//     ); 
//     // Start the service
//     pService->start();

//     // Start advertising
//     // pServer->getAdvertising()->start();

//     // Start advertising
//     NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
//     // pAdvertising->addServiceUUID(SERVICE_UUID);

//     // Set the custom advertisement name
    
//     pAdvertising->setName(((String)"IOT_" + liveObj.mac).c_str());

//     pAdvertising->start();

//     Serial.println("Waiting for a client to connect...");
// }

// void processCommunication() {
//     // Notify connected client with real-time data
//     // if (deviceConnected) {
//     //     notifyCharacteristic->setValue("Notification Data");
//     //     notifyCharacteristic->notify();
//     //     delay(1000);  // Notify every second
//     // }
// }
