
#include <Arduino.h>
#include "esp_system.h"
#include "esp_flash.h"

void flashTest(){ 
  
    size_t free_ram = esp_get_free_heap_size();
    
    Serial.print("Free RAM size: ");
    Serial.println(free_ram);

    esp_flash_t* flash = nullptr;   
    // Initialize the flash pointer
    flash = esp_flash_default_chip;

    
    // Initialize the flash
    esp_err_t err = esp_flash_init(flash);
    if (err != ESP_OK) {
        Serial.printf("Flash initialization failed: %s\n", esp_err_to_name(err));
        return;
    }

    // Get the size of the flash memory
    uint32_t flash_size;
    err = esp_flash_get_size(flash, &flash_size);
    if (err != ESP_OK) {
        Serial.printf("Failed to get flash size: %s\n", esp_err_to_name(err));
        return;
    }

    Serial.printf("Flash size: %u bytes\n", flash_size);
}