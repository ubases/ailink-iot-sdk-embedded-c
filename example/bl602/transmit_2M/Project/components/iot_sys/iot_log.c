/**
 * @file    ailink_log.c
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-10-17|1.0.0  |Ai-Thinker      |创建
 */
#include "iot_log.h"
#include "ailink_flashHal.h"



#define DEVICE_LOG_INFO_FLASH_BLOCK                     "IotLogInfo"


static uint8_t LogLevel = 4;



/**
 * @brief   Log日志初始化
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void IotLogInit(void)
{
    AilinkFlashRead(DEVICE_LOG_INFO_FLASH_BLOCK, (uint8_t *)&LogLevel, sizeof(LogLevel));
    if(LogLevel == 0)
    {
        LogLevel = 1;
    }
    printf("LogLevel = %d \r\n", LogLevel);
}


/**
 * @brief   设置日志输出等级
 * 
 * @param[in]   level
 *              0：关闭日志输出
 *              1：只输出错误日志
 *              2：输出错误日志和警告日志
 *              3：输出错误日志、警告日志和调试日志
 *              4：输出全部日志信息
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void IotSetLogLevel(uint8_t level)
{
    LogLevel = level;
    AilinkFlashWrite(DEVICE_LOG_INFO_FLASH_BLOCK, (uint8_t *)&LogLevel, sizeof(LogLevel));
}





/**
 * @brief   获取日志输出等级
 * 
 * @return  uint8_t 
 *              0：关闭日志输出
 *              1：只输出错误日志
 *              2：输出错误日志和警告日志
 *              3：输出错误日志、警告日志和调试日志
 *              4：输出全部日志信息
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
uint8_t IotGetLogLevel(void)
{
    return LogLevel;
}
















