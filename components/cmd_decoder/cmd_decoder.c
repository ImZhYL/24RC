#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/mcpwm_prelude.h"
#include "string.h"
// 自编头文件
#include "gpio_table.h"
#include "cmd_decoder.h"
#include "pwm_controller.h"
#include "my_tcp_client.h"
#include "boost_controller.h"
#include "uart_controller.h"
#include "device_manager.h"
static const char *TAG = "Decoder";
char *Remote_Command;
char *last_command = "SHUT"; // 初始化比较字符串last_command的值
uint8_t actv_dev_id = 0;     // 当前激活的设备id
bool sleeping_state = true;  // 睡眠状态
/******************************************
函数：遥控函数
********************************************/
uint8_t decode_and_run(char *rcvd_cmd, uint8_t dev_id)
{
    char *buf = (char *)malloc(strlen(rcvd_cmd) + 1);
    int n = sscanf(rcvd_cmd, "%s", buf);
    if (n == -1 || n == 0)
    {
        ESP_LOGI(TAG, "cmd is wrong");
    }
    if (strcmp(buf, "SWITCH") == 0)
    {
        if (dev_id == host_computer_id)
        {
            // 执行SWITCH命令
            uint8_t uppercom_id = 0;
            uint8_t n = sscanf(rcvd_cmd, "%*s %hhu", &uppercom_id);
            if (n == 1)
            {
                if (uppercom_id == host_computer_id)
                {
                    ESP_LOGI(TAG, "[SWITCH] Controlled by the PC");
                    char *sendmsg = "Controlled by the PC/I";
                    tcp_sendData(sendmsg);
                    actv_dev_id = host_computer_id;
                    suspend_openmv();
                }
                else if (uppercom_id == openmv_board_id)
                {
                    resume_openmv();
                    vTaskDelay(3500 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "[SWITCH] Controlled by the OpenMV Board");
                    char *sendmsg = "Controlled by the OpenMV Board/I";
                    tcp_sendData(sendmsg);
                    actv_dev_id = openmv_board_id;
                    char *cmd_SwitchToOpenmv = "FIND";

                    uart_sendData(cmd_SwitchToOpenmv);
                }
                else
                {
                    ESP_LOGW(TAG, "[SWITCH] Invalid ID: %d", uppercom_id);
                }
            }
            else
            {
                ESP_LOGW(TAG, "[SWITCH] Invalid command: %s", rcvd_cmd);
            }
        }
        else
        {
            ESP_LOGW(TAG, "[SWITCH] No permission, command from: %d", dev_id);
        }
    }
    else
    {
        if (dev_id == actv_dev_id)
        {
            // 执行SHUT命令 SHUT
            if (strcmp(buf, "SHUT") == 0)
            {
                disable_pulse_output();
                disable_all_channels();
            }
            else if (strcmp(buf, "RUN") == 0)
            {
                char pos_1;
                char pos_2;              // 左腿还是右腿，L或者R            // 前进还是后退，F或者B
                uint32_t freq_1, freq_2; // 前进速度，后退速度,单位Hz
                double pha1, pha2;
                int n = sscanf(rcvd_cmd, "%*s %c %lu %lf %c %lu %lf", &pos_1, &freq_1, &pha1, &pos_2, &freq_2, &pha2);
                if (n == 6)
                {
                    if (sleeping_state == true)
                    {
                        resume_walk_opas();
                        sleeping_state = false;
                    }
                    ESP_LOGI(TAG, "[RUN] %c leg freq=%lu phase=%f;[RUN] %c leg freq=%lu phase=%f", pos_1, freq_1, pha1, pos_2, freq_2, pha2);
                    char sendmsg[512];
                    snprintf(sendmsg, sizeof(sendmsg), "[RUN] %c leg freq=%lu phase=%f;[RUN] %c leg freq=%lu phase=%f/I", pos_1, freq_1, pha1, pos_2, freq_2, pha2);
                    tcp_sendData(sendmsg);
                    cmd_set_leg_pha_freq(pos_1, pha1, freq_1);
                    cmd_set_leg_pha_freq(pos_2, pha2, freq_2);
                    // 执行motion_control()函数会有20ms的时间，将两腿使能放在该函数后面，而不是写进这个函数是为了让两条腿尽量同时使能，而不是先使能左腿再使能右腿
                    if (pos_1 == 'L' || pos_2 == 'L')
                    {
                        enable_one_leg(left_leg_id);
                    }
                    if (pos_1 == 'R' || pos_2 == 'R')
                    {
                        enable_one_leg(right_leg_id);
                    }
                }
                else if (n == 3)
                {
                    if (sleeping_state == true)
                    {
                        resume_walk_opas();
                        sleeping_state = false;
                    }
                    ESP_LOGI(TAG, "[RUN] %c leg freq=%lu phase=%lf", pos_1, freq_1, pha1);
                    char sendmsg[512];
                    snprintf(sendmsg, sizeof(sendmsg), "[RUN] %c leg freq=%lu phase=%f/I", pos_1, freq_1, pha1);
                    tcp_sendData(sendmsg);
                    cmd_set_leg_pha_freq(pos_1, pha1, freq_1);
                    if (pos_1 == 'L')
                    {
                        enable_one_leg(left_leg_id);
                        disable_one_leg(right_leg_id);
                    }
                    else if (pos_1 == 'R')
                    {
                        enable_one_leg(right_leg_id);
                        disable_one_leg(left_leg_id);
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "[RUN] Invalid command: %s", rcvd_cmd);
                }
            }
            else if (strcmp(buf, "STOP") == 0)
            {
                // 执行STOP命令
                char stop_type;
                uint8_t n = sscanf(rcvd_cmd, "%*s %c", &stop_type);
                if (n == 1)
                {
                    switch (stop_type)
                    {
                    case 'A': // All暂停全部运动足
                        ESP_LOGI(TAG, "[STOP] Stop all");
                        char *sendmsg_A = "Stop all/I";
                        tcp_sendData(sendmsg_A);
                        disable_one_leg(left_leg_id);
                        disable_one_leg(right_leg_id);
                        break;
                    case 'L': // Left暂停左足
                        ESP_LOGI(TAG, "[STOP] Left leg");
                        char *sendmsg_L = "Stop Left leg/I";
                        tcp_sendData(sendmsg_L);
                        disable_one_leg(left_leg_id);
                        break;
                    case 'R': // Right暂停右足
                        ESP_LOGI(TAG, "[STOP] Right leg");
                        char *sendmsg_R = "Stop right leg/I";
                        tcp_sendData(sendmsg_R);
                        disable_one_leg(left_leg_id);
                        disable_one_leg(right_leg_id);
                        break;
                    case 'C': // Force强制停止，包括停止脉冲
                        ESP_LOGI(TAG, "[STOP] Clean stop");
                        char *sendmsg_C = "Clean stop all/I";
                        tcp_sendData(sendmsg_C);
                        disable_pulse_output();
                        disable_one_leg(left_leg_id);
                        disable_one_leg(right_leg_id);
                        break;
                    default:
                        ESP_LOGW(TAG, "[STOP] Invalid command: %s", rcvd_cmd);
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "[STOP] Invalid command: %s", rcvd_cmd);
                }
            }
            else if (strcmp(buf, "CLAMP") == 0)
            {
                uint8_t clamp = 0;
                uint8_t n = sscanf(rcvd_cmd, "%*s %hhu", &clamp);
                if (n == 1)
                {
                    switch (clamp)
                    {
                    case 0:
                        ESP_LOGI(TAG, "[CLAMP] Shutdown clamp");
                        char *sendmsg_0 = "Shutdown clamp/I";
                        tcp_sendData(sendmsg_0);

                        startup_clamp(0, 3);

                        break;
                    case 1:
                        ESP_LOGI(TAG, "[CLAMP] Startup clamp");
                        char *sendmsg_1 = "Startup clamp/I";
                        tcp_sendData(sendmsg_1);
                        
                        startup_clamp(1, 3);

                        break;
                    default:
                        ESP_LOGW(TAG, "[CLAMP] Invalid command: %s", rcvd_cmd);
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "[CLAMP] Invalid command: %s", rcvd_cmd);
                }
            }
            else if (strcmp(buf, "VOLT") == 0)
            {
                float volt = 0;
                uint8_t n = sscanf(rcvd_cmd, "%*s %f", &volt);
                if (n == 1)
                {
                    voltage_control(volt);
                    ESP_LOGI(TAG, "[VOLT] Set voltage: %f", volt);
                    char sendmsg_v[64];
                    snprintf(sendmsg_v, sizeof(sendmsg_v), "Set voltage to %f/I", volt);
                    tcp_sendData(sendmsg_v);
                }
                else
                {
                    ESP_LOGW(TAG, "[VOLT] Invalid command: %s", rcvd_cmd);
                }
                // 执行VOLT命令
            }
            else if (strcmp(buf, "PULSE") == 0) // PULSE 0/1 0-100
            {
                uint8_t pulse_switch = 0;
                double pulse_duty = 1.0;
                float pulse_freq = 1;
                uint8_t n = sscanf(rcvd_cmd, "%*s %hhu %f %lf", &pulse_switch,&pulse_freq, &pulse_duty);
                if (n == 3 || n == 1)
                {
                    switch (pulse_switch)
                    {
                    case 0:
                        ESP_LOGI(TAG, "[PULSE] Shutdown pulse output");
                        char *sendmsg_0 = "Shutdown pulse output/I";
                        tcp_sendData(sendmsg_0);
                        disable_pulse_output();

                        break;
                    case 1:
                        ESP_LOGI(TAG, "[PULSE] Startup pulse output");
                        char sendmsg_p[64];
                        snprintf(sendmsg_p, sizeof(sendmsg_p), "Startup pulse output,duty:%lf%%,freq:%f/I", pulse_duty,pulse_freq);
                        tcp_sendData(sendmsg_p);
                        enable_pulse_output();
                        set_pulse_freq(pulse_freq);
                        set_pulse_duty(pulse_duty);

                        break;
                    default:
                        ESP_LOGW(TAG, "[PULSE] Invalid command: %s", rcvd_cmd);
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "[PULSE] Invalid command: %s", rcvd_cmd);
                }
            }
            else if (strcmp(buf, "VIDEO") == 0)
            {
                uint8_t video_switch = 0;
                uint8_t n = sscanf(rcvd_cmd, "%*s %hhu", &video_switch);
                if (n == 1)
                {
                    switch (video_switch)
                    {
                    case 0:
                        ESP_LOGI(TAG, "[VIDEO] Shutdown Video Transmission");
                        char *cmd_VideoOff = "VIDEO0";
                        uart_sendData(cmd_VideoOff);
                        char *cmd_VideoOff_tcp = "Shutdown Video Transmission/I";
                        tcp_sendData(cmd_VideoOff_tcp);
                        suspend_openmv();
                        break;
                    case 1:
                        resume_openmv();
                        vTaskDelay(3500 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "[VIDEO] Startup Video Transmission");
                        char *cmd_VideoOn = "VIDEO1";
                        uart_sendData(cmd_VideoOn);
                        char *cmd_VideoOn_tcp = "Startup Video Transmission/I";
                        tcp_sendData(cmd_VideoOn_tcp);
                        break;
                    default:
                        ESP_LOGW(TAG, "[VIDEO] Invalid command: %s", rcvd_cmd);
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "[VIDEO] Invalid command: %s", rcvd_cmd);
                }
            }
            else if (strcmp(buf, "SLEEP") == 0)
            {
                uint8_t sleep_cmd = 0;
                uint8_t n = sscanf(rcvd_cmd, "%*s %hhu", &sleep_cmd);
                if (n == 1)
                {
                    switch (sleep_cmd)
                    {
                    case 1:
                        ESP_LOGI(TAG, "[SLEEP] All Device Sleep");
                        char *cmd_Sleep = "All Device Sleep";
                        suspend_walk_opas();
                        suspend_clamp_opas();
                        suspend_openmv();
                        disable_pulse_output();
                        disable_one_leg(left_leg_id);
                        disable_one_leg(right_leg_id);
                        sleeping_state = true;
                        tcp_sendData(cmd_Sleep);
                        break;
                    default:
                        ESP_LOGW(TAG, "[SLEEP] Invalid command: %s", rcvd_cmd);
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "[SLEEP] Invalid command: %s", rcvd_cmd);
                }
            }
            else
            {
                // 未知命令
                ESP_LOGW(TAG, "Invalid command: %s : No Such Command", rcvd_cmd);
                char sendmsg[64];
                snprintf(sendmsg, sizeof(sendmsg), "Invalid command: %s : No Such Command/E", rcvd_cmd);
                tcp_sendData(sendmsg);
            }
        }
        else
        {
            ESP_LOGW(TAG, "[decoder] The command from %d is blocked. The currently activated device is: %d ", dev_id, actv_dev_id);
            char sendmsg[64];
            snprintf(sendmsg, sizeof(sendmsg), "%d's command is blocked. Current device: %d/W", dev_id, actv_dev_id);
            tcp_sendData(sendmsg);
        }
    }

    free(buf);
    return actv_dev_id;
}

void cmd_set_leg_pha_freq(char pos, double phase, uint32_t freq)
{
    switch (pos)
    {
    case 'L': // 左侧
        set_channel_freq(left_leg_id, freq);
        set_channel_phase(left_leg_id, phase);
        break;
    case 'R': // 右侧
        set_channel_freq(right_leg_id, freq);
        set_channel_phase(right_leg_id, phase);
        break;
    }
}
void voltage_control(float volt)
{
    if (volt < 10 || volt > 80)
    {
        ESP_LOGW(TAG, "[VoltageControl] Invalid voltage: %f V [10-80V]\n", volt);
        return;
    }
    change_voltage(volt, max_voltage);
    ESP_LOGI(TAG, "Set voltage to %f V\n", volt);
}