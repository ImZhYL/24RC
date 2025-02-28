idf_component_register(SRCS "wifi_connect.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi
                    nvs_flash
                    protocol_examples_common
                    gpio_table
                    )

