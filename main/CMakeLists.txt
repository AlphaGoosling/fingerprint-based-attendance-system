idf_component_register(
    SRCS "attendance_system_main.cpp" "golioth_credentials.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES golioth_sdk
    REQUIRES arduino
)
