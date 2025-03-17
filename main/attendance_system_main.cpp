#include "utils.h"

TaskHandle_t FingerprintScannerTaskHandler = NULL;
TaskHandle_t DisplayTaskHandler = NULL;
TaskHandle_t GoliothTaskHandler = NULL;

QueueHandle_t fingerprintScannerQueue;

void Fingerprint_Scanner_Task(void *arg);
void Display_Task(void *arg);
void Golioth_Task(void *arg);

TFT_eSPI tft = TFT_eSPI(); 
extern TFT_eSPI_Button key[15];       
extern TFT_eSPI_Button mainkeys[3];

HardwareSerial fingerprintSerial(2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprintSerial);



extern const char* psk_id;
extern const char* psk;

extern const char *TAG;

extern char keyLabel[15][5];
extern char numberBuffer[NUM_LEN + 1];
extern uint8_t numberIndex;

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
  xTaskCreate(Fingerprint_Scanner_Task, "Fingerprint Scanner", 4096, NULL, 5, &FingerprintScannerTaskHandler);
  xTaskCreate(Golioth_Task, "Golioth", 4096, NULL, 5, &GoliothTaskHandler);
  fingerprintScannerQueue = xQueueCreate(5, sizeof(scannerJob));

}

void Fingerprint_Scanner_Task(void *arg)
{
  printf("Task running: Fingerprint Scanner Task .. \n");
  fingerprintSerial.begin(115200, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  struct scannerJob jobReceived; 

  while(!finger.verifyPassword()){
  Serial.println("Did not find fingerprint sensor :(");
  vTaskDelay(4000 / portTICK_PERIOD_MS); 
  }
  Serial.println("Found fingerprint sensor!");

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
    Serial.print("Sensor doesn't contain any fingerprint data. Please use the 'enroll' feature to add fingerprints.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  } 

  while(1){
    if(xQueueReceive(fingerprintScannerQueue, &jobReceived, 0)){
      if (jobReceived.jobCode == 'e'){
        getFingerprintEnroll(jobReceived.fingerprintID);
        printf("finished scan job");
      }
      else if (jobReceived.jobCode == 'd'){
        deleteFingerprint(jobReceived.fingerprintID);
        printf("finished delete job");
      }
      else if (jobReceived.jobCode == 'v'){
        getFingerprintID();
        printf("finished verify job");
      }
      else if (jobReceived.jobCode == 'm'){
        finger.emptyDatabase();    
        printf("Now the database is empty");
      }
    }
    vTaskDelay(100/portTICK_PERIOD_MS);
  } 
}
 

void Display_Task(void *arg)
{
  extern uint8_t onScreen; 
  struct scannerJob jobToSend; 
  Serial.println("starting display");
  tft.init();
  tft.setRotation(0);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);

  uint16_t calData[5] = { 225, 3565, 221, 3705, 7 };
  tft.setTouch(calData);

  // Draw keypad
  drawMainmenu();

  while(1){
    uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

    uint8_t fingerprint_id; //To store the fingerprint ID typed by the user in the keypad

    // Pressed will be set true is there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);

    if (onScreen == MAINMENU){
      for (uint8_t i = 0; i < 3; i++) {
        if (pressed && mainkeys[i].contains(t_x, t_y)) {
          mainkeys[i].press(true);  // tell the button it is pressed
        } else {  
          mainkeys[i].press(false);  // tell the button it is NOT pressed
        }
      }
      for (uint8_t i = 0; i < 3; i++) {
        tft.setFreeFont(MAINMENU_FONT);
        if (mainkeys[i].justReleased()) mainkeys[i].drawButton();     // draw normal

        if (mainkeys[i].justPressed()) {   
          mainkeys[i].drawButton(true);  // draw invert

          if (i == 0){   //enroll button
            drawKeypad();
            status("Please enter a number between 1 and ", 10, 0);
            tft.drawString("127 to select a fingerprint ID for the ", 10, 25);
            tft.drawString("fingerprint to be enrolled", 10, 50);
            jobToSend.jobCode = 'e';
          }
          if (i == 1){ //verify button
            status("Please place your finger on the", 10, 0);
            tft.drawString("scanner", 10, 25);
            jobToSend.jobCode = 'v';
            jobToSend.fingerprintID = 0;
            xQueueSend(fingerprintScannerQueue, &jobToSend, 0);
          }
          if (i == 2){
            drawKeypad(); //delete button
            status("Please enter the fingerprint ID of the", 10, 0);
            tft.drawString("fingerprint to be deleted", 10, 25);
            jobToSend.jobCode = 'd';
          }
          vTaskDelay(10 / portTICK_PERIOD_MS); // UI debouncing
        }
      }   
    }
    else if(onScreen == KEYPAD){
      // / Check if any key coordinate boxes contain the touch coordinates
      for (uint8_t b = 0; b < 16; b++) {
        if (pressed && key[b].contains(t_x, t_y)) {
          key[b].press(true);  // tell the button it is pressed
        } else {
          key[b].press(false);  // tell the button it is NOT pressed
        }
      } 
      // Check if any key has changed state
      for (uint8_t b = 0; b < 16; b++) {

        if ((b - 3)% 4 == 0) tft.setFreeFont(LABEL1_FONT);
        else tft.setFreeFont(LABEL2_FONT);

        if (key[b].justReleased()) key[b].drawButton();     // draw normal

        if (key[b].justPressed()) {
          key[b].drawButton(true);  // draw invert

          // if a numberpad button, append the relevant # to the numberBuffer
          if ((b - 3)% 4 != 0) {
            if (numberIndex < NUM_LEN) {
              numberBuffer[numberIndex] = keyLabel[b][0];
              numberIndex++;
              numberBuffer[numberIndex] = 0; // zero terminate
            }
            status("", 0, 0); // Clear the old status
          }

          // Del button, so delete last char
          if (b == 7) {
            numberBuffer[numberIndex] = 0;
            if (numberIndex > 0) {
              numberIndex--;
              numberBuffer[numberIndex] = 0;//' ';
            }
            status("", 0, 0); // Clear the old status
          }

          if (b == 11) {
            status("Sent value to serial port", 0, 0);
            Serial.println(numberBuffer);
            fingerprint_id = 0;
            for(int i = 0; i < numberIndex; i++){
              fingerprint_id += ((((int)numberBuffer[i]) -48)* pow(10,(numberIndex-1-i)));
            }
            if (fingerprint_id == 0 && jobToSend.jobCode == 'd'){
              jobToSend.jobCode = 'm';
            }
            else if(fingerprint_id > 127 || fingerprint_id < 1){
              Serial.println("invalid fingerprint ID");
              break;
              }
            jobToSend.fingerprintID = fingerprint_id;
            xQueueSend(fingerprintScannerQueue, &jobToSend, 0);
            numberIndex = 0; // Reset index to 0
            numberBuffer[numberIndex] = 0; // Place null in buffer
          }
          // we dont really check that the text field makes sense
          // just try to call
          if (b == 3) {
            status("Value cleared", 0, 0);
            numberIndex = 0; // Reset index to 0
            numberBuffer[numberIndex] = 0; // Place null in buffer
          }

          //back button
          if (b == 15) {
            drawMainmenu();
            numberIndex = 0; // Reset index to 0
            numberBuffer[numberIndex] = 0; // Place null in buffer
            break;
          }

          // Update the number display field
          tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
          tft.setFreeFont(&FreeSans18pt7b);  // Choose a nice font that fits box
          tft.setTextColor(DISP_TCOLOR);     // Set the font colour

          // Draw the string, the value returned is the width in pixels
          int xwidth = tft.drawString(numberBuffer, DISP_X + 4, DISP_Y + 12);

          // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
          // but it will not work with italic or oblique fonts due to character overlap.
          tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);
          
          vTaskDelay(10 / portTICK_PERIOD_MS); // UI debouncing
        }
      }
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
      }
    }
  };

  struct golioth_client *client = golioth_client_create(&config);
  assert(client);
  uint16_t counter = 0;
  while(1) {
    GLTH_LOGI(TAG, "Hello, Golioth! #%d", counter);
    ++counter;
    vTaskDelay(60000 / portTICK_PERIOD_MS);
  }
}
 