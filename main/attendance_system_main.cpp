#include "utils.h"

TaskHandle_t FingerprintScannerTaskHandler = NULL;
TaskHandle_t DisplayTaskHandler = NULL;
TaskHandle_t GoliothTaskHandler = NULL;

void Fingerprint_Scanner_Task(void *arg);
void Display_Task(void *arg);
void Golioth_Task(void *arg);


TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

HardwareSerial fingerprintSerial(2);



Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprintSerial);


#define CALIBRATION_FILE "/calibrationData"
 
#define esp_wifi_ssid      "redmi-black"
#define esp_wifi_password      "77777777"
#define EXAMPLE_ESP_MAXIMUM_RETRY  0

extern const char* psk_id;
extern const char* psk;

extern const char *TAG;


extern "C" void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    Serial.begin(115200);
    xTaskCreate(Display_Task, "Display", 4096, NULL, 5, &DisplayTaskHandler);
    xTaskCreate(Fingerprint_Scanner_Task, "Fingerprint Scanner", 4096, NULL, 4, &FingerprintScannerTaskHandler);
    xTaskCreate(Golioth_Task, "Golioth", 4096, NULL, 3, &GoliothTaskHandler);
}

void Fingerprint_Scanner_Task(void *arg)
{
  printf("Task running: Fingerprint Scanner Task .. \n");
  fingerprintSerial.begin(115200, SERIAL_8N1, 16, 17);
  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  vTaskDelay(10 / portTICK_PERIOD_MS); 
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { vTaskDelay(10000 / portTICK_PERIOD_MS); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  } 

  while(1){
    getFingerprintID();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void Display_Task(void *arg)
{
  uint16_t calibrationData[5];
  uint8_t calDataOK = 0;

  Serial.println("starting");

  tft.init();

  tft.setRotation(3);
  tft.fillScreen((0xFFFF));

  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
  tft.println("calibration run");

  // check file system
  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");

    SPIFFS.format();
    SPIFFS.begin();
  }

   // check if calibration file exists
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    fs::File f = SPIFFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)calibrationData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
  }
  if (calDataOK) {
    // calibration data valid
    tft.setTouch(calibrationData);
  } else {
    // data not valid. recalibrate
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    // store data
    fs::File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calibrationData, 14);
      f.close();
    }
  }

  tft.fillScreen((0xFFFF));

  while(1){
    uint16_t x, y;
    static uint16_t color;

    if (tft.getTouch(&x, &y)) {

      tft.setCursor(5, 5, 2);
      tft.printf("x: %i     ", x);
      tft.setCursor(5, 20, 2);
      tft.printf("y: %i    ", y);

      tft.drawPixel(x, y, color);
      color += 155;
    }
  } 
}

void Golioth_Task(void *arg)
{
    const struct golioth_client_config config = {
        .credentials = {
        .auth_type = GOLIOTH_TLS_AUTH_TYPE_PSK,
        .psk = {
            .psk_id = psk_id,
            .psk_id_len = strlen(psk_id),
            .psk = psk,
            .psk_len = strlen(psk),
    }}};
    struct golioth_client *client = golioth_client_create(&config);
    assert(client);
    uint16_t counter = 0;
    while(1) {
        GLTH_LOGI(TAG, "Hello, Golioth! #%d", counter);
        ++counter;
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}
 