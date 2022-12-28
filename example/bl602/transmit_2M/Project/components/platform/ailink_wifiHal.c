/**
 * @file    ailink_wifi.c
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-13|1.0.0  |Ai-Thinker     |创建
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ailink_wifiHal.h"
#include "iot_log.h"
#include "blog.h"
#include <aos/yloop.h>
#include <aos/kernel.h>
#include <event_device.h>
#include <bl_wifi.h>
#include <hal_wifi.h>
#include <wifi_mgmr_ext.h>
#include <bl_pm.h>
#include <hal_sys.h>


#define WIFI_AP_CHANNEL                     5               /**< 设置AP热点的信道 */




ailinkwifiConnetStatus ailinkwifiConnectStatus;



static wifi_conf_t conf =
{
    .country_code = "CN"
};
static wifi_interface_t g_wifi_sta_interface = NULL;
static wifi_interface_t g_wifi_ap_interface = NULL;
static bool wifiStaconnectStatus = false;               /**< true: WiFi已连接路由，false:WiFi未连接路由 */
static bool wifiWificonnStatus = false;
static bool wifiAPConnectStatus = false;
static bool wifi_is_init = false;
static bool wifi_sta_init = false;              

ailinkWifiInfo_t wifi_info = {0};

wifi_mgmr_sta_connect_ind_stat_info_t info;
ip4_addr_t ip, gw, mask;

extern int wifi_mgmr_sta_autoconnect_disable(void);
extern int wifi_mgmr_state_get(int *state);
extern int wifi_mgmr_status_code_clean_internal();


static void wifi_event_handler(input_event_t *event, void *private_data)
{
    char bssidstr[13] = {0}; 

    switch (event->code)
    {
        case CODE_WIFI_ON_INIT_DONE:
        {
            IOT_InfoLog("CODE_WIFI_ON_INIT_DONE\r\n");
            wifi_mgmr_start_background(&conf);
        }
        break;

        case CODE_WIFI_ON_MGMR_DONE:
        {
            IOT_InfoLog("CODE_WIFI_ON_MGMR_DONE\r\n");
            wifi_is_init = true;
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_INIT_DONE, NULL);
            }
        }
        break;

        case CODE_WIFI_ON_SCAN_DONE:
        {
            IOT_InfoLog("CODE_WIFI_ON_SCAN_DONE\r\n");
            // wifi_mgmr_cli_scanlist();
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_SCAN_DONE, NULL);
            }
        }
        break;

        case CODE_WIFI_ON_DISCONNECT:
        {
            IOT_InfoLog("CODE_WIFI_ON_DISCONNECT\r\n");
            wifiStaconnectStatus = false;
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_DISCONNECT, NULL);
            }
        }
        break;

        case CODE_WIFI_ON_CONNECTING:
        {
            IOT_InfoLog("CODE_WIFI_ON_CONNECTING\r\n");
        }
        break;

        case CODE_WIFI_ON_EMERGENCY_MAC:
        {
            IOT_InfoLog("CODE_WIFI_ON_EMERGENCY_MAC\r\n");

            IOT_InfoLog("[APP] [EVT] EMERGENCY MAC %lld\r\n", aos_now_ms());
            aos_msleep(10);
            extern void hal_reboot();
            hal_reboot();
        }
        break;

        case CODE_WIFI_CMD_RECONNECT:
        {
            IOT_InfoLog("CODE_WIFI_CMD_RECONNECT\r\n");
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_CMD_RECONNECT, NULL);
            }
        }
        break;

        case CODE_WIFI_ON_CONNECTED:
        {
            IOT_InfoLog("CODE_WIFI_ON_CONNECTED\r\n");
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_CONNECTED, NULL);
            }
        }
        break;

        case CODE_WIFI_ON_PRE_GOT_IP:
        {
            IOT_InfoLog("CODE_WIFI_ON_PRE_GOT_IP\r\n");
        }
        break;

        case CODE_WIFI_ON_GOT_IP:
        {
            IOT_InfoLog("CODE_WIFI_ON_GOT_IP\r\n");
            IOT_InfoLog("[SYS] Memory left is %d Bytes\r\n", xPortGetFreeHeapSize());
            wifiStaconnectStatus = true;
            memset(&info, 0, sizeof(info));
            memset(&ip, 0, sizeof(ip));
            memset(&gw, 0, sizeof(gw));
            memset(&mask, 0, sizeof(mask));
            wifi_mgmr_sta_ip_get(&ip.addr, &gw.addr, &mask.addr);
            wifi_mgmr_sta_connect_ind_stat_get(&info);

            for(uint8_t i = 0; i < 6; i++)
            {
                printf("0x%02x ", info.bssid[i]);
            }
            printf("\r\n");

            memset(bssidstr, 0, sizeof(bssidstr));
            snprintf(bssidstr, sizeof(bssidstr), "%02X%02X%02X%02X%02X%02X", info.bssid[0],info.bssid[1],info.bssid[2], info.bssid[3], info.bssid[4], info.bssid[5]);
            bssidstr[sizeof(bssidstr) - 1] = '\0';
            IOT_InfoLog("bssidstr = %s \r\n", bssidstr);

            strcpy(wifi_info.ip, ip4addr_ntoa(&ip));
            strcpy(wifi_info.mask, ip4addr_ntoa(&mask));
            strcpy(wifi_info.gw, ip4addr_ntoa(&gw));

            memcpy(wifi_info.bssid, bssidstr, sizeof(wifi_info.bssid));

            IOT_InfoLog("IP  :%s \r\n", wifi_info.ip);
            IOT_InfoLog("GW  :%s \r\n", wifi_info.gw);
            IOT_InfoLog("MASK:%s \r\n", wifi_info.mask);
            IOT_InfoLog("bssid:%s \r\n", wifi_info.bssid);
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_GOT_IP, &wifi_info);
            }
        }
        break;

        case CODE_WIFI_ON_AP_STARTED:
        {
            IOT_InfoLog("CODE_WIFI_ON_AP_STARTED\r\n");
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_AP_STARTED, NULL);
            }
        }
        break;

        case CODE_WIFI_ON_AP_STOPPED:
        {
            IOT_InfoLog("CODE_WIFI_ON_AP_STOPPED\r\n");
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_AP_STOPPED, NULL);
            }
        }
        break;

        case CODE_WIFI_ON_AP_STA_ADD:
        {
            IOT_InfoLog("CODE_WIFI_ON_AP_STA_ADD\r\n");
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_AP_STA_ADD, NULL);
            }
            wifiAPConnectStatus = true;
        }
        break;

        case CODE_WIFI_ON_AP_STA_DEL:
        {
            IOT_InfoLog("CODE_WIFI_ON_AP_STA_DEL\r\n");
            if(ailinkwifiConnectStatus)
            {
                ailinkwifiConnectStatus(AILINK_WIFI_ON_AP_STA_DEL, NULL);
            }
            wifiAPConnectStatus = false;
        }
        break;

        default:
        {
            blog_warn("unknown event: %u\r\n", event->code);
        }
        break;
    }
}

/**
 * @brief   对芯片进行软件复位
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-30
 */
void ailinkReboot(void)
{
    hal_reboot();
}


/**
 * @brief   获取芯片上的mac地址
 * 
 * @param[in]   str_mac         接收mac的buf
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-31
 */
void ailinkGetMac(char *str_mac, uint16_t str_mac_len)
{
    uint8_t mac[6];

    if(mac == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return;
    }

    wifi_hosal_efuse_read_mac(mac);
    memset(str_mac, 0, str_mac_len);
    snprintf(str_mac, str_mac_len, "%02X%02X%02X%02X%02X%02X", mac[0],mac[1],mac[2], mac[3], mac[4], mac[5]);
    IOT_InfoLog("str_mac = %s \r\n", str_mac);
}

/**
 * @brief   获取剩余的内存大小
 * 
 * @return  uint32_t  返回剩余的内存大小
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-31
 */
uint32_t ailinkGetFreeHeapSize(void)
{
    return xPortGetFreeHeapSize();
}

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
int32_t ailinkWifiInit(void)
{
    if(wifi_is_init)
    {
        IOT_ErrorLog("wifi already init \r\n");
        return -1;
    }
    aos_register_event_filter(EV_WIFI, wifi_event_handler, NULL);
    hal_wifi_start_firmware_task();
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);
    wifi_is_init = true;

    // g_wifi_sta_interface = wifi_mgmr_sta_enable();
    // if (!g_wifi_sta_interface)
    // {
    //     IOT_ErrorLog("g_wifi_sta_interface is NULL \r\n");
    //     return -1;
    // }

    return 0;
}

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
int32_t ailinkWifiDeinit(void)
{
    if(!wifi_is_init)
    {
        IOT_ErrorLog("wifi already deinit \r\n");
        return -1;
    }
    wifi_is_init = false;

    aos_unregister_event_filter(EV_WIFI, wifi_event_handler, NULL);

    return 0;
}

/**
 * @brief   开启WiFi的STA模式
 * 
 * @return  int32_t 
 *          0：成功开启STA模式
 *          -1：开启STA模式失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */

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
int32_t ailinkWifiStaInit(ailinkwifiConnetStatus wifiConnectStatusCb)
{
    if(wifi_sta_init)
    {
        IOT_ErrorLog("wifi sta already init \r\n");
        return -1;
    }

    if(wifiConnectStatusCb)
    {
        ailinkwifiConnectStatus = wifiConnectStatusCb;
    }
    
    // g_wifi_sta_interface = wifi_mgmr_sta_enable();
    // vTaskDelay(100);
    // wifi_mgmr_sta_autoconnect_disable();
    wifi_sta_init = true;
    IOT_InfoLog("wifi sta init \r\n");

    return 0; 
}

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
int32_t ailinkWifiStaDeint(void)
{
    if(!wifi_sta_init)
    {
        IOT_ErrorLog("wifi sta already deinit \r\n");
        return -1;
    }

    ailinkWifiDisconnect();
    vTaskDelay(1000);
    if(g_wifi_sta_interface)
    {
        wifi_mgmr_sta_disable(NULL);
        g_wifi_sta_interface = NULL;
    }
    vTaskDelay(100);

    ailinkwifiConnectStatus = NULL;
    wifi_sta_init = false;

    IOT_InfoLog("wifi sta deinit \r\n");

    return 0;
}

/**
 * @brief   获取WiFi协议栈初始化状态
 * 
 * @return  int32_t 
 *          0：WiFi协议栈已初始化完成
 *          1：WiFi协议栈未初始化完成
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */

/**
 * @brief   获取WiFi协议栈初始化状态
 * 
 * @return  true        协议栈初始化成功
 * @return  false       协议栈初始化失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
bool ailinkGetWifiInitStatus(void)
{
    return wifi_is_init;
}

/**
 * @brief   获取wifi的STA模式初始化状态
 * 
 * @return  true    WiFi的STA已初始化
 * @return  false   WiFi的STA未初始化
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-25
 */
bool ailinkGetWifiStaInitStatus(void)
{
    return wifi_sta_init;
}


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
int32_t ailinkWifiConnect(char *ssid, char *pwd)
{
    int32_t ret = 0;

    if (!ssid)
    {
        IOT_ErrorLog("param err \r\n");
        return -1;
    }
    
    
    g_wifi_sta_interface = wifi_mgmr_sta_enable();
    if (!g_wifi_sta_interface)
    {
        IOT_ErrorLog("g_wifi_sta_interface is NULL \r\n");
        return -1;
    }

    if(pwd)
    {
        IOT_InfoLog("will connect wifi, ssid: %s, pwd: %s\r\n", ssid, pwd);
        ret = wifi_mgmr_sta_connect(g_wifi_sta_interface, ssid, pwd, NULL, NULL, 0, 0);
        IOT_InfoLog("ret = %d \r\n", ret);
        wifiWificonnStatus = true;
        memcpy(wifi_info.ssid, ssid, strlen(ssid));
        memcpy(wifi_info.passwd, pwd, strlen(pwd));
    }
    else
    {
         IOT_InfoLog("will connect wifi, ssid: %s \r\n", ssid);
        ret = wifi_mgmr_sta_connect(g_wifi_sta_interface, ssid, NULL, NULL, NULL, 0, 0);
        IOT_InfoLog("ret = %d \r\n", ret);
        wifiWificonnStatus = true;
        memcpy(wifi_info.ssid, ssid, strlen(ssid));
    }

    wifi_mgmr_sta_autoconnect_enable();

    return 0;
}

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
int32_t ailinkWifiDisconnect(void)
{
    if((!ailinkWifiStaConnectStatus()) && (!wifiWificonnStatus))       /*当设备已断开WiFi了，便不再执行以下任务*/
    {
         IOT_ErrorLog("the wifi already disconnect \r\n");
        return -1;
    }

    wifi_mgmr_sta_autoconnect_disable();
    vTaskDelay(100);
    wifi_mgmr_sta_disconnect();
    vTaskDelay(1000);
    wifiWificonnStatus = false;
    if(g_wifi_sta_interface)
    {
        wifi_mgmr_sta_disable(NULL);
        g_wifi_sta_interface = NULL;
        IOT_ErrorLog("the wifi disconnect \r\n");
    }
    vTaskDelay(100);
    IOT_ErrorLog("the wifi disconnect \r\n");

    return 0;
}


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
int32_t ailinkWifiSoftApInit(char *ssid, char *pwd, ailinkwifiConnetStatus wifiConnectStatus)
{

    if(ssid == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return -1;
    }

    if(wifiConnectStatus)
    {
        ailinkwifiConnectStatus = wifiConnectStatus;
    }
    
    g_wifi_ap_interface = wifi_mgmr_ap_enable();
    wifi_mgmr_ap_start(g_wifi_ap_interface, ssid, 0, pwd, WIFI_AP_CHANNEL);


    return 0;

}

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
int32_t ailinkWifiSoftApDeinit(void)
{
    wifi_mgmr_ap_stop(NULL);

    return 0;
}


/**
 * @brief   获取WiFi STA模式的连接状态
 * 
 * @return  true  STA已成功连接路由
 * @return  false  STA已断开连接路由或是未连接路由
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
bool  ailinkWifiStaConnectStatus(void)
{
    return wifiStaconnectStatus;
}

/**
 * @brief   获取WiFi AP模式的连接状态
 * 
 * @return  true  AP已被连接成功
 * @return  false  AP连接已断开或未被连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
bool  ailinkWifiAPConnectStatus(void)
{
    return wifiAPConnectStatus;
}


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
int32_t ailinkWifiGetRssi(void)
{
    int32_t rssi= 0;

    if(ailinkWifiStaConnectStatus())
    {
        wifi_mgmr_rssi_get((int *)&rssi);

        return rssi;
    }
    else
    {
        return 0;
    }
}

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
int32_t ailinkWifiGetConnectStateCode(void)
{
    int state  = 0;

    wifi_mgmr_state_get(&state);

    return state;
}

/**
 * @brief   清除缓存WiFi连接的状态码
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-15
 */
void ailinkWifiCleanStateCode(void)
{
    wifi_mgmr_status_code_clean_internal();
}


