/**
 * @file    ailink_wdtHal.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-12
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-07-12|1.0.0  |Ai-Thinker      |创建
 */

#ifndef __AILINK_WDTHAL_H
#define __AILINK_WDTHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * @brief   初始化看门狗
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-12
 */
void AilinkWdtInit(void);

/**
 * @brief   反初始化看门狗
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-12
 */
void AilinkWdtDeinit(void);


/**
 * @brief   重装载看门狗
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-12
 */
void AilinkWdtReload(void);


#endif
