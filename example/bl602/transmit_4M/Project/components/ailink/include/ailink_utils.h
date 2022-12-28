/**
 * @file    ailink_utils.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-19
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-08-19|1.0.0  |Ai-Thinker      |创建
 */
#ifndef __AILINK_UTILS_H
#define __AILINK_UTILS_H
#include <stdint.h>



/* 闰年 */
#ifndef AILINK_LEAP_YEAR
#define AILINK_LEAP_YEAR   1
#endif // LEAP_YEAR

/* 平年 */
#ifndef AILINK_COMMON_YEAR
#define AILINK_COMMON_YEAR 0
#endif // COMMON_YEAR


#define   BASE_YEAR             (2006)


/* 判断是否为闰年 */
#define YEAR_LEAP(year) (((((year) % 4 == 0) && ((year) % 100 != 0)) || (((year) % 400 == 0) && ((year) % 3200 != 0)) || ((year) % 172800 == 0)) ? AILINK_LEAP_YEAR : AILINK_COMMON_YEAR)
/* 获取年份天数 */
#define DAYS_OF_THE_YEAR(year) (YEAR_LEAP(year) == AILINK_LEAP_YEAR ? 366 : 365)
/* 获取月份天数 */
#define DAYS_OF_THE_MONTH(year, month) ((((month) == 2) && (YEAR_LEAP(year) == AILINK_LEAP_YEAR)) ? 29 : GetMonthDays(month))



typedef struct{
    int32_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
	uint8_t week;
    int8_t cUTC;
}TimeType;



/**
 * @brief   获取星期几
 * 
 * @note    使用蔡勒公式计算星期几
 * 
 * @param[in]   iYear               指定月份
 * @param[in]   ucMonth             指定月份
 * @param[in]   ucDay               指定天数
 * @return  uint8_t                 返回星期几
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-19
 */
uint8_t cTimeToWeek(int32_t iYear, uint8_t ucMonth, uint8_t ucDay);

/**
 * @brief   获取日期和时间
 * 
 * @note    返回TimeType类型的日期和时间
 * 
 * @param[in]   lStamp              UNIX时间戳
 * @param[in]   ptypeTime           时间结构体指针
 * @param[in]   cUTC                时区（东时区为正、西时区为负）
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-19
 */
void vStampToTime(uint32_t lStamp, TimeType *ptypeTime, int8_t cUTC);

/**
 * @brief      获取以秒为单位的时间戳
 * 
 * @param[in]   typeTime        日期和时间
 * @return  uint32_t            返回时间戳
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-19
 */
uint32_t lTimeToStamp(TimeType typeTime);


/**
 * @brief   时间戳转换字符串时间
 * 
 * @param[in]   lStamp   时间戳
 * @param[in]   cUTC     时间戳时区
 * @return  char* 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-10
 */
char *pcStampToTimeStrings(uint32_t lStamp, int8_t cUTC);
/**
 * @brief     时间戳转换日期
 * 
 * @param[in]   lStamp   时间戳
 * @param[in]   cUTC     时间戳时区
 * @return  char* 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-10
 */
char *pcStampToDateStrings(uint32_t lStamp , int8_t cUTC);



/**
 * @brief   获取月份天数
 * 
 * @param[in]   month       指导月份
 * @return  uint8_t         返回指定月份的天数
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-19
 */
uint8_t GetMonthDays(uint8_t month);


#endif
