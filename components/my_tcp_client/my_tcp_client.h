 #ifndef _MY_TCP_CLIENT_H_
 #define _MY_TCP_CLIENT_H_

//操作
void wifi_init();
void tcp_sendData(char *SendMsg);
//内部函数
void tcp_client_task(void *pvParameters);
//void running_status(char *status);

#endif



