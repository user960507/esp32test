idf_component_register(SRCS "ledc_basic_example_main.c"
                    PRIV_REQUIRES esp_driver_gpio esp_driver_tsens esp_wifi nvs_flash
                    INCLUDE_DIRS ".")
