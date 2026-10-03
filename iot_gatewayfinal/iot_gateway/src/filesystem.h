#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_
#include "FS.h"
#include "SPIFFS.h"
/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

void testFilesystem();
void testFileIO(fs::FS &fs, const char * path);
void deleteFile(fs::FS &fs, const char * path);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void appendFile(fs::FS &fs, const char * path, const char * message);
void writeFile(fs::FS &fs, const char * path, const char * message);
void readFile(fs::FS &fs, const char * path);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
int32_t saveFile(fs::FS &fs, const char * path, const uint8_t * message, uint32_t length);
int32_t loadFile(fs::FS &fs, const char * path, uint8_t * message, uint32_t count);

#endif

