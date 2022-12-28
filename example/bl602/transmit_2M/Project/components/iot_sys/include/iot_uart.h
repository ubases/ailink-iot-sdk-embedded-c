/**
 * @file    iot_uart.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-05
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-12-05|1.0.0  |Ai-Thinker      |创建
 */
#ifndef __IOT_UART_H
#define __IOT_UART_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * @brief   串口参数配置
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-05
 */
typedef struct 
{
    uint8_t uart_rx_pin;
    uint8_t uart_tx_pin;
    uint32_t uart_bound;
}IotUartParams;

/**
 * @brief   串口初始化配置
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-05
 */
typedef struct
{
    IotUartParams params;

    void (*IotUartDataRecvCb)(uint8_t data);
}IotUartConfig;




/**
 * @brief   应用层串口初始化，配置和使用串口1
 * 
 * @param[in]   config   配置参数
 *              串口rx引脚配置
 *              串口tx引脚配置
 *              串口波特率配置
 *              串口接收回调配置
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-05
 */
void IotUartInit(IotUartConfig *config);


/**
 * @brief   串口1反初始化
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-05
 */
void IotUartDeinit(void);


/**
 * @brief   串口1发送函数
 * 
 * @param[in]   buff        待发送的数据buff
 * @param[in]   buff_len    待发送数据的长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-05
 */
void IotUartSend(uint8_t *buff, uint16_t buff_len);



#endif

