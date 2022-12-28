/**
 * @file    mcu_process.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-23
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-06-23|1.0.0  |Ai-Thinker      |创建
 */
#ifndef __MCU_PROCESS_H
#define __MCU_PROCESS_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>




/**
 * @brief   重新设备发送心跳包的间隔时间
 * 
 * @param[in]   SetTimeout   设置发送心跳包的间隔时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
void  SetHeartBeatTimeout(uint32_t SetTimeout);


/**
 * @brief   重新设置向云端发送数据的超时时间
 * 
 * @param[in]   SetTimeout      超时时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-01
 */
void SetPublicDataTimeout(uint32_t SetTimeout);

/**
 * @brief   初始化MCU通信处理任务
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-25
 */
void McuProcessInit(void);


/**
 * @brief   模组与MCU通信的串口初始化，采用中断接收处理数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-11-14
 */
void McuUartInit(void);


/**
 * @brief   模组与MCU的串口反初始化，关闭中断接收数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-11-14
 */
void McuUartDeinit(void);



#endif
