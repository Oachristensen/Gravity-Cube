# Register the main component
idf_component_register(SRCS "hardware_logic/panel_data.c" "IMU_lib/icm20948_spi_lib.c" "sim_logic/sim_functions.c" "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES freertos esp_ringbuf xtensa)