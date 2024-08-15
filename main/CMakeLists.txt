# Register the main component
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES freertos esp_ringbuf xtensa)