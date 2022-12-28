/**
 * @file    ailink_timerHal.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-06-08|1.0.0  |Ai-Thinker     |创建
 */
#ifndef __AILINK_TIMERHAL_H
#define __AILINK_TIMERHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * 设备时间结构体
 */
typedef struct
{
    uint16_t year;              /**< 年 */
    uint8_t mon;                /**< 月 */
    uint8_t day;                /**< 日 */
    uint8_t hour;               /**< 时 */
    uint8_t minute;             /**< 分 */
    uint8_t second;             /**< 秒 */
    uint8_t week;               /**< 星期日到星期六，值为0，1，2，3，4，5，6 */
}AilinkRTCTimerHal_t;


/**
 * @brief   设备适配RTC定时器初始化函数
 * 
 * @return  int32_t 
 *          0：初始化成功
 *          -1：初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
int32_t AilinkHalRTCTimerInit(void);

/**
 * @brief   设备反初始化RTC定时器函数
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
void AilinkHalRTCTimerDeInit(void);


/**
 * @brief   更新RTC时间
 * 
 * @param[in]   rtc_time     待更新的RTC时间
 * @return  int32_t 
 *          0：更新时间成功
 *          -1：更新时间失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
int32_t AilinkHalRTCSetTime(AilinkRTCTimerHal_t *rtc_time);

/**
 * @brief   获取RTC时间
 * 
 * @return  AilinkRTCTimerHal_t*  返回RTC时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
AilinkRTCTimerHal_t *AilinkHalGetRTCTime(void);


/**
 * @brief   获取RTC时间
 * 
 * @return  char*   返回RTC时间字符串数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-08
 */
char *AilinkHalGetRTCTimerData(void);


/**
 * @brief   设备时间设置函数
 * 
 * @note    从服务器或者其它途径获取当前的时间，以AilinkRTCTimerHal_t结构的格式通过本接口传入给设备处理，使设备时间
 *          与当前时间同步；必须在模块初始化成功后调用本接口
 * 
 * @param[in]   time_data   特定格式的时间字符串，可以从服务器获取，例如如下格式，
 *                          示例："2022-07-21T09:04:14Z"
 * @return  int32_t 
 *          0：设置成功
 *          -1：设置失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
bool AilinkRTCTimerSetDevTime(const char *time_data);

/**
 * @brief   根据时区进行更新时间
 * 
 * @param[in]   time_zone  时区数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-21
 */
void AilinkUpdateRtcTimeWithZone(char *time_zone);



/**
 * @brief   获取当前系统的时间戳
 * 
 * @return  uint32_t  已获取到的时间戳数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-07
 */
uint32_t GetCurrentTimeStamp(void);

/**
 * @brief   存储设备启动时的时间
 * 
 * @note    当设备启动后，通过UTC获取当前时间后，并将时间存储与flash上
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void AilinkSaveTimeInfo(void);

/**
 * @brief   清除flash存储的时间数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void AilinkClearTimeInfo(void);


/**
 * @brief   从flash中获取设备启动的时间
 * 
 * @param[in]   RtcTime    待获取的时间数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void AilinkLoadTimeInfo(AilinkRTCTimerHal_t *RtcTime);


#endif
