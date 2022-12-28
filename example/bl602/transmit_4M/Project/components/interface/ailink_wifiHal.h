/**
 * @file    ailink_wifiHal.h
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
#ifndef __AILINK_WIFIHAL_H
#define __AILINK_WIFIHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>




#define AILINK_WIFI_STATE_STA      (1)
#define AILINK_WIFI_STATE_AP       (2)

#define AILINK_WIFI_ON_INIT_DONE             1           /**< wifi协议栈已初始化完成 */
#define AILINK_WIFI_ON_SCAN_DONE             2           /**< WiFi扫描完成 */
#define AILINK_WIFI_ON_DISCONNECT            3           /**< WiFi已断开 */
#define AILINK_WIFI_CMD_RECONNECT            4           /**< wifi正在重连 */
#define AILINK_WIFI_ON_CONNECTED             5           /**< wif 已连接成功 */
#define AILINK_WIFI_ON_GOT_IP                6           /**< IP已获取 */
#define AILINK_WIFI_ON_AP_STARTED            7           /**< AP模式已开启 */
#define AILINK_WIFI_ON_AP_STOPPED            8           /**< AP模式已停止 */
#define AILINK_WIFI_ON_AP_STA_ADD            9           /**< AP热点已被连接 */
#define AILINK_WIFI_ON_AP_STA_DEL            10         /**< 连接AP热点的设备已断开 */



typedef struct 
{
    char ssid[33];
    char passwd[65];
    char ip[16];
    char mask[16];
    char gw[16];
    char bssid[13];

}ailinkWifiInfo_t;



typedef void (*ailinkwifiConnetStatus)(int32_t err, ailinkWifiInfo_t *wifi_info);




/**
 * @brief   wifi协议栈初始化
 * 
 * @return  int32_t 
 *          0：初始化成功
 *          -1：初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkWifiInit(void);

/**
 * @brief   WiFi协议栈反初始化
 * 
 * @return  int32_t 
 *          0：初始化成功
 *          -1：反初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkWifiDeinit(void);


/**
 * @brief   开启WiFi的STA模式
 * 
 * @param[in]   wifiConnectStatusCb   当WiFi连接后，可将WiFi连接状态事件和信息通该回调上传给应用层。
 * @return  int32_t 
 *          0：成功开启STA模式
 *          -1：开启STA模式失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
int32_t ailinkWifiStaInit(ailinkwifiConnetStatus wifiConnectStatusCb);

/**
 * @brief   关闭WiFi的STA模式
 * 
 * @return  int32_t 
 *          0：成功关闭STA模式
 *          -1：关闭STA模式失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkWifiStaDeint(void);

/**
 * @brief   获取WiFi协议栈初始化状态
 * 
 * @return  true        协议栈初始化成功
 * @return  false       协议栈初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
bool ailinkGetWifiInitStatus(void);



/**
 * @brief   获取wifi的STA模式初始化状态
 * 
 * @return  true    WiFi的STA已初始化
 * @return  false   WiFi的STA未初始化
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-25
 */
bool ailinkGetWifiStaInitStatus(void);

/**
 * @brief   WiFi连接
 * 
 * @param[in]   ssid    [in] 路由的ssid
 * @param[in]   pwd     [in] 路由的pwd
 * @return  int32_t 
 *          0:WiFi连接成功
 *          -1：WiFi连接失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkWifiConnect(char *ssid, char *pwd);

/**
 * @brief   WiFi断开连接
 * 
 * @return  int32_t 
 *          0：WiFi断开成功
 *          -1：WiFi断开失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkWifiDisconnect(void);

/**
 * @brief   开启AP模式
 * 
 * @param[in]   ssid                    AP模式下的ssid
 * @param[in]   pwd                     AP模式虾的pwd
 * @param[in]   wifiConnectStatus       可通过该回调上传WiFi事件和信息
 * @return  int32_t 
 *          0：AP模式开启成功
 *          -1：AP模式开启失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
int32_t ailinkWifiSoftApInit(char *ssid, char *pwd, ailinkwifiConnetStatus wifiConnectStatus);

/**
 * @brief   关闭AP模式
 * 
 * @return  int32_t 
 *          0：AP开启成功
 *          -1：AP开启失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkWifiSoftApDeinit(void);


/**
 * @brief   获取WiFi STA模式的连接状态
 * 
 * @return  true  STA已成功连接路由
 * @return  false  STA已断开连接路由或是未连接路由
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
bool  ailinkWifiStaConnectStatus(void);

/**
 * @brief   获取WiFi AP模式的连接状态
 * 
 * @return  true  AP已被连接成功
 * @return  false  AP连接已断开或未被连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
bool  ailinkWifiAPConnectStatus(void);


/**
 * @brief   获取wifi的信号强度
 * 
 * @return  int32_t   返回设备连接路由后，路由信号强度值
            0：表示设备未连接路由
            非0：表示设备已连接路由，该返回值未信号强度值
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
int32_t ailinkWifiGetRssi(void);

/**
 * @brief   对芯片进行软件复位
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-30
 */
void ailinkReboot(void);

/**
 * @brief   获取芯片上的mac地址
 * 
 * @param[in]   str_mac         接收mac的buf
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-31
 */
void ailinkGetMac(char *str_mac, uint16_t str_mac_len);

/**
 * @brief   获取剩余的内存大小
 * 
 * @return  uint32_t  返回剩余的内存大小
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-31
 */
uint32_t ailinkGetFreeHeapSize(void);

/**
 * @brief   获取WiFi连接状态码
 * 
 * @return  int32_t 返回WiFi连接的状态码
 *  WIFI_STATE_UNKNOWN                              = 0x00,
 *  WIFI_STATE_IDLE                                 = 0x01,
 *  WIFI_STATE_CONNECTING                           = 0x02,
 *  WIFI_STATE_CONNECTED_IP_GETTING                 = 0x03,
 *  WIFI_STATE_CONNECTED_IP_GOT                     = 0x04,
 *  WIFI_STATE_DISCONNECT                           = 0x05,
 *  WIFI_STATE_WITH_AP_IDLE                         = 0x11,
 *  WIFI_STATE_WITH_AP_CONNECTING                   = 0x12,
 *  WIFI_STATE_WITH_AP_CONNECTED_IP_GETTING         = 0x13,
 *  WIFI_STATE_WITH_AP_CONNECTED_IP_GOT             = 0x14,
 *  WIFI_STATE_WITH_AP_DISCONNECT                   = 0x15,
 *  WIFI_STATE_IFDOWN                               = 0x06,
 *  WIFI_STATE_SNIFFER                              = 0x07,
 *  WIFI_STATE_PSK_ERROR                            = 0x08,
 *  WIFI_STATE_NO_AP_FOUND                          = 0x09,
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-15
 */
int32_t ailinkWifiGetConnectStateCode(void);

/**
 * @brief   清除缓存WiFi连接的状态码
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-15
 */
void ailinkWifiCleanStateCode(void);




#endif
