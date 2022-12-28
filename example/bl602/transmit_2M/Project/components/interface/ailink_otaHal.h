/**
 * @file    ailink_otaHal.h
 * @brief   
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-06-12
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-06-12|1.0.0  |Julius          |创建
 */

#ifndef __AILINK_OTAHAL_H
#define __AILINK_OTAHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>





/**
 * @brief   ota状态回调函数
 * 
 * @param[in]   status  ota状态参数
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-16
 */
void AilinkOtaStatusCallback(uint8_t status);



/**
 * @brief   ota固件数据的输出回调函数
 * 
 * @param[in]   offset          ota固件数据偏移大小
 * @param[in]   data            ota固件数据
 * @param[in]   length          ota固件数据长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-16
 */
void AilinkOtaDownloadCallback(uint32_t offset, uint8_t *data, uint32_t length);


/**
 * @brief   MCU ota固件数据的输出回调函数
 * 
 * @param[in]   offset          ota固件数据偏移大小
 * @param[in]   data            ota固件数据
 * @param[in]   length          ota固件数据长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-16
 */
void AilinkMcuOtaDownloadCallback(uint32_t offset, uint8_t *data, uint32_t length);


/**
 * @brief   获取ota安装状态
 * 
 * @return  true        ota已安装成功
 * @return  false       ota安装失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-20
 */
bool AilinkGetOtaInstallStatus(void);


/**
 * @brief   设置ota安装状态
 * 
 * @param[in]   ota_status
 *              true: ota已安装成功
 *              false：ota未安装成功
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-20
 */
void AilinkSetOtaInstallStatus(bool ota_status);



#endif
