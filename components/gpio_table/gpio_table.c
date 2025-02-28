#include <stdint.h> 

/***************************************************IO表*************************************************/
//腿部方波驱动信号
const uint8_t left_leg_x_pin = 9;//42; // 假设left_leg_x_io的值为13
const uint8_t left_leg_y_pin = 7;//41;
const uint8_t right_leg_x_pin = 5;//40;
const uint8_t right_leg_y_pin = 3;//39;
const uint8_t clamp_a_phase_pin = 45;//38;
const uint8_t clamp_b_phase_pin = 41;//37;

//调压PWM信号
const uint8_t boost_pwm_pin = 38;//假设boost_pwm_io的值为33
//控制芯片的io口

//uart通信端口
const uint8_t uart_tx_pin = 17; //假设uart_tx的值为19
const uint8_t uart_rx_pin = 18;//假设uart_rx的值为21 
//spi通信端口
const uint8_t spi_mosi_pin=11;
const uint8_t spi_clk_pin=12;
const uint8_t spi_cs_pin=10;
//const uint8_t spi_miso_io=13;

//openmv使能引脚
const uint8_t openmv_rst_pin=40;

//LED示意灯
const uint8_t led_pin=21;

//运放使能引脚
const uint8_t opa_lx_en_pin=14;
const uint8_t opa_ly_en_pin=8;
const uint8_t opa_rx_en_pin=6;
const uint8_t opa_ry_en_pin=4;
const uint8_t clamp_a_en2_pin=2;
const uint8_t clamp_b_en2_pin=42;
/***************************************************全局变量***********************************************/

// 初始化二维数组
const uint8_t gen_gpios[3][2] = {
    {left_leg_x_pin, left_leg_y_pin},
    {right_leg_x_pin, right_leg_y_pin},
    {clamp_a_phase_pin, clamp_b_phase_pin}
};

//腿部编号
const uint8_t left_leg_id=0;
const uint8_t right_leg_id=1;
const uint8_t clamp_id=2;
//一条腿有两条通道
const uint8_t x_id=0;
const uint8_t y_id=1;
//设备编号
const uint8_t host_computer_id=0;
const uint8_t openmv_board_id=1;
/*******************************************************常数********************************************* */
//初始电压，用作电压校准用
const float max_voltage=80.6;


