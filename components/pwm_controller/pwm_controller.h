 #ifndef _PWM_CONTROLLER_H_
 #define _PWM_CONTROLLER_H_



//pwm初始化相关
void pwm_controller_init(void);
//操作函数
void enable_all_channels(void);//开启所有输出口，清除所有关闭状态
void disable_all_channels(void);//禁用所有输出口
void enable_pulse_output(void);
void disable_pulse_output(void);
void set_channel_freq(uint8_t leg_id,uint32_t freq);//改变腿频率
void enable_all_channels(void);
void disable_all_channels(void);
void enable_one_leg(uint8_t leg_id);
void disable_one_leg(uint8_t leg_id);
//钳子
void shutdown_clamp(void);
void startup_clamp(bool dir,uint8_t duration);
//调整相位值
void set_channel_phase(uint8_t leg_id,double phase);//phase=90
void channels_reverse_phase(uint8_t leg_id);
//调整脉冲占空比
void set_pulse_duty(double duty);//duty=0.01~99.99
void set_pulse_freq(float freq);//freq=0.1~10hz
//内部函数
void enable_all_pwm_timers(void);
void clamp_timer_init(void);
void startup_pwm_timer(uint8_t timer_id);
void shutdown_pwm_timer(uint8_t timer_id);
#endif



