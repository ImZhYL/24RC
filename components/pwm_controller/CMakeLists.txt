idf_component_register(SRCS "pwm_controller.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver 
                            esp_timer
                            gpio_table)

