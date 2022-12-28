/**
 * @file    iot_log.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-16
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-08-16|1.0.0  |Ai-Thinker      |创建
 */
#ifndef __IOT_LOG_H
#define __IOT_LOG_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "ailink_timerHal.h"


#define filename(x) strrchr(x,'/')?strrchr(x,'/')+1:x

#define	IOT_OUTPUT_ERROR_PREFIX					            (1)
#define	IOT_OUTPUT_WARN_PREFIX						        (1)
#define	IOT_OUTPUT_INFO_PREFIX						        (1)
#define	IOT_OUTPUT_DEBUG_PREFIX					            (1)



/**
 * @brief   Log日志初始化
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void IotLogInit(void);

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
void IotSetLogLevel(uint8_t level);


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
uint8_t IotGetLogLevel(void);



#define IOT_ErrorLog(fmt, args...)\
				do{\
						if(IotGetLogLevel() > 0)\
						{\
								if(IOT_OUTPUT_ERROR_PREFIX)\
									printf("[IOT Error[%s] : %s, %d, %s] ", AilinkHalGetRTCTimerData(),filename(__FILE__),__LINE__,__FUNCTION__);\
								printf(fmt, ##args);\
								printf("\n");\
						}\
					}while(0)

#define IOT_WarnLog(fmt, args...)\
				do{\
						if(IotGetLogLevel() > 1)\
						{\
								if(IOT_OUTPUT_WARN_PREFIX)\
									printf("[IOT Warn[%s] : %s, %d, %s] ", AilinkHalGetRTCTimerData(),filename(__FILE__),__LINE__,__FUNCTION__);\
								printf(fmt, ##args);\
								printf("\n");\
						}\
					}while(0)

#define IOT_InfoLog(fmt, args...)\
				do{\
						if(IotGetLogLevel() > 3)\
						{\
								if(IOT_OUTPUT_INFO_PREFIX)\
									printf("[IOT Info[%s] : %s, %d, %s] ",  AilinkHalGetRTCTimerData(),filename(__FILE__), __LINE__,__FUNCTION__);\
								printf(fmt, ##args);\
								printf("\n");\
						}\
					}while(0)

#define IOT_DebugLog(fmt, args...)\
				do{\
						if(IotGetLogLevel() > 2)\
						{\
								if(IOT_OUTPUT_DEBUG_PREFIX)\
									printf("[IOT Debug[%s] : %s, %d, %s] ",  AilinkHalGetRTCTimerData(),filename(__FILE__), __LINE__,__FUNCTION__);\
								printf(fmt, ##args);\
								printf("\n");\
						}\
					}while(0)


#endif
