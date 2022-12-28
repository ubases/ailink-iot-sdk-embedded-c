/**
 * @file    ailink_ota.h
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
 
#ifndef __AILINK_OTA_H
#define __AILINK_OTA_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * @brief   OTA结果状态
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-15
 */
#define AILINK_OTA_STATUS_SUCCESS   (0)  /**< OTA成功 */
#define AILINK_OTA_STATUS_FAIL      (1)  /**< OTA失败 */
#define AILINK_OTA_STATUS_TIMEOUT   (2)  /**< OTA超时 */
#define AILINK_OTA_STATUS_STRART    (3)     /**< OTA开始 */


/**
 * @brief   OTA在获取数据时，采用的tcp连接类型
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-15
 */
#define OTA_CONNECT_TYPE_HTTP    (0)  /** OTA http连接 */
#define OTA_CONNECT_TYPE_HTTPS   (1)  /** OTA https 连接 */

#define OTA_TYPE_LOCAL              (1)     /**< 模组OTA */
#define OTA_TYPE_MCU                (2)     /**< mcu OTA */


typedef void (*ailinkOtaStatusCallback)(uint8_t status);                /**< OTA结果回调函数*/
typedef void (*ailinkOtaDownloadCallback)(uint32_t offset, uint8_t *data, uint32_t length);             /**< 获取到OTA固件的固件，通过该回调传出 */

/**
 * @brief   协议信息结构
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-22
 */
typedef struct 
{
    uint8_t channel;                    /**< 升级渠道，1：云端，0：APP */
    char    pointVer[10];               /**< 指定目标版本号，即指定某一个版本号可升级 */
    char    baseVer[10];                /**< 固件最低兼容版本号,例如：最低兼容版本号是2.0.1，而2.0.5该版本可升级，而1.5.0该版本不可升级 */
    char    OtaVer[10];                 /**< OTA固件待升级的版本号 */
    char    mcuBaseVer[10];             /**< 指定MCU最低兼容版本号，例如：最低兼容版本号是2.0.1，而2.0.5该版本可升级，而1.5.0该版本不可升级 */
    char    otaType[20];                /**< OTA包下载类型，如：module_ota_all为整包下载*/
    char    download_url[400];          /**< OTA包下载链接 */
    char    PubId[33];                  /**< 云端下发的发布id */
    
}ailinkOtaProInfo;

/**
 * @brief   OTA包头信息结构
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-22
 */
typedef struct 
{
    char device_fw_version[17];             /**< OTA包固件版本号 */
    char device_productid[64];              /**< 设备的产品ID */
    char ota_type;                          /**< OTA类型，1：模组OTA固件包，2：MCU OTA固件包 */
    char ota_flag[8];                       /**< OTA包头标志，即0xaa 0x55 0xff 0x44 0x88 0xaa */
    uint16_t  head_offset;                  /**< OTA包头偏移长度 */
    uint32_t  ota_length;                   /**< OTA包数据长度 */
}ailink_ota_head_info;


/**
 * @brief   OTA 配置信息结构体
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-22
 */
typedef struct 
{
    uint32_t ota_timeout;                       /**< OTA超时时间 */
    ailink_ota_head_info    head_info;          /**< OTA包头信息结构 */
    ailinkOtaProInfo        ProInfo;            /**< OTA协议信息结构 */
    ailinkOtaStatusCallback OtaStatusCallback;          /**< OTA升级状态回调函数 */
    ailinkOtaDownloadCallback OtaDownloadCallback;      /**< OTA包数据透传回调函数 */
    ailinkOtaDownloadCallback McuOtaDownloadCallback;   /**< MCU OTA包数据透传回调函数 */

}ailink_ota_config;


/**
 * @brief   ota功能初始化，配置OTA各个参数
 * 
 * @param[in]   config          待配置OTA的参数
 * @return  int32_t         
 *          0：ota初始化成功
 *          -1：ota初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-16
 */
int32_t AilinkOtaInit(ailink_ota_config *config);

/**
 * @brief   ota功能反初始化，但ota升级结束后，可调用该函数，释放ota过程中占用的资源
 * 
 * @return  int32_t 
 *          0：反初始化成功
 *          -1：反初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-16
 */
int32_t AilinkOtaDeInit(void);


/**
 * @brief   获取OTA启动状态
 * 
 * @return  true        设备的OTA功能已启动
 * @return  false       设备的OTA功能未启动
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-31
 */
bool AilinkOtaIsStart(void);



#endif
