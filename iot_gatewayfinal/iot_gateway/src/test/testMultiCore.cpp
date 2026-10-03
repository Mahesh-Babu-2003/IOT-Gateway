// #include <Arduino.h>

// // Define Task Handles
// TaskHandle_t Task1;
// TaskHandle_t Task2;

// // Function to run on Core 0
// void Task1Code(void* pvParameters) {
//   Serial.print("Task 1 running on core ");
//   Serial.println(xPortGetCoreID());

//   while (true) {
//     Serial.println("Task 1 is running...");
//     delay(1000);
//   }
// }

// // Function to run on Core 1
// void Task2Code(void* pvParameters) {
//   Serial.print("Task 2 running on core ");
//   Serial.println(xPortGetCoreID());

//   while (true) {
//     Serial.println("Task 2 is running...");
//     delay(2000);
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   Serial.println("Setup started");

//   // Create Task 1 on Core 0
//   xTaskCreatePinnedToCore(
//     Task1Code,   // Function to run
//     "Task1",     // Name of task
//     10000,       // Stack size in bytes
//     NULL,        // Parameter to pass to the task
//     1,           // Task priority
//     &Task1,      // Task handle
//     0            // Core 0
//   );

//   // Create Task 2 on Core 1
//   xTaskCreatePinnedToCore(
//     Task2Code,   // Function to run
//     "Task2",     // Name of task
//     10000,       // Stack size in bytes
//     NULL,        // Parameter to pass to the task
//     1,           // Task priority
//     &Task2,      // Task handle
//     1            // Core 1
//   );

//   Serial.println("Setup complete");
// }

// void loop() {
//   // Main loop can also run on Core 1 (default behavior in Arduino framework)
//   Serial.println("Loop running on core " + String(xPortGetCoreID()));
//   delay(3000);
// }
