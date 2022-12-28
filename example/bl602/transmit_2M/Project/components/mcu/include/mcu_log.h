/**
 * @file    mcu_log.h
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
#ifndef __MCU_LOG_H
#define __MCU_LOG_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>




#define filename(x) strrchr(x,'/')?strrchr(x,'/')+1:x


#define	MCU_OUTPUT_ERROR_PREFIX					            (1)
#define	MCU_OUTPUT_WARN_PREFIX						        (1)
#define	MCU_OUTPUT_INFO_PREFIX						        (1)
#define	MCU_OUTPUT_DEBUG_PREFIX					            (1)



/**
 * @brief   Log日志初始化
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void McuLogInit(void);


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
void McuSetLogLevel(uint8_t level);



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
uint8_t McuGetLogLevel(void);



#define MCU_ErrorLog(fmt, args...)\
				do{\
						if(McuGetLogLevel() > 0)\
						{\
								if(MCU_OUTPUT_ERROR_PREFIX)\
									printf("[MCU Error : %s, %d, %s] ",filename(__FILE__),__LINE__,__FUNCTION__);\
								printf(fmt, ##args);\
								printf("\n");\
						}\
					}while(0)

#define MCU_WarnLog(fmt, args...)\
				do{\
						if(McuGetLogLevel() > 1)\
						{\
								if(MCU_OUTPUT_WARN_PREFIX)\
									printf("[MCU Warn : %s, %d, %s] ", filename(__FILE__),__LINE__,__FUNCTION__);\
								printf(fmt, ##args);\
								printf("\n");\
						}\
					}while(0)

#define MCU_InfoLog(fmt, args...)\
				do{\
						if(McuGetLogLevel() > 3)\
						{\
								if(MCU_OUTPUT_INFO_PREFIX)\
									printf("[MCU Info : %s, %d, %s] ",  filename(__FILE__), __LINE__,__FUNCTION__);\
								printf(fmt, ##args);\
								printf("\n");\
						}\
					}while(0)


#define MCU_InfoDebug(fmt, args...)\
				do{\
						if(McuGetLogLevel() > 2)\
						{\
								if(MCU_OUTPUT_DEBUG_PREFIX)\
									printf("[MCU Debug : %s, %d, %s] ",  filename(__FILE__), __LINE__,__FUNCTION__);\
								printf(fmt, ##args);\
						}\
					}while(0)


#endif
