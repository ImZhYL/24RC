idf_component_register(SRCS "my_tcp_client.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi
                    nvs_flash
                    protocol_examples_common
                    cmd_decoder
                    gpio_table
                    )

