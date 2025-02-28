 #ifndef _MY_UDP_CLIENT_H_
 #define _MY_UDP_CLIENT_H_

extern char *udpSendDataPtr ;
extern int udpSendDataLength ;
extern TaskHandle_t udp_client_task;
void wifi_init(void);
void my_udp_client_task(void *pvParameters);
#endif



