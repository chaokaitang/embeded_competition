#ifndef UART_H
#define UART_H
#define UART_BUFF_SIZE 4
#define U_SLEEP_TIME 100000
#define IOT_I2C_IDX_BAUDRATE (400 * 1000)
#define CW2015_I2C_IDX 0
#define IOT_PWM_PORT_PWM3 3
#define CW2015_READ_ADDR (0xC5)
#define CW2015_WRITE_ADDR (0xC4)
#define WRITELEN 2
#define CW2015_HIGHT_REGISTER 0x02
#define CW2015_LOW_REGISTER 0x03
#define CW2015_WAKE_REGISTER 0x0A
#define DELYA_US2000 20

#define Recieve_node 0xab
#define Sensor_node_1 0xfa
#define Sensor_node_2 0xfb
#define Sensor_node_3 0xfc
void Uart1GpioInit(void);
void Uart1Config(void);
void Uart1_init(void);
#endif 
