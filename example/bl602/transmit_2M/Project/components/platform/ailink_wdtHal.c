/**
 * @file    ailink_wdt.c
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
#include "ailink_wdtHal.h"
#include <hosal_wdg.h>
#include "iot_log.h"


static hosal_wdg_dev_t wdg;
static bool WdtIsInit = false;

/**
 * @brief   初始化看门狗
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-12
 */
void AilinkWdtInit(void)
{
    // return ;
    if(WdtIsInit)
    {
        IOT_ErrorLog("wdt already init, not aganst init \r\n");
        return;
    }

    /* wdg port set */
    wdg.port = 0;
    /* max timeout is 65532/16 ms */
    wdg.config.timeout = 10000;
    /* init wdg with the given settings */
    hosal_wdg_init(&wdg);
    IOT_DebugLog("wdt init \r\n");
    WdtIsInit = true;
}

/**
 * @brief   反初始化看门狗
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-12
 */
void AilinkWdtDeinit(void)
{
    // return ;
    if(!WdtIsInit)
    {
        IOT_ErrorLog("wdt already Deinit, not aganst Deinit \r\n");
        return;
    }

    hosal_wdg_finalize(&wdg);
    WdtIsInit = false;
    IOT_DebugLog("wdt Deinit \r\n");
}

/**
 * @brief   重装载看门狗
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-12
 */
void AilinkWdtReload(void)
{
    // return ;
    hosal_wdg_reload(&wdg);
}