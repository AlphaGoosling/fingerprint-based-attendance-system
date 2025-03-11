idf_component_register(
    SRCS "utils.cpp" "attendance_system_main.cpp" "golioth_credentials.cpp" 
    INCLUDE_DIRS "." 
    PRIV_REQUIRES golioth_sdk 
                  Adafruit-Fingerprint-Sensor-Library 
                  tft_espi
    REQUIRES arduino 
)
