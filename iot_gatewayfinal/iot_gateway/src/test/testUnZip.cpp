// #include <SPIFFS.h>
// #include "unzip.h"  // MiniUnzip header

// void extractZipFile(const char* zipFilePath, const char* extractPath) {
//     // Open the ZIP file
//     File zipFile = SPIFFS.open(zipFilePath, "r");
//     if (!zipFile) {
//         Serial.println("Failed to open ZIP file");
//         return;
//     }

//     unzFile zip = unzOpen(NULL, &zipFile);
//     if (!zip) {
//         Serial.println("Failed to open ZIP archive");
//         zipFile.close();
//         return;
//     }

//     // Navigate through files in the archive
//     if (unzGoToFirstFile(zip) != UNZ_OK) {
//         Serial.println("No files in ZIP");
//         unzClose(zip);
//         zipFile.close();
//         return;
//     }

//     do {
//         // Get the current file name
//         char filename[256];
//         unzGetCurrentFileInfo(zip, NULL, filename, sizeof(filename), NULL, 0, NULL, 0);

//         // Open the current file
//         if (unzOpenCurrentFile(zip) != UNZ_OK) {
//             Serial.println("Failed to open file inside ZIP");
//             continue;
//         }

//         // Create a new file in SPIFFS to write the extracted content
//         String filePath = String(extractPath) + "/" + String(filename);
//         File outFile = SPIFFS.open(filePath, "w");
//         if (!outFile) {
//             Serial.println("Failed to create file in SPIFFS");
//             unzCloseCurrentFile(zip);
//             continue;
//         }

//         // Read and write the file in chunks
//         const size_t chunkSize = 1024;
//         uint8_t buffer[chunkSize];
//         int bytesRead;
//         while ((bytesRead = unzReadCurrentFile(zip, buffer, chunkSize)) > 0) {
//             outFile.write(buffer, bytesRead);
//         }

//         outFile.close();
//         unzCloseCurrentFile(zip);

//         Serial.println("Extracted: " + filePath);
//     } while (unzGoToNextFile(zip) == UNZ_OK);

//     unzClose(zip);
//     zipFile.close();
// }

// void setup() {
//     Serial.begin(115200);

//     // Mount SPIFFS
//     if (!SPIFFS.begin(true)) {
//         Serial.println("Failed to mount SPIFFS");
//         return;
//     }

//     // Extract ZIP file
//     extractZipFile("/example.zip", "/extracted");
// }

// void loop() {}
