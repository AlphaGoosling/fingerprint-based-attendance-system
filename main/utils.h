#ifndef UTILS_H_
#define UTILS_H_
 
#include "Arduino.h"
#include "Adafruit_Fingerprint.h"
#include "TFT_eSPI.h"
#include "SPI.h"

extern "C"{
    #include "string.h"
    #include "stdio.h"
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
    #include "golioth/client.h"  
    #include "freertos/queue.h"
}
 
#define esp_wifi_ssid      "redmi-black"
#define esp_wifi_password      "77777777"
#define EXAMPLE_ESP_MAXIMUM_RETRY  0

extern const char* psk_id;
extern const char* psk;

struct scannerJob{
    char jobCode;
    uint8_t fingerprintID;
};

#define MAINMENU 0
#define KEYPAD 1

#define SCREEN_W 320
#define SCREEN_H 480

//Main menu key sizes
#define MAINKEY_H 40
#define MAINKEY_W 100
// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 296
#define KEY_W 62 // Width and height
#define KEY_H 30
#define KEY_SPACING_X 18 // X and Y gap
#define KEY_SPACING_Y 20
#define KEY_TEXTSIZE 1   // Font size multiplier

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT   &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT   &FreeSansBold12pt7b    // Key label font 2
#define MAINMENU_FONT &FreeSerifBold12pt7b
#define FSS9          &FreeSans9pt7b 

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 210
#define DISP_W 318
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Number length, buffer for storing it and character index
#define NUM_LEN 12

// We have a status line for messages
#define STATUS_X 20 // Centred on this
#define STATUS_Y 20


extern "C" void wifi_init_sta(void);

uint8_t getFingerprintID();
uint8_t getFingerprintEnroll(uint8_t fingerprintID);
int     getFingerprintIDez();
uint8_t deleteFingerprint(uint8_t fingerprint_id);
uint8_t readnumber(void);
uint8_t downloadFingerprintTemplate(uint16_t fingerprint_id);
void    printHex(int num, int precision);

void drawKeypad();
void drawMainmenu();
void status(const char *msg, int x, int y);

#endif 