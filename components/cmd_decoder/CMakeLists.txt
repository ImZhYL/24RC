idf_component_register(SRCS "cmd_decoder.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver 
                            esp_timer
                            pwm_controller
                            my_tcp_client
		                boost_controller
                                gpio_table
                                boost_controller
                                uart_controller
                                device_manager)

