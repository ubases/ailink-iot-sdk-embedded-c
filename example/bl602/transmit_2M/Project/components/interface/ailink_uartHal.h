/**
 * @file    iotgo_uart.h
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

#ifndef __IOTGO_UARTHAL_H
#define __IOTGO_UARTHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



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
int32_t AilinkUart1Init(void);



int32_t AilinkUart1DeInit(void);

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
int32_t AilinkUart1Send(uint8_t *data, uint16_t data_len);


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
int32_t AilinkUart1Receive(uint8_t *data, uint16_t data_len);


#endif

