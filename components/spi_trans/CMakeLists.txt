idf_component_register(SRCS "spi_trans.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi
                    driver
                    nvs_flash
                    protocol_examples_common
                    gpio_table
                    my_udp_client
                    )

