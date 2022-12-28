/**
 * @file    iotgo_sys.c
 * @brief   业务逻辑功能的实现
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-10|1.0.0  |Ai-Thinker     |创建
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include "blog.h"
#include <aos/yloop.h>
#include <aos/kernel.h>
#include <event_device.h>
#include <bl_wifi.h>
#include <hal_wifi.h>
#include <wifi_mgmr_ext.h>
#include <bl_pm.h>
#include <bl_sys.h>


#include "ailink_socketHal.h"
#include "ailink_mdnsHal.h"
#include "ailink_flashHal.h"
#include "ailink_profile.h"
#include "ailink_timerHal.h"
#include "cJSON.h"
#include "lwip/err.h"
#include <lwip/sockets.h>
#include <lwip/udp.h>
#include <aos/kernel.h>

#include <lwip/apps/mdns.h>
#include "hosal_flash.h"
#include <hosal_wdg.h>

#include "iot_sys.h"
#include "iot_protocol.h"
#include "mcu_system.h"
#include "mcu_process.h"

#include "ailink_wdtHal.h"
#include "ailink_bleHal.h"
#include "ailink_softtimer.h"
#include "ailink_wifiHal.h"
#include "ailink_uartHal.h"
#include "iot_cmd.h"
#include "iot_log.h"
#include "ringbuff.h"
#include "iot_uart.h"


#define  DEVICE_PARAMS_FLASH_BLOCK                        "device_params"


#define         UART_RX_BUFF_MAX_LEN                (1024)
// #define         UART_CMD_MAX_LEN                    (UART_RX_BUFF_MAX_LEN)

// static uint8_t  uart_cmd_buffer[UART_CMD_MAX_LEN] = {0};
static uint8_t  uart_ring_buffer[UART_RX_BUFF_MAX_LEN] = {0};

static char mdns_host_name[65] = {0};
static profile_info_t profile_info = {0};
static profile_event_t profile_event = {0};
static iotgoDeviceInfo device_info = {0};
static bool  ailinkInitstate = false;
static AILINK_TIMER   iot_sys_timer = -1;
static ring_buff_t uart_ring_buff_hd = {0};
static IotDeviceParams iot_device_params = {0};


static void IotUart1DataRecevCb(uint8_t data);


static bool iotCheckDeviceInfoIsEmpty(iotgoDeviceInfo *deviceInfo)
{
    if(deviceInfo == NULL)
    {
        return false;
    }
    if((strlen(deviceInfo->userName) == 0) || (strlen(deviceInfo->userPasswd) == 0) || (strlen(deviceInfo->deviceId) == 0))
    {
        return false;
    }

    return true;
}


static void iotgoGetDeviceInfo(iotgoDeviceInfo *deviceInfo)
{
    char databuf[200] = {0};
    cJSON *cjson_root   = NULL;
    cJSON *cjson_value   = NULL;
    char  *str_value = NULL;

    if(deviceInfo == NULL)
    {
        IOT_InfoLog("param err \r\n");
        return ;
    }

    hosal_flash_raw_read(databuf, DEVICE_INFO_ADDR, sizeof(databuf) - 1);

    cjson_root = cJSON_Parse(databuf);
    if(cjson_root == NULL)
    {
        // IOT_InfoLog("json parse err \r\n");
        return ;
    }

    cjson_value = cJSON_GetObjectItem(cjson_root, "username");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        IOT_InfoLog("str_value = %s \r\n", str_value);
        memcpy(deviceInfo->userName, str_value, strlen(str_value));
        cjson_value = NULL;
    }
    
    cjson_value = cJSON_GetObjectItem(cjson_root, "password");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        IOT_InfoLog("str_value = %s \r\n", str_value);
        memcpy(deviceInfo->userPasswd, str_value, strlen(str_value));
        cjson_value = NULL;
    }    
    
    cjson_value = cJSON_GetObjectItem(cjson_root, "deviceid");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        IOT_InfoLog("str_value = %s \r\n", str_value);
        memcpy(deviceInfo->deviceId, str_value, strlen(str_value));
        cjson_value = NULL;
    }

    cJSON_Delete(cjson_root);
}

void iotgo_sys_entry(void)
{
    uint8_t mac[6];
    char macstr[13]; 
    AilinkRTCTimerHal_t SetTimer = {0};
    AilinkRTCTimerHal_t *timer = NULL;
    char DataBuff[100] = {0};
    static uint16_t WdtCount = 0;
    static uint8_t LogCount = 0;
    uint32_t  cmdlen = 0;
    int32_t    ret = 0;
    IotUartConfig   uart_config = {0};
    IotDeviceParams params = {0};

    /* 关闭日志打印 */
#ifndef DEVICE_LOG_ENABLE
    bl_sys_logall_disable();
#endif

    /* 获取芯片上的mac地址 */
    wifi_hosal_efuse_read_mac(mac);
    memset(macstr, 0, sizeof(macstr));
    snprintf(macstr, sizeof(macstr), "%02X%02X%02X%02X%02X%02X", mac[0],mac[1],mac[2], mac[3], mac[4], mac[5]);
    macstr[sizeof(macstr) - 1] = '\0';
    IOT_InfoLog("macstr = %s \r\n", macstr);

    /* 从flash上获取三元组数据 */
    iotgoGetDeviceInfo(&device_info);
    IotLoadDeviceParams(&params);
    if(params.uart_bound == 0)
    {
        params.uart_bound = 9600;
    }


    AilinkHalRTCTimerInit();            /* RTC时钟初始化 */
    AilinkTimerInit();                  /* 软件定时器初始化 */
    ring_buff_init(&uart_ring_buff_hd, (char *)uart_ring_buffer, UART_RX_BUFF_MAX_LEN);


    SetTimer.year = 2000;
    SetTimer.mon = 1;
    SetTimer.day = 1;
    SetTimer.hour = 0;
    SetTimer.minute = 0;
    SetTimer.second = 0;
    AilinkHalRTCSetTime(&SetTimer);     /* 初始化RTC时间 */

    ailinkWifiInit();                                   /* WiFi协议栈初始化 */
    ailinkBleStackInit();                               /* ble协议栈初始化 */
    vTaskDelay(100);

    /* 创建重复的软件定时器， 用于更新RTC时间和对看门狗进行喂狗 */
    iot_sys_timer = AilinkTimerAdd(AILINK_REPEAT_TIMER);
    if(iot_sys_timer == -1)
    {
        IOT_ErrorLog("add timer fail \r\n");
    }
    
    IotUartDeinit();
    uart_config.params.uart_rx_pin = 7;
    uart_config.params.uart_tx_pin = 16;
    uart_config.params.uart_bound = params.uart_bound;
    uart_config.IotUartDataRecvCb = IotUart1DataRecevCb;
    IotUartInit(&uart_config);

    if(!iotCheckDeviceInfoIsEmpty(&device_info))
    {
        DEVICE_INFO_OUTPUT("Device info is empty \r\n");

        while (1)
        {
            if(ring_buff_get_size(&uart_ring_buff_hd) <= 0)
            {
                /* 从flash上获取三元组数据 */
                iotgoGetDeviceInfo(&device_info);
                if(iotCheckDeviceInfoIsEmpty(&device_info))
                {
                    break;
                }
                vTaskDelay(50);
                continue;
            }

            ring_buff_pop_data(&uart_ring_buff_hd, (DataBuff + cmdlen), 1);
            cmdlen += 1;
            if(cmdlen > 100)
            {
                AT_RESPONSE("ERR\r\n");
                IOT_ErrorLog("rev data[%d]: %s \r\n",cmdlen, DataBuff);
                ring_buff_flush(&uart_ring_buff_hd);
                memset(DataBuff, 0, 100);
                continue;
            }

            if((cmdlen >= 4) && ('\r' == DataBuff[cmdlen-2]) && ('\n' == DataBuff[cmdlen-1]))
            {
                printf("\r\n");
                for(uint8_t i = 0; i < cmdlen; i++)
                {
                    printf("%02x", DataBuff[i]);
                }
                printf("\r\n");
                IOT_DebugLog("rev:%s \r\n", DataBuff);
                IOT_DebugLog("cmdlen:%d \r\n", cmdlen);
                DataBuff[cmdlen-2] = '\0';
                atCmdExecute((char *)DataBuff);
                cmdlen = 0;
                memset(DataBuff, 0, sizeof(DataBuff));
            }
        }

        ring_buff_flush(&uart_ring_buff_hd);
        memset(DataBuff, 0, 100);
    }
    else
    {
        IOT_DebugLog("################# Device Info ################ \r\n");
        IOT_DebugLog("device_info.userName = %s \r\n", device_info.userName);
        IOT_DebugLog("device_info.userPasswd = %s \r\n", device_info.userPasswd);
        IOT_DebugLog("device_info.deviceId = %s \r\n", device_info.deviceId);
        IOT_DebugLog("##############################################\r\n");
    }

    /* ailink IOT协议栈配置 */
    memcpy(profile_info.user_name, device_info.userName, strlen(device_info.userName));
    memcpy(profile_info.user_passwd, device_info.userPasswd, strlen(device_info.userPasswd));
    memcpy(profile_info.deviceid, device_info.deviceId, strlen(device_info.deviceId));
    memcpy(profile_info.fw_version, DEVICE_FW_VERSION, strlen(DEVICE_FW_VERSION));
    memcpy(profile_info.device_type, DEVICE_TYPE, strlen(DEVICE_TYPE));
    memcpy(profile_info.lan_info.mdns_service_type, DEVICE_MDNS_SERVER_TYPE, strlen(DEVICE_MDNS_SERVER_TYPE));
    snprintf(mdns_host_name, sizeof(mdns_host_name), "axy_%s", device_info.deviceId);
    memcpy(profile_info.lan_info.mdns_host_name, mdns_host_name, strlen(mdns_host_name));

    
    profile_event.ailinkRecvControlInfoCb = AilinkRecvControlInfo;
    profile_event.ailinkRecvOTAInfoCb = AilinkRecvOtaInfo;
    profile_event.ailinkEventCb = AilinkEvent;

    McuProcessInit();                                   /* 初始化MCU通信处理 */
    AilinkWdtInit();                                    /* 初始化看门狗 */
    AilinkTimerStart(iot_sys_timer, 500);            /* 启动软件定时器 */
    IotLogInit();

    DEVICE_INFO_OUTPUT("############################## Device info  ##################################\r\n");
    snprintf(DataBuff, sizeof(DataBuff), "build time: %s, fw version: %s \r\n", BUILDTIME, DEVICE_FW_VERSION);
    DEVICE_INFO_OUTPUT(DataBuff);
    memset(DataBuff, 0, sizeof(DataBuff));
    snprintf(DataBuff, sizeof(DataBuff), "userName: %s\r\nuserPasswd: %s\r\ndeviceId: %s\r\n", device_info.userName, device_info.userPasswd, device_info.deviceId);
    DEVICE_INFO_OUTPUT(DataBuff);
    memset(DataBuff, 0, sizeof(DataBuff));
    DEVICE_INFO_OUTPUT("############################## end  ##################################\r\n");

    while(1)
    {
        if(AilinkCheckTimerTimeout(iot_sys_timer) == AILINK_TIMER_TIMEOUT)                    /* 软件定时器超时检测 */
        {
            AilinkWdtReload();                                                      /* 看门狗喂狗 */
            timer = AilinkHalGetRTCTime();                                                  /* 获取RTC时间 */
            if(timer == NULL)
            {
                IOT_ErrorLog("timer is NULL \r\n");
                return ;
            }

            LogCount++;
            if(LogCount > 10)
            {
                IOT_DebugLog("the Date : %02d:%02d:%02d \r\n", timer->year, timer->mon, timer->day);
                IOT_DebugLog("the time : %02d:%02d:%02d \r\n", timer->hour, timer->minute, timer->second);
                IOT_DebugLog("the week : %02d \r\n", timer->week);
                IOT_DebugLog("Memory left is %d Bytes\r\n", xPortGetFreeHeapSize());
                LogCount = 0;
            }
        }

        if(GetMCUConnectState() && !ailinkInitstate)                                /* 该处用于处理MCU通信，但MCU与模组握手成功后，方可执行以下内容 */
        {
            if(GetProductID() == NULL || strlen(GetProductID()) == 0)
            {
                continue;
            }
            if(GetProductFlag() == NULL || strlen(GetProductFlag()) == 0)
            {
                continue;
            }
            if(GetMcuVersion() == NULL || strlen(GetMcuVersion()) == 0)
            {
                continue;
            }
            IOT_DebugLog("productid = %s \r\n", GetProductID());
            IOT_DebugLog("productflag = %s \r\n", GetProductFlag());
            memcpy(profile_info.productid, GetProductID(), strlen(GetProductID()));

            snprintf(DataBuff, sizeof(DataBuff), "axy_%s-%s_p1_", GetProductFlag(), GetProductID());          /* 将AP热点和蓝牙广播名按照协议格式拼接成需要的名称 */
            memcpy(profile_info.pair_info.pair_ap_ssid, DataBuff, strlen(DataBuff));
            memcpy(profile_info.pair_info.pair_ble_adv_name, DataBuff, strlen(DataBuff));
            strncat(profile_info.pair_info.pair_ap_ssid, &macstr[6], 6);
            strncat(profile_info.pair_info.pair_ble_adv_name, &macstr[6], 6);
            memcpy(profile_info.mcu_version, GetMcuVersion(), strlen(GetMcuVersion()));
            IOT_DebugLog("profile_info.pair_info.pair_ap_ssid = %s \r\n", profile_info.pair_info.pair_ap_ssid);
            IOT_DebugLog("profile_info.pair_info.pair_ble_adv_name = %s \r\n", profile_info.pair_info.pair_ble_adv_name);

            AilinkInit(&profile_info, &profile_event);              /* 初始化ailink IOT协议栈 */
            ailinkInitstate = true;
        }

        IotProcotolProcess();
        vTaskDelay(20);
    }
    
}

/**
 * @brief   获取设备的三元组数据
 * 
 * @return  iotgoDeviceInfo* 三元组数据结构
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
iotgoDeviceInfo *IotGetDeviceInfo(void)
{
    return (&device_info);
}



static void IotUart1DataRecevCb(uint8_t data)
{
    char value = (char )data;

    // printf("%c", value);
    // printf("%02x", data);
    ring_buff_push_data(&uart_ring_buff_hd, &value, 1);
}

/**
 * @brief   存储设备参数数据
 * 
 * @param[in]   params      待存储数据参数
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void IotSaveDeviceParams(IotDeviceParams *params)
{
    if(params == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return ;
    }
    iot_device_params.uart_bound = params->uart_bound;
    iot_device_params.work_mode = params->work_mode;

    AilinkFlashWrite(DEVICE_PARAMS_FLASH_BLOCK, (uint8_t *)&iot_device_params, sizeof(IotDeviceParams));
}


/**
 * @brief   清除设备存储的参数数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void IotClearDeviceParams(void)
{
    memset(&iot_device_params, 0, sizeof(IotDeviceParams));
    AilinkFlashWrite(DEVICE_PARAMS_FLASH_BLOCK, (uint8_t *)&iot_device_params, sizeof(IotDeviceParams));
}

/**
 * @brief   读取设备存储的参数数据
 * 
 * @param[in]   params  已读取的参数数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void IotLoadDeviceParams(IotDeviceParams *params)
{
    if(params == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return ;
    }

    memset(&iot_device_params, 0, sizeof(IotDeviceParams));
    AilinkFlashRead(DEVICE_PARAMS_FLASH_BLOCK, (uint8_t *)&iot_device_params, sizeof(IotDeviceParams));
    IOT_DebugLog("iot_device_params.uart_bound = %d \r\n", iot_device_params.uart_bound);
    IOT_DebugLog("iot_device_params.work_mode = %d \r\n", iot_device_params.work_mode);

    memcpy(params, &iot_device_params, sizeof(IotDeviceParams));
}


