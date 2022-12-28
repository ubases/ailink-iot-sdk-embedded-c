/**
 * @file    iotgo_uart.c
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-19
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-19|1.0.0  |Ai-Thinker     |创建
 */

#include "ailink_uartHal.h"
#include <hosal_uart.h>
#include "bl_uart.h"
#include "vfs.h"
#include "ailink_osHal.h"
#include "iot_log.h"

/*Init UART In the first place*/
HOSAL_UART_DEV_DECL(iotgo_uart1_stdio, 1, 16, 7, 115200);


#define BUF_SIZE               (1024)
#define IOTGO_UART_NUM          1






/**
 * @brief   串口初始化
 * 
 * @return  int32_t 
 *          0：初始化成功
 *          -1：初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-23
 */
int32_t AilinkUart1Init(void)
{

    /*Init UART In the first place*/
    bl_uart_init(IOTGO_UART_NUM, 16, 7, 0, 0, 9600);

     return 0;
}


int32_t AilinkUart1DeInit(void)
{
    // hosal_uart_finalize(&iotgo_uart1_stdio);

    return 0;
}



/**
 * @brief   串口发送数据
 * 
 * @param[in]   data            待发送的数据
 * @param[in]   data_len        待发送数据的长度
 * @return  int32_t 
 *          大于0:数据发送成功
 *          其他：数据发送失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-23
 */
int32_t AilinkUart1Send(uint8_t *data, uint16_t data_len)
{
    if(data == NULL)
    {
        return -1;
    }
    uint16_t cnt = 0;

    // printf("data_len = %d\r\n", data_len);
    while (cnt < data_len) 
    {
        bl_uart_data_send(IOTGO_UART_NUM, data[cnt]);
        // printf("0x%02x ", data[cnt]);
        cnt++;
    }
    // printf("\r\n");

    return cnt;
}

/**
 * @brief   串口接收
 * 
 * @param[in]   data            串口接收数据的buf
 * @param[in]   data_len        串口接收数据buf的最大空间
 * @return  int32_t 
 *          大于0：串口接收到数据的大小
 *          其他：未接收到数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-23
 */
int32_t AilinkUart1Receive(uint8_t *data, uint16_t data_len)
{
    uint16_t count = 0;
    int32_t ret = 0;

    if(data == NULL)
    {
        return -1;
    }
    
    // while (count < data_len)
    // {
    //     data[count] = bl_uart_data_recv(IOTGO_UART_NUM);
    //     if(data[count] < 0)
    //     {
    //         break;
    //     }
    //     count++;
    // }
    count = hosal_uart_receive(&iotgo_uart1_stdio, data, data_len);
    
    return count;
}




