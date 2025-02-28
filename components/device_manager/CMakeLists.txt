idf_component_register(SRCS "device_manager.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver 
                            gpio_table)

