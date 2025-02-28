#ifndef _UART_CONTROLLER_H
#define _UART_CONTROLLER_H


//freertos
void uart_rx_task(void *arg);
//
void uart_controller_init(void);
int uart_sendData(char* data);



#endif



