#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "driver/mcpwm_prelude.h"
#include "pwm_controller.h"
#include "driver/gptimer.h"
#include "freertos/queue.h"
#include "gpio_table.h"
const uint32_t period=2000;
uint32_t timer_period_ticks[3]={period,period,period} ;    // 频率为80000000/2000/2=20000hz

const uint32_t timer_resoulution_hz=80000000; // 80M


const uint8_t pwm_timer_num=3;
const uint8_t oper_num=3;
const uint8_t comp_per_oper=2;
const uint8_t gen_per_oper=2;
bool pulse_output_state=false;
uint32_t cmp_value[3][2]={{period/4,3*period/4},{period/4,3*period/4},{period/4,3*period/4}};
static const char *TAG = "PWM_Control";
/*******************************定义pwm_timer相关*******************************/
mcpwm_timer_handle_t pwm_timer[3] = {}; // timer 的handle=timer
mcpwm_timer_config_t pwm_timer_config =
    {
        .group_id = 0, // 组别0
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = timer_resoulution_hz,   // 80000000
        .period_ticks = period*2,      // 计时器TICKS*25600=RESOLUTION
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP_DOWN, // 上计数模式
};

mcpwm_oper_handle_t pwm_operators[3] = {}; // operator和operator配置，要和timer一个group，operator的handle为leg1_oper
mcpwm_operator_config_t pwm_operator_config = {
    .group_id = 0, // operator must be in the same group to the timer
};

mcpwm_cmpr_handle_t pwm_comparators[3][2] = {};
mcpwm_comparator_config_t pwm_comparator_config = {
    .flags.update_cmp_on_tez = true,
};

mcpwm_gen_handle_t pwm_generators[3] [2]= {};
mcpwm_generator_config_t pwm_generator_config = {};
/*******************************定义pulse_timer相关*******************************/
// 消息队列结构体
typedef struct {
    uint64_t event_count;
} example_queue_element_t;
// 定义脉冲定时器中断句柄
gptimer_handle_t pulse_timer = NULL;
//定义脉冲状态变量，true=on，false=off
volatile bool pulse_state=true;
//定时器中断警报值
uint64_t pulse_value1 = 100000;
uint64_t pulse_value2 = 9900000;
uint64_t sum_value=1000000;
float pulse_freq=1;
// 定义钳子定时器中断句柄
gptimer_handle_t clamp_timer =NULL;
uint64_t clamp_value = 5000000;
float clamp_freq=1000;
/*****************************************
 *脉冲定时器中断回调函数
**************************************/
static bool IRAM_ATTR example_timer_on_alarm_cb_v1(gptimer_handle_t pulse_timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    int level=0;
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = edata->alarm_value, // alarm in next 1s
    };
    if (pulse_state)
    {
        //mcpwm_generator_set_force_level(generator, level,1);
        gptimer_set_raw_count(pulse_timer, 0);
        alarm_config.alarm_count = pulse_value2;
        pulse_state=false;
        level=0;
    }
    else
    {
        //mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
        //mcpwm_generator_set_force_level(generator, -1,1);
        gptimer_set_raw_count(pulse_timer, 0);
        alarm_config.alarm_count = pulse_value1;
        pulse_state=true;
        level=-1;
    }
    gptimer_set_alarm_action(pulse_timer, &alarm_config);
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[left_leg_id][x_id],level,1));  
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[left_leg_id][y_id],level,1));
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[right_leg_id][x_id],level,1));
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[right_leg_id][y_id],level,1));

    // return whether we need to yield at the end of ISR
    return true;
}

/*****************************************
 *钳子定时器中断回调函数
**************************************/
bool clamp_timer_on_alarm_cb(gptimer_handle_t clamp_timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    shutdown_clamp();
    gptimer_stop(clamp_timer);
    return true;
}
/******************************************
函数：timer初始化
********************************************/
void pwm_timer_init(void)
{
    ESP_LOGI(TAG, "Create %d timers", pwm_timer_num);
    for(int i=0;i<pwm_timer_num;i++){
        ESP_ERROR_CHECK(mcpwm_new_timer(&pwm_timer_config, &pwm_timer[i])); // new  timer 函数创建一个timer
    }
}

/******************************************
函数：oparator初始化,3个operator
********************************************/
void pwm_operator_init(void)
{
    ESP_LOGI(TAG, "Creat operators and connect to the timer");

    for (int i = 0; i < oper_num; i++)
    {
        ESP_ERROR_CHECK(mcpwm_new_operator(&pwm_operator_config, &pwm_operators[i]));
    } // new_operator 创建operator

    for (int i = 0; i < oper_num; i++)
    {
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(pwm_operators[i], pwm_timer[i]));
    } // 连接至timer
}

/******************************************
函数：comparator初始化,4个comparator
********************************************/
void pwm_comparator_init(void)
{
    for (int i = 0; i < oper_num; i++)
    {
        for (int j = 0; j < comp_per_oper; j++)
        {
            ESP_ERROR_CHECK(mcpwm_new_comparator(pwm_operators[i], &pwm_comparator_config, &pwm_comparators[i][j]));
            // set compare value to 0, we will adjust the speed in a period timer callback
        }
    }
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[left_leg_id][x_id], cmp_value[left_leg_id][x_id])); //=400/2/2
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[left_leg_id][y_id], cmp_value[left_leg_id][y_id]));     //=400/2/2
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[right_leg_id][x_id], cmp_value[right_leg_id][x_id]));     //=400/2/2
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[right_leg_id][y_id], cmp_value[right_leg_id][y_id]));
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[clamp_id][x_id], cmp_value[clamp_id][x_id]));
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[clamp_id][y_id], cmp_value[clamp_id][y_id]));
        ESP_LOGI(TAG, "SUCESSFULLY SET COMPARATOR");
}

/******************************************
函数：generator初始化,4个generator
********************************************/
 void pwm_generator_init(void)
{
    ESP_LOGI(TAG, "Create PWM generators");
    // 配置generator
    for (int i = 0; i < oper_num; i++)
    {
        for (int j = 0; j < gen_per_oper; j++)
        {
            pwm_generator_config.gen_gpio_num = gen_gpios[i][j];
            pwm_generator_config.flags.pull_down=1;
            pwm_generator_config.flags.pull_up=1;
            ESP_ERROR_CHECK(mcpwm_new_generator(pwm_operators[i], &pwm_generator_config, &pwm_generators[i][j]));
        }
    }
    ESP_LOGI(TAG, "Set generators actions");
    for (int i = 0; i < oper_num; i++)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(pwm_generators[i][x_id],
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, pwm_comparators[i][x_id], MCPWM_GEN_ACTION_HIGH),
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, pwm_comparators[i][y_id], MCPWM_GEN_ACTION_LOW),
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
        ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(pwm_generators[i][y_id],
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, pwm_comparators[i][y_id], MCPWM_GEN_ACTION_HIGH),
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, pwm_comparators[i][x_id], MCPWM_GEN_ACTION_LOW),
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    }
}

/******************************************
函数：初始化脉冲输出
********************************************/
void pulse_output_init(void)
{
//创建timer句柄
    ESP_LOGI(TAG, "Create timer handle");
    gptimer_config_t gptimer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&gptimer_config, &pulse_timer));
    //配置回调函数
    gptimer_event_callbacks_t cbs = {
        .on_alarm = example_timer_on_alarm_cb_v1,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(pulse_timer, &cbs, NULL));
 
    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(pulse_timer));
    // 配置报警
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = pulse_value1, // period = 1s
        .flags.auto_reload_on_alarm = false,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(pulse_timer, &alarm_config));
}

/******************************************
函数：初始化clamp模块定时器
********************************************/
void clamp_timer_init(void)
{
//创建timer句柄
    ESP_LOGI(TAG, "Create clamp timer handle");
    gptimer_config_t gptimer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&gptimer_config, &clamp_timer));
    //配置回调函数
    gptimer_event_callbacks_t cbs = {
        .on_alarm = clamp_timer_on_alarm_cb,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(clamp_timer, &cbs, NULL));
 
    ESP_LOGI(TAG, "Enable clamp timer");
    ESP_ERROR_CHECK(gptimer_enable(clamp_timer));
    // 配置报警
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = clamp_value, // period = 3s
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(clamp_timer, &alarm_config));
}

/******************************************
函数：初始化所有模块
********************************************/
void pwm_controller_init(void)
{
    pwm_timer_init();
    pwm_operator_init();
    pwm_comparator_init();
    pwm_generator_init();
    pulse_output_init();
    clamp_timer_init();
    enable_all_pwm_timers();
    disable_all_channels();
    set_channel_freq(clamp_id,1000);
    set_channel_phase(clamp_id,90);
}

/******************************************
函数：使能MCPWM定时器
********************************************/
void enable_all_pwm_timers(void)
{
    ESP_LOGI(TAG, "Enable and start all timers");
    for(int i=0;i<pwm_timer_num;i++){
    ESP_ERROR_CHECK(mcpwm_timer_enable(pwm_timer[i]));
    }

}

/******************************************
函数：运行所有MCPWM定时器
********************************************/
void startup_all_pwm_timers(void)
{
    ESP_LOGI(TAG, "Start all timers");
    for(int i=0;i<pwm_timer_num;i++){
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(pwm_timer[i], MCPWM_TIMER_START_NO_STOP));
    }
}
/******************************************
函数：关闭所有MCPWM定时器
********************************************/
void shutdown_all_pwm_timers(void)
{
    ESP_LOGI(TAG, "Shutdown all timers");
    for(int i=0;i<pwm_timer_num;i++){
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(pwm_timer[i], MCPWM_TIMER_STOP_EMPTY));
    }
}
/******************************************
函数：运行某一个MCPWM定时器,输入对应left_leg_id/right_leg_id/CLAMP_ID
********************************************/
inline void startup_pwm_timer(uint8_t timer_id)
{
    //ESP_LOGI(TAG, "Start timer %d",timer_id);
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(pwm_timer[timer_id], MCPWM_TIMER_START_NO_STOP));
}
/******************************************
函数：关闭某一个MCPWM定时器,输入对应left_leg_id/right_leg_id/CLAMP_ID
********************************************/
inline void shutdown_pwm_timer(uint8_t timer_id)
{
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(pwm_timer[timer_id], MCPWM_TIMER_STOP_EMPTY));
}
/******************************************
函数：调整两相pwm相位差反相
********************************************/
void channels_reverse_phase(uint8_t leg_id)
{
    uint32_t temp;
    temp = cmp_value[leg_id][x_id];
    cmp_value[leg_id][x_id] = cmp_value[leg_id][y_id];
    cmp_value[leg_id][y_id] = temp;
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[leg_id][x_id], cmp_value[leg_id][0]));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[leg_id][y_id], cmp_value[leg_id][1]));
}

/******************************************
函数：设置通道相位
********************************************/
void set_channel_phase(uint8_t leg_id,double phase)//phase=90
{
    char *tag = "set_channel_phase";
    uint32_t cmpx=1;
    uint32_t cmpy=1;
    //判断phase合法性
    if (phase>180){
        ESP_LOGW(tag, "phase:%f is too high,set to 179", phase);
        phase=179.9;
    }
    else if (phase<-180){
        ESP_LOGW(tag, "phase:%f is too low,set to -179", phase);
        phase=-179.9;
    }
    //计算cmpx
    cmpx=((phase+180)*1.0/360.0)*timer_period_ticks[leg_id];
    //判断cmpx合法性
    if (cmpx>=timer_period_ticks[leg_id]){
        ESP_LOGW(tag, "cmpx:%lu is too HIGH,set to timer_period_ticks-1: %lu", cmpx,timer_period_ticks[leg_id]-1);
        cmpx=timer_period_ticks[leg_id]-1;
    }
    else if (cmpx==0)
    {
        ESP_LOGW(tag, "cmpx:%lu is too LOW,set to 1",cmpx);
        cmpx=1;
    }
    cmpy=timer_period_ticks[leg_id]-cmpx;
    cmp_value[leg_id][x_id]=cmpx;
    cmp_value[leg_id][y_id]=cmpy;
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[leg_id][x_id],cmp_value[leg_id][x_id]));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[leg_id][y_id],cmp_value[leg_id][y_id])); 
    ESP_LOGI(tag, "Set cmpx=%lu,cmpy=%lu ",cmpx,cmpy);

}
/******************************************
函数：清除所有关闭状态,使能所有输出，包括控制夹爪的pwm
********************************************/
void enable_all_channels(void)
{
    startup_all_pwm_timers();
    for(int i=0;i<oper_num;i++)
    {
        for(int j=0;j<gen_per_oper;j++)
        {
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[i][j], -1,1));  
        }
    }
}

/******************************************
函数：禁用所有输出口,包括控制夹爪的PWM信号
********************************************/
void disable_all_channels(void)
{
    shutdown_all_pwm_timers();
    for(int i=0;i<oper_num;i++)
    {
        for(int j=0;j<gen_per_oper;j++)
        {
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[i][j], 0,1));  
        }
    }
}

/******************************************
函数：开启某一腿两通道xy输出
********************************************/
void enable_one_leg(uint8_t leg_id)
{
    if(pulse_output_state){
        startup_pwm_timer(leg_id);
    }
    else{
        startup_pwm_timer(leg_id);
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[leg_id][x_id], -1,1)); 
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[leg_id][y_id], -1,1));  
    }
}
/******************************************
函数：禁用某一腿输出
********************************************/
void disable_one_leg(uint8_t leg_id)
{
    if (pulse_output_state){
        shutdown_pwm_timer(leg_id);
    }
    else{
    shutdown_pwm_timer(leg_id);
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[leg_id][x_id], 0,1));  
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[leg_id][y_id], 0,1));
    }
}

/******************************************
函数：使能脉冲输出
********************************************/
void enable_pulse_output(void)
{
    if (!pulse_output_state){
    ESP_ERROR_CHECK(gptimer_start(pulse_timer));
    pulse_output_state=true;
    ESP_LOGI(TAG, "pulse_output_enable");}
}

/******************************************
函数：禁用脉冲输出
********************************************/ 
void disable_pulse_output(void)
{
    if (pulse_output_state){
    ESP_ERROR_CHECK(gptimer_stop(pulse_timer));
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[left_leg_id][x_id],-1,1));  
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[left_leg_id][y_id],-1,1));
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[right_leg_id][x_id],-1,1));
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[right_leg_id][y_id],-1,1));
    pulse_output_state=false;
    ESP_LOGI(TAG, "pulse_output_disable");
    }
    else{
        ESP_LOGI(TAG, "pulse_output is not running");
    }
    
}
/******************************************
函数：设置通道频率
********************************************/
void set_channel_freq(uint8_t leg_id,uint32_t freq)
{
    char *tag = "set_channel_freq";
    if (freq>1000000){
        ESP_LOGW(tag, "freq:%lu is too HIGH,set to 1000000",freq);
        freq=1000000;
    }
    else if(freq<200){
        ESP_LOGW(tag, "freq:%lu is too LOW,set to 1000",freq);
        freq=200;
    }
    timer_period_ticks[leg_id] = timer_resoulution_hz / freq/2;
    ESP_LOGI(tag, "period=%lu", timer_period_ticks[leg_id]);
    uint32_t temp_x=timer_period_ticks[leg_id]*(1.0*cmp_value[leg_id][x_id]/(cmp_value[leg_id][x_id]+cmp_value[leg_id][y_id]));
    uint32_t temp_y=timer_period_ticks[leg_id]-temp_x;
    cmp_value[leg_id][x_id]=temp_x;
    cmp_value[leg_id][y_id]=temp_y;
    ESP_ERROR_CHECK(mcpwm_timer_set_period(pwm_timer[leg_id], timer_period_ticks[leg_id]*2));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[leg_id][x_id],cmp_value[leg_id][x_id]));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(pwm_comparators[leg_id][y_id],cmp_value[leg_id][y_id])); 
    ESP_LOGI(tag, "Set cmpx=%lu,cmpy=%lu ",temp_x,temp_y);
}

/******************************************
函数：开启钳子输出
********************************************/
void startup_clamp(bool dir,uint8_t duration)
{
    // if (duration>20){
    //     ESP_LOGW(TAG, "duration:%d is too HIGH,set to 20",duration);
    //     duration=20;
    // }
    // gptimer_alarm_config_t alarm_config = {
    //     .alarm_count = duration*1000000, // period = 3s
    // };
    // ESP_ERROR_CHECK(gptimer_set_alarm_action(clamp_timer, &alarm_config));
    if (dir){
        gpio_set_level(clamp_a_en2_pin, 1);
        gpio_set_level(clamp_b_en2_pin, 1);
        set_channel_phase(clamp_id,90);
    }
    else{
        gpio_set_level(clamp_a_en2_pin, 1);
        gpio_set_level(clamp_b_en2_pin, 1);
        set_channel_phase(clamp_id,-90);
    }
    ESP_LOGI(TAG, "Clamp START");
    startup_pwm_timer(clamp_id);
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[clamp_id][x_id], -1,1)); 
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[clamp_id][y_id], -1,1));  
    gptimer_start(clamp_timer);
}
/******************************************
函数：关闭钳子输出
********************************************/
void shutdown_clamp(void)
{
    gpio_set_level(clamp_a_en2_pin, 0);
    gpio_set_level(clamp_b_en2_pin, 0);
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[clamp_id][x_id], 0,1));
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(pwm_generators[clamp_id][y_id], 0,1));
    shutdown_pwm_timer(clamp_id);
}
/******************************************
函数：调整脉冲输出占空比
********************************************/
void set_pulse_duty(double duty) // 设置脉冲占空比,为duty%
{
    if (duty < 0.01)
    {
        duty = 0.01;
    }
    else if (duty > 99.99)
    {
        duty = 99.99;
    }
    uint64_t value_target = (uint64_t)(sum_value* duty/100);
    pulse_value1 = value_target;
    pulse_value2 = sum_value - value_target;
    ESP_LOGI(TAG, "Set pulse output duty to : %f%%", duty); 
}
void set_pulse_freq(float freq) // 设置脉冲频率,为freqHz
{
    if (freq < 0.1)
    {
        freq = 0.1;
    }
    else if (freq > 20)
    {
        freq = 20;
    }
    sum_value=(uint64_t)(sum_value/freq);
    pulse_value1 = (uint64_t)(pulse_value1/freq);
    pulse_value2 =sum_value- pulse_value1;
    ESP_LOGI(TAG, "Set pulse output freq to : %f Hz", freq); 
}

