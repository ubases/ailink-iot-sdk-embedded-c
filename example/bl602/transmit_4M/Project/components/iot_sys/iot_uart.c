/**
 * @file    iot_uart.c
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
#include "iot_uart.h"
#include <bl602_glb.h>
#include <bl_uart.h>
#include "iot_log.h"


#define AT_UART_NUM 1

#define AT_UART_TX_PIN                  16
#define AT_UART_RX_PIN                  7
#define AT_UART_BOUND                   9600


static IotUartConfig uart_config = {0};
static bool uartIsInit = false;


static void __uart_rx_irq(void *p_arg);
static void __uart_tx_irq(void *p_arg);

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
void IotUartInit(IotUartConfig *config)
{
    if(config)
    {
        memcpy(&uart_config, config, sizeof(IotUartConfig));

        IOT_DebugLog("uart rx pin is %d \r\n", uart_config.params.uart_rx_pin);
        IOT_DebugLog("uart tx pin is %d \r\n", uart_config.params.uart_tx_pin);
        IOT_DebugLog("uart bound is %d \r\n", uart_config.params.uart_bound);

        bl_uart_init(AT_UART_NUM, uart_config.params.uart_tx_pin, uart_config.params.uart_rx_pin, 0, 0, uart_config.params.uart_bound);
        bl_uart_int_tx_notify_register(AT_UART_NUM, __uart_tx_irq, NULL);
        bl_uart_int_rx_notify_register(AT_UART_NUM, __uart_rx_irq, NULL);
        bl_uart_int_enable(AT_UART_NUM);
        bl_uart_int_tx_disable(AT_UART_NUM);
    }
    else
    {
        bl_uart_init(AT_UART_NUM, AT_UART_TX_PIN, AT_UART_RX_PIN, 0, 0, AT_UART_BOUND);
    }

    uartIsInit = true;
}

/**
 * @brief   串口1反初始化
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-05
 */
void IotUartDeinit(void)
{
    memset(&uart_config, 0, sizeof(uart_config));
    bl_uart_int_rx_notify_unregister(AT_UART_NUM, __uart_tx_irq, NULL);
    bl_uart_int_tx_notify_unregister(AT_UART_NUM, __uart_rx_irq, NULL);
    bl_uart_int_disable(AT_UART_NUM);

    uartIsInit = false;
}

/**
 * @brief   串口1发送函数
 * 
 * @param[in]   buff        待发送的数据buff
 * @param[in]   buff_len    待发送数据的长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-05
 */
void IotUartSend(uint8_t *buff, uint16_t buff_len)
{
    uint16_t cnt = 0;

    while (cnt < buff_len) 
    {
        bl_uart_data_send(AT_UART_NUM, ((uint8_t*)buff)[cnt]);
        cnt++;
    }
}



static void __uart_tx_irq(void *p_arg)
{
    bl_uart_int_tx_disable(AT_UART_NUM);
}





static void __uart_rx_irq(void *p_arg)
{
    int ch;
    uint8_t value = 0;

    while ((ch = bl_uart_data_recv(AT_UART_NUM)) >= 0) 
    {
        value = (uint8_t)ch;
        if(uart_config.IotUartDataRecvCb && uartIsInit)
        {
            uart_config.IotUartDataRecvCb(value);
        }
    }
}