#ifndef UTILS_H_
#define UTILS_H_
 
#include "Arduino.h"
#include "Adafruit_Fingerprint.h"
#include "TFT_eSPI.h"
#include "FS.h"

extern "C"{
    #include <string.h>
    #include <stdio.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/event_groups.h"
    #include "esp_system.h"
    #include "esp_wifi.h"
    #include "esp_event.h"
    #include "esp_log.h"
    #include "nvs_flash.h"
    #include "lwip/err.h"
    #include "lwip/sys.h"  
    #include <golioth/client.h>  
}

#define CALIBRATION_FILE "/calibrationData"
 
#define esp_wifi_ssid      "redmi-black"
#define esp_wifi_password      "77777777"
#define EXAMPLE_ESP_MAXIMUM_RETRY  0

extern const char* psk_id;
extern const char* psk;

extern "C" void wifi_init_sta(void);



uint8_t id;

uint8_t getFingerprintID();
int     getFingerprintIDez();
uint8_t deleteFingerprint(uint8_t id);
uint8_t readnumber(void);
uint8_t getFingerprintEnroll();
uint8_t downloadFingerprintTemplate(uint16_t id);
void    printHex(int num, int precision);

#endif 