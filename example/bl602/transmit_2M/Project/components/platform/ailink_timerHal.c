/**
 * @file    ailink_rtctimer_hal.c
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
#include "ailink_timerHal.h"
#include <hal_hwtimer.h>
#include "ailink_utils.h"
#include "iot_log.h"
#include "ailink_flashHal.h"


#define     TIME_ZONE_DIRECTION_EAST_FLAG                   (0)
#define     TIME_ZONE_DIRECTION_WEST_FLAG                   (1)


#define DEVICE_TIME_INFO_FLASH_BLOCK            "time_info"


#define PLATFORM_TIMER_PERIOD          (100)

#define DAYS_PER_WEEK   (7)
#define BIG_MONTH       (1)     /**< 大月份，1、3、5、7、8、10、12月 */
#define LITTLE_MONTH    (2)     /**< 小月份，4、6、9、11月 */
#define FEB_MONTH       (3)     /**< 二月份 */




static AilinkRTCTimerHal_t local_time = {1970, 1, 1, 0, 0, 0, 0};
static uint32_t milliscond = 0;
static char   TimeData[30] = {0};
static uint8_t timeZone = 0;
static uint8_t zoneDirection = 0; 


static void timeCountRun(void)
{
    uint32_t days[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

    if (YEAR_LEAP(local_time.year) == 1)
    {
        days[2] = 29;
    }
    else
    {
        days[2] = 28;
    }

    milliscond += PLATFORM_TIMER_PERIOD;
    if(milliscond == 1000)
    {
        milliscond = 0;
        local_time.second++;
        if (local_time.second == 60)
        {
            local_time.second = 0;
            local_time.minute++; 
            if (local_time.minute == 60)
            {
                local_time.minute = 0;
                local_time.hour++;
                if (local_time.hour == 24)
                {
                    local_time.hour = 0;
                    local_time.day++;
                    if (local_time.day > days[local_time.mon]) 
                    {
                        local_time.mon++;
                        if (local_time.mon == 13)
                        {
                            local_time.mon = 0;
                            local_time.year++;
                        }
                    }
                }
            }
        }
    }

    if (local_time.second % 10 == 0 && milliscond == 0) 
    {
        local_time.week = cTimeToWeek(local_time.year,local_time.mon,local_time.day);
    }
}


static void hwTimerCb(void)
{
    timeCountRun();
}

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
int32_t AilinkHalRTCTimerInit(void)
{
    hal_hwtimer_init();
    hal_hwtimer_create(100, hwTimerCb, 1);

    return 0;
}

/**
 * @brief   设备反初始化RTC定时器函数
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
void AilinkHalRTCTimerDeInit(void)
{
    local_time.week = 0;
    milliscond = 0;
    local_time.second = 0;
    local_time.minute = 0;
    local_time.hour = 0;
    local_time.day = 1;
    local_time.mon = 1;
    local_time.year = 1970;
}


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
int32_t AilinkHalRTCSetTime(AilinkRTCTimerHal_t *rtc_time)
{
    if(rtc_time == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return -1;
    }
    local_time.year = rtc_time->year; 
    local_time.mon = rtc_time->mon; 
    local_time.day = rtc_time->day; 
    local_time.hour = rtc_time->hour; 
    local_time.minute = rtc_time->minute; 
    local_time.second = rtc_time->second; 
    local_time.week = cTimeToWeek(rtc_time->year,rtc_time->mon,rtc_time->day);

    return 0;
}

/**
 * @brief   获取RTC时间
 * 
 * @return  AilinkRTCTimerHal_t*  返回RTC时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
AilinkRTCTimerHal_t *AilinkHalGetRTCTime(void)
{
    return &local_time;
}

/**
 * @brief   获取RTC时间
 * 
 * @return  char*   返回RTC时间字符串数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-08
 */
char *AilinkHalGetRTCTimerData(void)
{
    memset(TimeData, 0, sizeof(TimeData));
    snprintf(TimeData, sizeof(TimeData), "%02d/%02d/%02d-%02d:%02d:%02d", local_time.year, local_time.mon, local_time.day, local_time.hour, local_time.minute, local_time.second);

    return TimeData;
}



static bool AilinkVerifyLocalTime(AilinkRTCTimerHal_t *local_time)
{
    uint8_t temp_day = 0;

    if (NULL == local_time)
    {
        IOT_ErrorLog("param err\n");
        return false;
    }

    /* 获取当月的天数 */
    temp_day = DAYS_OF_THE_MONTH(local_time->year, local_time->mon);
    if ((local_time->year >= BASE_YEAR) && 
        ((local_time->mon >= 1)  && (local_time->mon <= 12)) && 
        ((local_time->day >= 1) && (local_time->day <= temp_day)) && 
        (local_time->hour <= 23) && (local_time->minute <= 59) && 
        (local_time->second <= 59))
    {
        return true;
    }

    IOT_ErrorLog("verify err\n");
    return false;
}


/**
 * @brief   设备时间设置函数
 * 
 * @note    从服务器或者其它途径获取当前的时间，以AilinkRTCTimerHal_t结构的格式通过本接口传入给设备处理，使设备时间
 *          与当前时间同步；必须在模块初始化成功后调用本接口
 * 
 * @param[in]   time_data   特定格式的时间字符串，可以从服务器获取，例如如下格式，
 *                          示例："2022-07-21T09:04:14Z"
 * @return  
 *          true：设置成功
 *          false：设置失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
bool AilinkRTCTimerSetDevTime(const char *time_data)
{
    char temp[24] = {0};
    AilinkRTCTimerHal_t local_time = {0};
    uint8_t tmp = 0;

    if (time_data)
    {
        if ((strlen(time_data) == 20) && (time_data[10] == 'T') && (time_data[19] == 'Z'))
        {
            memset(temp, 0, sizeof(temp));
            strncpy(temp, time_data + 0, 4);
            local_time.year = atoi(temp);

            memset(temp, 0, sizeof(temp));
            strncpy(temp, time_data + 5, 2);
            local_time.mon = atoi(temp);

            memset(temp, 0, sizeof(temp));
            strncpy(temp, time_data + 8, 2);
            local_time.day = atoi(temp);

            memset(temp, 0, sizeof(temp));
            strncpy(temp, time_data + 11, 2);
            local_time.hour = atoi(temp);

            memset(temp, 0, sizeof(temp));
            strncpy(temp, time_data + 14, 2);
            local_time.minute = atoi(temp);

            memset(temp, 0, sizeof(temp));
            strncpy(temp, time_data + 17, 2);
            local_time.second = atoi(temp);

            IOT_InfoLog("the Date : %d:%d:%d \r\n", local_time.year, local_time.mon, local_time.day);
            IOT_InfoLog("the time : %d:%d:%d \r\n", local_time.hour, local_time.minute, local_time.second);

            if (AilinkVerifyLocalTime(&local_time))
            {
                if(zoneDirection == TIME_ZONE_DIRECTION_WEST_FLAG)
                {
                    local_time.day -= 1;
                    if(local_time.hour > 12)
                    {
                        local_time.hour -= timeZone;
                    }
                    else
                    {
                        tmp = local_time.hour;
                        local_time.hour = 24 - timeZone + tmp;
                    }
                    if(local_time.hour == 24)           // 当设备处于0时，获取到utc时间并根据时区进行更新本地时间时，其时间将变成24时，此时需处理成0时，且将天数累加。
                    {
                        local_time.hour = 0;
                        local_time.day += 1;
                    }
                }
                else
                {
                    local_time.hour += timeZone;
                    if(local_time.hour == 24)           // 当设备处于0时，获取到utc时间并根据时区进行更新本地时间时，其时间将变成24时，此时需处理成0时，且将天数累加。
                    {
                        local_time.hour = 0;
                        local_time.day += 1;
                    }
                }

                AilinkHalRTCSetTime(&local_time);
                return true;
            }
            else
            {
                IOT_ErrorLog("time data verify err \r\n");
                return false;
            }
        }
        else
        {
            IOT_ErrorLog("time data format is err \r\n");
        }
    }
    else
    {
        IOT_ErrorLog("time data is NULL \r\n");
    }
    return false;
}



/**
 * @brief   根据时区进行更新时间
 * 
 * @param[in]   time_zone  时区数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-21
 */
void AilinkUpdateRtcTimeWithZone(char *time_zone)
{
    char str_temp[5] = {0};
    
    AilinkRTCTimerHal_t *local_time = NULL;

    if(time_zone == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return;
    }

    local_time = AilinkHalGetRTCTime();

    memcpy(str_temp, &time_zone[3], 1);
    IOT_InfoLog("str_temp = %s \r\n", str_temp);
    if(strcmp(str_temp, "+") == 0)
    {
        memset(str_temp, 0, sizeof(str_temp));
        memcpy(str_temp, &time_zone[4], 2);
        IOT_InfoLog("str_temp = %s \r\n", str_temp);
        timeZone = atoi(str_temp);
        IOT_InfoLog("timeZone = %d \r\n", timeZone);
        zoneDirection = TIME_ZONE_DIRECTION_EAST_FLAG;
    }
    else
    {
        memset(str_temp, 0, sizeof(str_temp));
        memcpy(str_temp, &time_zone[4], 2);
        IOT_InfoLog("str_temp = %s \r\n", str_temp);
        timeZone = atoi(str_temp);
        IOT_InfoLog("timeZone = %d \r\n", timeZone);
        zoneDirection = TIME_ZONE_DIRECTION_WEST_FLAG;
    }

    AilinkHalRTCSetTime(local_time);
}




/**
 * @brief   获取当前系统的时间戳
 * 
 * @return  uint32_t  已获取到的时间戳数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-07
 */
uint32_t GetCurrentTimeStamp(void)
{
    TimeType timeData = {0};
    uint32_t  timestamp = 0;
    AilinkRTCTimerHal_t *timer = NULL;

    timer = AilinkHalGetRTCTime();
    if(timer == NULL)
    {
        IOT_ErrorLog("timer is NULL \r\n");
        return 0;
    }

    // IOT_InfoLog("the Date : %d:%d:%d \r\n", timer->year, timer->mon, timer->day);
    // IOT_InfoLog("the time : %d:%d:%d \r\n", timer->hour, timer->minute, timer->second);

    timeData.year = timer->year;
    timeData.month = timer->mon;
    timeData.day = timer->day;
    timeData.minute = timer->minute;
    timeData.second = timer->second;
    timeData.hour = timer->hour;
    timeData.cUTC = 8;

    timestamp = lTimeToStamp(timeData);
    // IOT_InfoLog("timestamp = %ld \r\n", timestamp);


    return timestamp;
}

/**
 * @brief   存储设备启动时的时间
 * 
 * @note    当设备启动后，通过UTC获取当前时间后，并将时间存储与flash上
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void AilinkSaveTimeInfo(void)
{
    AilinkRTCTimerHal_t *RtcTime = NULL;

    RtcTime = AilinkHalGetRTCTime();
    if(RtcTime == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return ;
    }

    AilinkFlashWrite(DEVICE_TIME_INFO_FLASH_BLOCK, (uint8_t *)RtcTime, sizeof(AilinkRTCTimerHal_t));
}


/**
 * @brief   清除flash存储的时间数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void AilinkClearTimeInfo(void)
{
    AilinkRTCTimerHal_t RtcTime = {0};

    AilinkFlashWrite(DEVICE_TIME_INFO_FLASH_BLOCK, (uint8_t *)&RtcTime, sizeof(AilinkRTCTimerHal_t));
}


/**
 * @brief   从flash中获取设备启动的时间
 * 
 * @param[in]   RtcTime    待获取的时间数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void AilinkLoadTimeInfo(AilinkRTCTimerHal_t *RtcTime)
{
    if(RtcTime == NULL)
    {
        IOT_ErrorLog("Param err \r\n");
        return ;
    }

    AilinkFlashRead(DEVICE_TIME_INFO_FLASH_BLOCK, (uint8_t *)RtcTime, sizeof(AilinkRTCTimerHal_t));
    IOT_InfoLog("the Date : %02d:%02d:%02d \r\n", RtcTime->year, RtcTime->mon, RtcTime->day);
    IOT_InfoLog("the time : %02d:%02d:%02d \r\n", RtcTime->hour, RtcTime->minute, RtcTime->second);

}

