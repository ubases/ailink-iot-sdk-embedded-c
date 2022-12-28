/**
 * @file    ailink_softtimer.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-18|1.0.0  |Ai-Thinker     |创建
 */
#ifndef __AILINK_SOFTTIMER_H
#define __AILINK_SOFTTIMER_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* 最多可创建定时器个数 */
#define  AILINK_TIMER_MAX_NUM               20

/* 删除所有创建的定时器 */
#define  AILINK_TIMER_DELETE_ALL            0xffffffff

/**
 * @brief   AilinkTimerTyep
 * 
 * @note    定时器类型
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
#define AILINK_SINGLE_TIMER                 (0)
#define AILINK_REPEAT_TIMER                 (1)


/**
 * @brief   AilinkTimerStatus
 * 
 * @note    定时器状态
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
#define     AILINK_TIMER_CANCEL             (0)
#define     AILINK_TIMER_EXIST              (1)
#define     AILINK_TIMER_STOP               (2)
#define     AILINK_TIMER_RUNNING            (3)
#define     AILINK_TIMER_TIMEOUT            (4)


/**
 * @brief   定义标准类型定义，主要用于声明定时器句柄
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
typedef  int32_t  AILINK_TIMER;

/**
 * @brief   软件定时器初始化
 * 
 * @return  true        已初始化成功
 * @return  false       初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
bool AilinkTimerInit(void);

/**
 * @brief   新增一个软件定时器
 * 
 * @param[in]   timertype           软件定时器类型，具体类型可参考AilinkTimerTyep
 * @return  AILINK_TIMER            返回定时器句柄
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
AILINK_TIMER AilinkTimerAdd(int32_t timertype);

/**
 * @brief   设置定时器的超时时间，启动该定时器
 * 
 * @param[in]   timer_fd    已创建定时器的句柄
 * @param[in]   ms          定时器超时时间，注意：该值不能小于或等于0，若是小于0或等于0，定时器将启动失败
 * @return  int32_t     返回启动情况
 *          0：启动成功
 *          -1：启动失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
int32_t AilinkTimerStart(AILINK_TIMER timer_fd, uint32_t ms);

/**
 * @brief   停止该定时器，会将该定时器的超时时间清0，不再检查该定时器
 * 
 * @param[in]   timer_fd   已创建定时器的句柄
 * @return  int32_t         返回定时器停止情况 
 *          0：定时器停止成功
 *          -1：定时器停止失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
int32_t AilinkTimerStop(AILINK_TIMER timer_fd);

/**
 * @brief   删除该定时器，当定时器被删除后，该定时器信息将会从链表中删除。便不能再启动该定时器了。
 * 
 * @note    注意：当该定时器删除后，定时器将被赋值为-1，重新进行启动或是检查，将返回无启动失败或是无该定时器。
 * 
 * @param[in]   timer_fd        定时器句柄
 * @return  int32_t 
 *          0：删除成功
 *          -1：删除失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
int32_t AilinkTimerDel(AILINK_TIMER timer_fd);

/**
 * @brief   检查指定定时器的超时情况
 * 
 * @param[in]   timer_fd        已创建定时器的句柄
 * @return  int32_t             返回定时器运行情况，具体返回值可参考AilinkTimerStatus
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-18
 */
int32_t AilinkCheckTimerTimeout(AILINK_TIMER timer_fd);



#endif

