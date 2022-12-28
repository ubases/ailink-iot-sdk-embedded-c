/**
 * @file    ailink_profile.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-12|1.0.0  |Ai-Thinker     |创建
 */
#ifndef __AILINK_PROFILE_H
#define __AILINK_PROFILE_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "cJSON.h"


/* ailink IOT协议版本号 */
#define DEVICE_AILINK_IOT_VERSION           ("1.0.0")


/* 配网类型 */
#define         AILINK_PAIR_AP_BLE_TYPE                     0       /**< AP与BLE共存配网 */
#define         AILINK_PAIR_AP_TYPE                         1       /**< AP配网 */
#define         AILINK_PAIR_BLE_TYPE                        2       /**< BLE配网 */


/* ailink IOT协议栈回调事件 */
#define         AILINK_AP_BLE_EVENT                         1               /**< AP与BLE共存配网启动事件 */
#define         AILINK_AP_EVENT                             2               /**< AP配网启动事件 */
#define         AILINK_BLE_EVENT                            3               /**< BLE配网启动事件 */
#define         AILINK_WIFI_DISCONNECT_EVENT                4               /**< wifi断开事件 */
#define         AILINK_WIFI_CONNECT_EVENT                   5               /**< WiFi已连接事件 */
#define         AILINK_CLOUD_DISCONNECT_EVENT               6               /**< 网络断开事件 */
#define         AILINK_CLOUD_CONNECT_EVENT                  7               /**< 网络已连接事件 */
#define         AILINK_CLOUD_QUERY_EVENT                    8               /**< 网络请求上报所有物模型数据 */
#define         AILINK_DEVICE_RESTORE_EVENT                 9               /**< 网络下发恢复出厂指令事件 */      
#define         AILINK_DEVICE_REBOOT_EVENT                  10              /**< 网络下发设备重启指令事件 */



/**
 * @brief   待配置的配网信息结构
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-11
 */
typedef struct 
{
    char pair_ap_ssid[33];                  /**< AP热点的ssid */
    char pair_ap_passwd[65];                /**< AP热点的passwd */
    char pair_ble_adv_name[32];             /**< BLE广播名 */

}profile_pair_info;

/**
 * @brief   局域网待配置的信息结构
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-11
 */
typedef struct 
{
    char mdns_service_type[24];             /**< 局域网功能上的MDNS服务类型 */
    char mdns_host_name[65];                /**< 局域网功能上的MDNS主机名 */
    uint16_t  lan_port;                     /**< 局域网功能上的UDP服务端口号 */

}profile_lan_info;

/**
 * @brief   设备连接上路由后其网络信息
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-11
 */
typedef struct 
{
    char ip[16];
    char mask[16];
    char gw[16];
    char bssid[13];
    char ssid[33];

}profile_wifi_info;



/**
 * @brief   ailink IOT协议栈配置信息结构
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-15
 */
typedef struct
{
    char user_name[65];                         /**< MQTT用户名 */
    char user_passwd[65];                       /**< MQTT用户密码 */
    char deviceid[65];                          /**< 设备ID */
    char productid[65];                         /**< 产品ID */
    char fw_version[10];                        /**< 模组固件版本号 */
    char mcu_version[10];                       /**< MCU固件版本号 */
    char device_type[10];                       /**< 设备类型，如博流芯片为bl602 */
    uint8_t pair_type;                          /**< 配置配网类型，如：AP和BLE共存配网为AILINK_PAIR_AP_BLE_TYPE */
    profile_pair_info pair_info;                /**< 配网配置信息 */
    profile_lan_info  lan_info;                 /**< 局域网配置信息 */

}profile_info_t;

/**
 * @brief   ailink IOT协议栈回调函数
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-15
 */
typedef struct 
{
    /**
     * @brief   接收设备控制的json字符串数据回调函数
     * 
     * @param[in]   data        json字符串事件
     * @param[in]   mid         消息ID数据
     * @param[in]   ts          时间戳
     * 
     * @author  Ai-Thinker (zhuolm@tech-now.com)
     * @date    2022-08-15
     */
    void (*ailinkRecvControlInfoCb)(char *data, char *mid, uint32_t ts);
    /**
     * @brief   接收设备OTA的json字符串数据回调函数
     * 
     * @param[in]   data        json字符串事件
     * @param[in]   mid         消息ID数据
     * @param[in]   ts          时间戳
     * 
     * @author  Ai-Thinker (zhuolm@tech-now.com)
     * @date    2022-08-15
     */
    void (*ailinkRecvOTAInfoCb)(char *data, char *mid, uint32_t ts);
    /**
     * @brief   接收ailink IOT协议栈中的事件回调函数
     * 
     * @param[in]   event        回调事件
     * 
     * @author  Ai-Thinker (zhuolm@tech-now.com)
     * @date    2022-08-15
     */
    void (*ailinkEventCb)(uint8_t event);

}profile_event_t;




/**
 * @brief   初始化Ailink IOT协议栈
 * 
 * @param[in]   info        待配置的信息
 * @param[in]   eventCb     事件回调函数
 * @return  int32_t 
 *              0：初始化成功
 *              -1：初始化失败
 *              -2：重复初始化
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-11
 */
int32_t AilinkInit(profile_info_t *info, profile_event_t *eventCb);


/**
 * @brief   AiLink IOT协议栈反初始化
 * 
 * @note    该函数主要是处理释放AiLink占用的资源以及关闭AiLink所有功能
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-25
 */
void AilinkDeInit(void);

/**
 * @brief   进入配网函数
 * 
 * @note    当调用该函数时，设备便进入配网模式（配网模式由初始化AilinkInit设置决定）
 *          1. 当设备已配过网时，通过该函数先行关闭STA模式以及反初始化广域网和局域网模块，然后进入配网模式
 *          2. 当设备未配网过时，通过该函数将直接进入配网模式
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-12
 */
void AilinkEnterPair(void);


/**
 * @brief   退出配网模式
 * 
 * @note    当调用该函数时有以下两种情况
 *          1. 设备已配过网，当设备重新进入配网模式时，可通过该函数退出配网，然后重新连接之前配网过路由。
 *             若是重新连接路由超时时（3分钟超时），设备将会重新进入配网模式。
 *          2. 设备未配过网，当设备已进入配网模式时，通过该函数退出配网后，设备仍会进入配网模式。
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-12
 */
void AilinkExitPair(void);


/**
 * @brief   获取profile信息
 * 
 * @return  profile_info_t*  返回获取profile存储的信息
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-28
 */
profile_info_t *AilinkGetProfileInfo(void);


/**
 * @brief   获取WiFi信息
 * 
 * @return  profile_wifi_info*      返回已连接的路由信息 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-07-05
 */
profile_wifi_info *AilinkGetWifiInfo(void);




/**
 * @brief   向云端回应控制ack
 * 
 * @param[in]   code        错误码
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-31
 */
void AilinkPublicControlAck(uint8_t code, char *msgMid);


/**
 * @brief   MCU主动上报设备所有的属性
 * 
 * @param[in]   json_device
 * @return  int32_t 
 *          0：上报成功
 *          -1：参数为空
 *          -2：设备未上线
 *          -3：json创建失败
 *          -4：json协议打包失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-26
 */
int32_t AilinkPublicReportAttributes(cJSON  *json_device);


/**
 * @brief   OTA升级进度上报
 * 
 * @param[in]   code         错误码状态
 *              0：表示OTA成功
 *              1：表示下载失败
 *              2：表示安装失败
 *              3：表示协议数据错误
 *              4：表示OTA数据包错误
 *              
 * @param[in]   otaStatus        ota上报状态
 *              Downloading：表示下载中
 *              Installing：表示固件正在安装中
 * @param[in]   OtaVer        ota待升级的版本号
 * @param[in]   progress         ota上报进度
 * @param[in]   PubId            ota信息的发布ID
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-18
 */
void AilinkPublicReportProgress(uint8_t code, char *otaStatus, char *OtaVer, char *PubId, uint32_t progress);


/**
 * @brief   断开云端的MQTT和TCP连接
 * 
 * @note    可在设备复位前或是主动断开云端前发送该函数，设备便可断开了云端连接
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-18
 */
void AilinkSeverDisconnect(void);


#endif
