#ifndef _GPIO_TABLE_H_
#define _GPIO_TABLE_H_

#define HOST_IP_ADDR "192.168.137.1" // IP地址
#define TCP_PORT 29999                    // 端口号
#define UDP_PORT 30000
//腿部方波驱动信号

extern const uint8_t left_leg_x_pin;
extern const uint8_t left_leg_y_pin;
extern const uint8_t right_leg_x_pin;
extern const uint8_t right_leg_y_pin;
extern const uint8_t clamp_a_phase_pin;
extern const uint8_t clamp_b_phase_pin;
extern const uint8_t gen_gpios[3][2];
//调压PWM信号
extern const uint8_t boost_pwm_pin;
//控制芯片的io口
//uart io口
extern const uint8_t uart_tx_pin; //假设uart_tx的值为19
extern const uint8_t uart_rx_pin;//假设uart_rx的值为21

//ID
extern const uint8_t left_leg_id;
extern const uint8_t right_leg_id;
//初始电压，用作电压校准用
extern const float max_voltage;

//腿部编号
extern const uint8_t left_leg_id;
extern const uint8_t right_leg_id;
extern const uint8_t clamp_id;
//一条腿有两条通道
extern const uint8_t x_id;
extern const uint8_t y_id;
//设备编号
extern const uint8_t host_computer_id;
extern const uint8_t openmv_board_id;
//spi通信
extern const uint8_t spi_cs_pin;
extern const uint8_t spi_mosi_pin;
extern const uint8_t spi_miso_io;
extern const uint8_t spi_clk_pin;
//其他使能引脚
extern const uint8_t openmv_rst_pin;
extern const uint8_t opa_lx_en_pin;
extern const uint8_t opa_ly_en_pin;
extern const uint8_t opa_rx_en_pin;
extern const uint8_t opa_ry_en_pin;
extern const uint8_t clamp_a_en2_pin;
extern const uint8_t clamp_b_en2_pin;
//led
extern const uint8_t led_pin;
#endif
