idf_component_register(SRCS "uart_controller.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver 
                    esp_timer
                    gpio_table
                    cmd_decoder
                    )

