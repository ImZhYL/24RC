 #ifndef _CMD_DECODER_H_
 #define _CMD_DECODER_H_


//坐标系操作

//freertos
uint8_t decode_and_run(char *rcvd_cmd,uint8_t dev_id);
void cmd_set_leg_pha_freq(char pos, double phase, uint32_t freq);
void voltage_control(float volt);
//内部函数



#endif



