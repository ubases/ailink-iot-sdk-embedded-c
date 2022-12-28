/**
 * @file    iot_protocol.c
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-11
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-08-11|1.0.0  |Ai-Thinker      |创建
 */

#include "iot_protocol.h"
#include "ailink_profile.h"
#include "iot_log.h"
#include "iot_sys.h"
#include "mcu_system.h"
#include "ailink_ota.h"
#include "ailink_otaHal.h"
#include "iot_cmd.h"
#include "ailink_profile.h"
#include "ailink_wifiHal.h"
#include "ailink_osHal.h"
#include "hosal_flash.h"
#include "ailink_timerHal.h"
#include "md5.h"
#include "ailink_socketHal.h"
#include "ailink_flashHal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <bl_sys.h>
#include <hosal_gpio.h>
#include "bl_gpio.h"
#include <wifi_hosal.h>

#define     IOT_PROTOCOL_STR_CONTROL            "control"
#define     PROTOCOL_STR_PARAM                  "param"
#define     PROTOCOL_STR_CHANNEL                "chanel"
#define     PROTOCOL_STR_POINT_VER               "pointVer"
#define     PROTOCOL_STR_BASE_VER                "baseVer"
#define     PROTOCOL_STR_MCU_BASE_VER            "mcuBaseVer"
#define     PROTOCOL_STR_OTA_TYPE               "otaType"
#define     PROTOCOL_STR_APP_URL                "appUrl"
#define     PROTOCOL_STR_MCU_URL                "mcuUrl"
#define     PROTOCOL_STR_PROGRESS               "progress"
#define     PROTOCOL_STR_OTA_STATUS             "otaState"
#define     PROTOCOL_STR_OTA_VER                 "otaVer"
#define     PROTOCOL_STR_PUBID                  "pubId"


#define DEVICE_OTA_INFO_FLASH_BLOCK              "ota_info"


#define     IOT_PROTOCOL_DATA_MAX_LEN           (1024)
#define     PROTOCOL_LOCAL_OTA_TYPE                     (1)
#define     PROTOCOL_MCU_OTA_TYPE                       (2)
#define     PROTOCOL_OTA_TIMEOUT                        (2 * 60 * 1000)


#define     LEDTEST_THREAD_NAME            "led thread"
#define     LEDTEST_THREAD_STACK            ((1 * 1024)/ 4)
#define     LEDTEST_THREAD_PRIO             15



#define  IOT_PROCOTOL_EVENT_INVAILD                      0
#define  IOT_PROCOTOL_EVENT_SEND_CMD_TO_MCU              1
#define  IOT_PROCOTOL_EVENT_OTA_START                    2


#define  LED_PIN_B                                      3
#define  LED_PIN_R                                      14
#define  LED_PIN_G                                      17
#define  LED_PIN_W                                      1
#define  LED_PIN_C                                      5


static iotgoDeviceInfo DeviceInfo = {0};
static ailinkOtaProInfo  OtaProInfo = {0};
static Ailink_mutex_t Iotmetex = NULL;
static uint8_t IotProcotolEvent = IOT_PROCOTOL_EVENT_INVAILD;
static uint8_t IotLastEvent = IOT_PROCOTOL_EVENT_INVAILD;
static uint8_t  McuCmdDataBuf[IOT_PROTOCOL_DATA_MAX_LEN] = {0};
static uint16_t McuCmdDataLen = 0;
static uint8_t QuertCmdCount = 0;
static bool     MucCmdSendRetry = false;
static uint32_t lastTick = 0;
static uint32_t CurTick = 0;
static bool waterfall_light = false;
static bool waterfall_light_state = false;
static uint8_t waterfall_num = 0;
static hosal_gpio_dev_t led_b_gpio = {0};
static hosal_gpio_dev_t led_r_gpio = {0};
static hosal_gpio_dev_t led_g_gpio = {0};
static hosal_gpio_dev_t led_w_gpio = {0};
static hosal_gpio_dev_t led_c_gpio = {0};

static int32_t IotProtocolParse(char *data);
static int32_t AilinkOtaProInfoParse(char *data, ailinkOtaProInfo *pro_info);
static void IotSaveDeviceInfo(void);
static int32_t IotMutexLock(void);
static int32_t IotMutexUnLock(void);
static void IotLoadOTAInfo(void);
static void IotClearOTAInfo(void);
static void IotSaveOTAInfo(void);
static char *strrstr( char *dest,char *src);
extern void AilinkSetLogLevel(uint8_t level);
extern void McuSetLogLevel(uint8_t level);

/**
 * @brief   接收设备控制的json字符串数据回调函数
 * 
 * @param[in]   data        json字符串事件
 * @param[in]   mid         消息ID数据
 * @param[in]   ts          时间戳
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-16
 */
void AilinkRecvControlInfo(char *data, char *mid, uint32_t ts)
{
    IOT_InfoLog("########### Receive Control Data ###################\r\n");
    IOT_DebugLog("data = %s \r\n", data);
    IOT_DebugLog("mid = %s \r\n", mid);
    IOT_DebugLog("ts = %d \r\n", ts);

    IotProtocolParse(data);
}

/**
 * @brief   接收设备OTA的json字符串数据回调函数
 * 
 * @param[in]   data        json字符串事件
 * @param[in]   mid         消息ID数据
 * @param[in]   ts          时间戳
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-16
 */
void AilinkRecvOtaInfo(char *data, char *mid, uint32_t ts)
{
    int32_t ret = 0;
    

    IOT_InfoLog("########### Receive OTA Data ###################\r\n");
    IOT_DebugLog("data = %s \r\n", data);
    IOT_DebugLog("mid = %s \r\n", mid);
    IOT_DebugLog("ts = %d \r\n", ts);

    if(AilinkOtaIsStart())
    {
        IOT_ErrorLog("get profile info err \r\n");
        AilinkPublicReportProgress(7, NULL, OtaProInfo.OtaVer,OtaProInfo.PubId,0xffffffff);
        return ;
    }

    IotClearOTAInfo();
    ret = AilinkOtaProInfoParse(data, &OtaProInfo);
    if(ret == 0)
    {
        // AilinkPublicReportProgress(0, NULL, OtaProInfo.OtaVer,OtaProInfo.PubId,0xffffffff);
        IotSaveOTAInfo();
        IOT_InfoLog("########### Pre Start Ota ###################\r\n");
        IotProcotolEvent = IOT_PROCOTOL_EVENT_OTA_START;
    }
    else
    {
        if(ret == -3)
        {
            AilinkPublicReportProgress(6, NULL,OtaProInfo.OtaVer,OtaProInfo.PubId,0xffffffff);
        }
        else if(ret == -2)
        {
            AilinkPublicReportProgress(3, NULL, OtaProInfo.OtaVer,OtaProInfo.PubId,0xffffffff);
        }
        else 
        {
            AilinkPublicReportProgress(5, NULL, OtaProInfo.OtaVer,OtaProInfo.PubId,0xffffffff);
        }
        IOT_ErrorLog("ota info parse fail \r\n");
        AilinkPublicReportProgress(3, NULL, OtaProInfo.OtaVer,OtaProInfo.PubId,0xffffffff);
    }
}

/**
 * @brief   接收ailink IOT协议栈中的事件回调函数
 * 
 * @param[in]   event        回调事件
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-16
 */
void AilinkEvent(uint8_t event)
{
    switch (event)
    {
        case AILINK_AP_BLE_EVENT:
        {
            IOT_InfoLog("AILINK_AP_BLE_EVENT \r\n");
            SetWifiNetWorkState(BLE_AND_AP_STATE);
        }
        break;

        case AILINK_AP_EVENT:
        {
            IOT_InfoLog("AILINK_AP_EVENT \r\n");
            SetWifiNetWorkState(AP_CONFIG_STATE);
        }
        break;

        case AILINK_BLE_EVENT:
        {
            IOT_InfoLog("AILINK_BLE_EVENT \r\n");
            SetWifiNetWorkState(BLE_CONFIG_STATE);
        }
        break;

        case AILINK_WIFI_DISCONNECT_EVENT:
        {
            IOT_InfoLog("AILINK_WIFI_DISCONNECT_EVENT \r\n");
            SetWifiNetWorkState(WIFI_NOT_CONNECTED);
        }
        break;

        case AILINK_WIFI_CONNECT_EVENT:
        {
            IOT_InfoLog("AILINK_WIFI_CONNECT_EVENT \r\n");
            SetWifiNetWorkState(WIFI_CONNECTED);
        }
        break;

        case AILINK_CLOUD_DISCONNECT_EVENT:
        {
            IOT_InfoLog("AILINK_CLOUD_DISCONNECT_EVENT \r\n");
            // SetWifiNetWorkState(BLE_AND_AP_STATE);
        }
        break;

        case AILINK_CLOUD_CONNECT_EVENT:
        {
            IOT_InfoLog("AILINK_CLOUD_CONNECT_EVENT \r\n");
            SetWifiNetWorkState(WIFI_CONN_CLOUD);
            if(AilinkGetOtaInstallStatus())
            {
                IotLoadOTAInfo();
                AilinkPublicReportProgress(0, NULL, OtaProInfo.OtaVer,OtaProInfo.PubId,0xffffffff);
                AilinkSetOtaInstallStatus(false);
            }
        }
        break;

        case AILINK_CLOUD_QUERY_EVENT:
        {
            IOT_InfoLog("AILINK_CLOUD_QUERY_EVENT \r\n");
            UartProFrameSend(STATE_QUERY_CMD, NULL, 0);
        }
        break;

        case AILINK_DEVICE_RESTORE_EVENT:
        {
            IOT_InfoLog("AILINK_DEVICE_RESTORE_EVENT \r\n");
            // SetWifiNetWorkState(BLE_AND_AP_STATE);
        }
        break;

        case AILINK_DEVICE_REBOOT_EVENT:
        {
            IOT_InfoLog("AILINK_DEVICE_REBOOT_EVENT \r\n");
            // SetWifiNetWorkState(BLE_AND_AP_STATE);
        }
        break;
        
        default:
            break;
    }
}


/**
 * @brief   获取Tick时间差
 * 
 * @param[in]   start    开始的tick时间
 * @param[in]   end      结束的tick时间
 * @return  uint32_t     返回tick时间差
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-18
 */
static uint32_t diffTimeTick(uint32_t start, uint32_t end)
{
    return ((end >= start) ? (end-start) : ((0xFFFFFFFF-start) + end));
}



/**
 * @brief   事件处理函数
 * @note    循环调用该函数，时刻处理事件
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
void IotProcotolProcess(void)
{
    profile_info_t *profile_info = NULL;
    ailink_ota_config  OtaConfig = {0};

    IotMutexLock();

    switch (IotProcotolEvent)
    {
        case IOT_PROCOTOL_EVENT_INVAILD:
        {
            if(IotProcotolEvent != IOT_PROCOTOL_EVENT_INVAILD)
            {
                IotLastEvent = IotProcotolEvent;
                IOT_InfoLog("IOT_PROCOTOL_EVENT_INVAILD \r\n");
            }
        }
        break;

        case IOT_PROCOTOL_EVENT_SEND_CMD_TO_MCU:
        {
            if(IotProcotolEvent != IOT_PROCOTOL_EVENT_SEND_CMD_TO_MCU)
            {
                IotLastEvent = IotProcotolEvent;
                IOT_InfoLog("IOT_PROCOTOL_EVENT_SEND_CMD_TO_MCU \r\n");
            }

            if(!MucCmdSendRetry)
            {
                McuSendQuertCmd(McuCmdDataBuf, McuCmdDataLen);
                MucCmdSendRetry = true;
                QuertCmdCount++;
                lastTick = ailinkGetSystemTick();
                IOT_InfoLog("send cmd data to mcu \r\n");
            }

            if(!McuGetSendCmdState() || QuertCmdCount > 10)
            {
                MucCmdSendRetry = false;
                QuertCmdCount = 0;
                IotProcotolEvent = IOT_PROCOTOL_EVENT_INVAILD;
                IOT_InfoLog("stop send cmd data to mcu \r\n");
            }

            CurTick = ailinkGetSystemTick();
            if(CurTick - lastTick > 2000)
            {
                lastTick = CurTick;
                MucCmdSendRetry = false;
            } 

        }
        break;

        case IOT_PROCOTOL_EVENT_OTA_START:
        {
            if(IotProcotolEvent != IOT_PROCOTOL_EVENT_OTA_START)
            {
                IotLastEvent = IotProcotolEvent;
                IOT_InfoLog("IOT_PROCOTOL_EVENT_OTA_START \r\n");
            }

            
            profile_info = AilinkGetProfileInfo();
            if(profile_info == NULL)
            {
                IOT_ErrorLog("get profile info err \r\n");
                AilinkPublicReportProgress(5, NULL, OtaProInfo.OtaVer,OtaProInfo.PubId,0xffffffff);
                return ;
            }
            IotLoadOTAInfo();
            memcpy(&(OtaConfig.ProInfo), &OtaProInfo, sizeof(OtaProInfo));
            memcpy(OtaConfig.head_info.device_fw_version, profile_info->fw_version, strlen(profile_info->fw_version));
            memcpy(OtaConfig.head_info.device_productid, profile_info->productid, strlen(profile_info->productid));
            OtaConfig.head_info.ota_type = PROTOCOL_LOCAL_OTA_TYPE;
            OtaConfig.OtaStatusCallback = AilinkOtaStatusCallback;
            OtaConfig.OtaDownloadCallback = AilinkOtaDownloadCallback;
            OtaConfig.McuOtaDownloadCallback = AilinkMcuOtaDownloadCallback;
            OtaConfig.ota_timeout = PROTOCOL_OTA_TIMEOUT;
            IOT_InfoLog("########### Start Ota ###################\r\n");
            AilinkOtaInit(&OtaConfig);

            IotProcotolEvent = IOT_PROCOTOL_EVENT_INVAILD;
        }
        
        default:
            break;
    }
    IotMutexUnLock();

    if(waterfall_light)
    {
        if(!waterfall_light_state)
        {
            lastTick = ailinkGetSystemTick();
            waterfall_light_state = true;
        }

        CurTick = ailinkGetSystemTick();
        if(diffTimeTick(lastTick, CurTick) > 1000)
        {
            lastTick = CurTick;
            
            switch (waterfall_num)
            {
                case 0:
                {
                    // hosal_gpio_output_set(&led_b_gpio, 1);
                    // hosal_gpio_output_set(&led_c_gpio, 0);
                    bl_gpio_output_set(LED_PIN_B, 1);
                    bl_gpio_output_set(LED_PIN_G, 0);
                    IOT_InfoLog("led_b on, led_c off \r\n");
                    waterfall_num++;
                }
                break;

                case 1:
                {
                    // hosal_gpio_output_set(&led_r_gpio, 1);
                    // hosal_gpio_output_set(&led_b_gpio, 0);
                    bl_gpio_output_set(LED_PIN_R, 1);
                    bl_gpio_output_set(LED_PIN_B, 0);
                    IOT_InfoLog("led_r on, led_b off \r\n");
                    waterfall_num++;
                }
                break;

                case 2:
                {
                    // hosal_gpio_output_set(&led_g_gpio, 1);
                    // hosal_gpio_output_set(&led_r_gpio, 0);
                    bl_gpio_output_set(LED_PIN_G, 1);
                    bl_gpio_output_set(LED_PIN_R, 0);
                    IOT_InfoLog("led_g on, led_r off \r\n");
                    waterfall_num=0;
                }
                break;
                
                default:
                    break;
            }
        }

    }

}


/**
 * @brief   IOT 物模型控制协议
 * 
 * @param[in]   data    json字符串数据
 * @return  int32_t 
 *          0：解析协议成功
 *          -1：解析协议失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-25
 */
static int32_t IotProtocolParse(char *data)
{
    cJSON *cjson_root   = NULL;
    cJSON *cjson_control   = NULL;
    cJSON *cjson_value   = NULL;
    char  str[10] = {0};
    uint8_t num = 0;
    uint32_t valueNum = 0;
    char     *valueString = NULL;
    uint16_t valueLen = 0;

    if(data == NULL)
    {
        IOT_ErrorLog("Param err \r\n");
        return -1;
    }

    cjson_root = cJSON_Parse(data);
    if(cjson_root == NULL)
    {
        IOT_ErrorLog("json parse err \r\n");
        return -1;
    }

    memset(McuCmdDataBuf, 0, sizeof(McuCmdDataBuf));
    McuCmdDataLen = 0;
    cjson_control = cJSON_GetObjectItem(cjson_root, IOT_PROTOCOL_STR_CONTROL);
    if(cjson_control)
    {
        for(num = 0; num < 255; num++)
        {
            snprintf(str, 10, "%d", num);
            // IOT_InfoLog("num = %s \r\n", str);

            
            cjson_value = cJSON_GetObjectItem(cjson_control, str);
            if(cjson_value)
            {
                McuCmdDataBuf[McuCmdDataLen + 0] = num;
                IOT_InfoLog("num = %s \r\n", str);
                if(cjson_value->type == cJSON_Number)
                {
                    valueNum = cJSON_GetNumberValue(cjson_value);
                    IOT_InfoLog("valueNum = %d \r\n", valueNum);
                    McuCmdDataBuf[McuCmdDataLen + 1] = DATA_TYPE_VALUE;
                    valueLen = 4;
                    McuCmdDataBuf[McuCmdDataLen + 2] = ((valueLen >> 8) & 0xff);
                    McuCmdDataBuf[McuCmdDataLen + 3] = (valueLen & 0xff);
                    McuCmdDataBuf[McuCmdDataLen + 4] = ((valueNum >> 24) & 0xff);
                    McuCmdDataBuf[McuCmdDataLen + 5] = ((valueNum >> 16) & 0xff);;
                    McuCmdDataBuf[McuCmdDataLen + 6] = ((valueNum >> 8) & 0xff);;
                    McuCmdDataBuf[McuCmdDataLen + 7] = (valueNum & 0xff);
                    McuCmdDataLen += 4 + valueLen;
                }
                else if(cjson_value->type == cJSON_String)
                {
                    valueString = cJSON_GetStringValue(cjson_value);
                    if(valueString)
                    {
                        IOT_InfoLog("valueString = %s \r\n", valueString);
                        McuCmdDataBuf[McuCmdDataLen + 1] = DATA_TYPE_STRING;
                        valueLen = strlen(valueString);
                        McuCmdDataBuf[McuCmdDataLen + 2] = ((valueLen >> 8) & 0xff);
                        McuCmdDataBuf[McuCmdDataLen + 3] = (valueLen & 0xff);
                        memcpy(&McuCmdDataBuf[McuCmdDataLen + 4], valueString, valueLen);
                        McuCmdDataLen += 4 + valueLen;
                    }
                    else
                    {
                        IOT_ErrorLog("json string value is empty \r\n");
                    }
                }
                else if(cjson_value->type == cJSON_Array)
                {
                    IOT_InfoLog("receive value array \r\n");
                }
                else if(cjson_value->type == cJSON_Object)
                {
                    IOT_InfoLog("receive value object \r\n");
                }
                else if(cjson_value->type == cJSON_True)
                {
                    IOT_InfoLog("value is true \r\n");
                    McuCmdDataBuf[McuCmdDataLen + 1] = DATA_TYPE_BOOL;
                    valueLen = 1;
                    McuCmdDataBuf[McuCmdDataLen + 2] = ((valueLen >> 8) & 0xff);
                    McuCmdDataBuf[McuCmdDataLen + 3] = (valueLen & 0xff);
                    McuCmdDataBuf[McuCmdDataLen + 4] = 1;
                    McuCmdDataLen += 4 + valueLen;
                }
                else if(cjson_value->type == cJSON_False)
                {
                    IOT_InfoLog("value is false \r\n");
                    McuCmdDataBuf[McuCmdDataLen + 1] = DATA_TYPE_BOOL;
                    valueLen = 1;
                    McuCmdDataBuf[McuCmdDataLen + 2] = ((valueLen >> 8) & 0xff);
                    McuCmdDataBuf[McuCmdDataLen + 3] = (valueLen & 0xff);
                    McuCmdDataBuf[McuCmdDataLen + 4] = 0;
                    McuCmdDataLen += 4 + valueLen;
                }
                else
                {
                    IOT_ErrorLog("find not value type \r\n");
                }
            }
            memset(str, 0, sizeof(str));
        }
    }
    else
    {
        IOT_ErrorLog("cjson_param is NULL \r\n");
        cJSON_Delete(cjson_root);
        return -1;
    }


    IOT_InfoLog("McuCmdDataLen = %d \r\n", McuCmdDataLen);
    for(uint16_t i = 0; i < McuCmdDataLen; i++)
    {
        printf("0x%02x ", McuCmdDataBuf[i]);
    }
    printf("\r\n");
    // UartProFrameSend(DATA_QUERT_CMD, McuCmdDataBuf, McuCmdDataLen);
    IOT_InfoLog("########### Pre Mcu Send Cmd ###################\r\n");
    IotProcotolEvent = IOT_PROCOTOL_EVENT_SEND_CMD_TO_MCU;
    MucCmdSendRetry = false;

    cJSON_Delete(cjson_root);

    return 0;
}



/**
 * @brief   比较OTA版本号
 * 
 * @param[in]   target_version      OTA固件版本号
 * @param[in]   source_version      模组固件版本号
 * @return  int8_t 
 *          1: OTA固件版本号大于模组固件版本号
 *          0：OTA固件版本号等于模组固件版本号
 *          -1：OTA固件版本号小于模组固件版本号
 *          -2：比较失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-24
 */
static int8_t AilinkCheckVersion(char *target_version, char *source_version)
{
    int order = 0;
    double res = 0;
    const char *p_v1 = target_version;
    const char *p_v2 = source_version;

    if(p_v1 == NULL || p_v2 == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return -2;
    }

    IOT_InfoLog("target_version = %s \r\n", p_v1);
    IOT_InfoLog("source_version = %s \r\n", p_v2);

    if(strlen(target_version) == 0)
    {
        IOT_InfoLog("target_version < source_version \r\n");
        return -1;
    }
    else if(strlen(source_version) == 0)
    {
        IOT_InfoLog("target_version > source_version \r\n");
        return 1;
    }

    while (*p_v1 && *p_v2) 
    {
        char buf_v1[32] = {0};
        char buf_v2[32] = {0};

        char *i_v1 = strchr(p_v1, '.');
        char *i_v2 = strchr(p_v2, '.');

        if (!i_v1 || !i_v2) 
        {
            break;
        }

        if (i_v1 != p_v1) 
        {
            strncpy(buf_v1, p_v1, i_v1 - p_v1);
            p_v1 = i_v1;
        }
        else
        {
            p_v1++;
        }

        if (i_v2 != p_v2) 
        {
            strncpy(buf_v2, p_v2, i_v2 - p_v2);
            p_v2 = i_v2;
        }
        else
        {
            p_v2++;
        }

        order = atoi(buf_v1) - atoi(buf_v2);
        if (order != 0)
        {
            IOT_InfoLog("order = %d \r\n", order);
            res = order < 0 ? 2 : 1;
            if (res == 2)
            {
                IOT_InfoLog("target_version < source_version \r\n");
                return -1;
            }
            else
            {
                IOT_InfoLog("target_version > source_version \r\n");
                return 1;
            }
        }

        vTaskDelay(20);
    }

    res = atof(p_v1) - atof(p_v2);

    if (res < 0)
    {
        IOT_InfoLog("target_version < source_version \r\n");
        return -1;
    }
    if (res > 0)
    {
        IOT_InfoLog("target_version > source_version \r\n");
        return 1;
    }

    IOT_InfoLog("target_version == source_version \r\n");
    return 0;
}



/**
 * @brief   解析OTA升级信息
 * 
 * @param[in]   data   属性数据
 * @param[in]   pro_info   ota处理信息
 * @return  int32_t 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-08
 */
static int32_t AilinkOtaProInfoParse(char *data, ailinkOtaProInfo *pro_info)
{
    cJSON *cjson_root   = NULL;
    cJSON *cjson_param   = NULL;
    cJSON *cjson_value   = NULL;
    char  *str_value = NULL;
    int32_t ret = 0;
    // uint8_t channel = 0;
    bool url_exit = false;

    if(data == NULL || pro_info == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return -1;
    }


    cjson_root = cJSON_Parse(data);
    if(cjson_root == NULL)
    {
        IOT_ErrorLog("json parse err \r\n");
        return -2;
    }

    cjson_param = cJSON_GetObjectItem(cjson_root, PROTOCOL_STR_PARAM);
    if(cjson_param == NULL)
    {
        IOT_ErrorLog("cjson_param is NULL \r\n");
        cJSON_Delete(cjson_root);
        return -2;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_CHANNEL);
    if(cjson_value)
    {
        pro_info->channel = cJSON_GetNumberValue(cjson_value);
        IOT_InfoLog("pro_info->channel = %d \r\n", pro_info->channel);
        cjson_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_OTA_VER);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            if(strlen(str_value) != 0)
            {
                IOT_InfoLog("str_value = %s \r\n", str_value);
                memcpy(pro_info->OtaVer, str_value, strlen(str_value));

                IOT_InfoLog("pro_info->OtaVer = %s \r\n", pro_info->OtaVer);
                if(AilinkCheckVersion(pro_info->OtaVer, DEVICE_FW_VERSION) != 1)
                {
                    IOT_ErrorLog("OTA version err \r\n");
                    cJSON_Delete(cjson_root);
                    return -3;
                }
            }
        }
        cjson_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_POINT_VER);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            if(strlen(str_value) != 0)
            {
                IOT_InfoLog("str_value = %s \r\n", str_value);
                memcpy(pro_info->pointVer, str_value, strlen(str_value));

                IOT_InfoLog("pro_info->pointVer = %s \r\n", pro_info->pointVer);
                if(AilinkCheckVersion(pro_info->pointVer, DEVICE_FW_VERSION) != 0)
                {
                    IOT_ErrorLog("OTA version err \r\n");
                    cJSON_Delete(cjson_root);
                    return -3;
                }
            }
        }
        cjson_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_BASE_VER);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            if(strlen(str_value) != 0)
            {
                IOT_InfoLog("str_value = %s \r\n", str_value);
                memcpy(pro_info->baseVer, str_value, strlen(str_value));

                IOT_InfoLog("pro_info->baseVer = %s \r\n", pro_info->baseVer);

                if(AilinkCheckVersion(pro_info->pointVer, DEVICE_FW_VERSION) == 1)
                {
                    IOT_ErrorLog("OTA version err \r\n");
                    cJSON_Delete(cjson_root);
                    return -3;
                }
            }
        }
        cjson_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_MCU_BASE_VER);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            if(strlen(str_value) != 0)
            {
                IOT_InfoLog("str_value = %s \r\n", str_value);
                memcpy(pro_info->mcuBaseVer, str_value, strlen(str_value));

                IOT_InfoLog("pro_info->mcuBaseVer = %s \r\n", pro_info->mcuBaseVer);
            }
        }
        cjson_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_OTA_TYPE);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            IOT_InfoLog("str_value = %s \r\n", str_value);
            memcpy(pro_info->otaType, str_value, strlen(str_value));

            IOT_InfoLog("pro_info->otaType = %s \r\n", pro_info->otaType);
        }
        cjson_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_APP_URL);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            if(strlen(str_value) != 0)
            {
                IOT_InfoLog("str_value = %s \r\n", str_value);
                memcpy(pro_info->download_url, str_value, strlen(str_value));

                IOT_InfoLog("pro_info->download_url = %s \r\n", pro_info->download_url);
                url_exit = true;
            }
        }

        cjson_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_MCU_URL);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            if(strlen(str_value) != 0)
            {
                IOT_InfoLog("str_value = %s \r\n", str_value);
                memcpy(pro_info->download_url, str_value, strlen(str_value));

                IOT_InfoLog("pro_info->download_url = %s \r\n", pro_info->download_url);
                url_exit = true;
            }
        }

        cjson_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_param, PROTOCOL_STR_PUBID);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            if(strlen(str_value) != 0)
            {
                str_value = cJSON_GetStringValue(cjson_value);
                IOT_InfoLog("str_value = %s \r\n", str_value);
                memcpy(pro_info->PubId, str_value, strlen(str_value));

                IOT_InfoLog("pro_info->PubId = %s \r\n", pro_info->PubId);
                url_exit = true;
            }
        }
        cjson_value = NULL;
    }

    if(!url_exit)
    {
        ret = -2;
    }


    cJSON_Delete(cjson_root);
    IOT_InfoLog("############ OTA Info parse finish ############ \r\n");

    return ret;
}




/*strrstr函数的实现，查找字符子串src在字符串dest中最后一次出现的位置*/
static char *strrstr( char *dest,char *src)
{
	char *p = dest;
	while(*p != '\0')		//将指针指向末尾
		p++;
	for(p = p-1;p >= dest;p--)
	{
		if((*p == *src) && ((strncmp(p,src,strlen(src))) == 0))
			return p;
	}
	return NULL;
}

/**
 * @brief   获取Ota信息数据
 * 
 * @return  ailinkOtaProInfo*   返回Ota信息数据缓存buf的指针
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-24
 */
ailinkOtaProInfo *IotGetOtaProInfo(void)
{
    IotLoadOTAInfo();
    return (&OtaProInfo);
}


static int32_t IotMutexLock(void)
{
    if(!Iotmetex)
    {
        Iotmetex = Ailink_mutex_new();
        if(Iotmetex == NULL)
        {
            IOT_ErrorLog("mutex create failed \r\n");
            return -1;
        }
    }

    return Ailink_mutex_lock(Iotmetex);
}


static int32_t IotMutexUnLock(void)
{
    if(!Iotmetex)
    {
        Iotmetex = Ailink_mutex_new();
        if(Iotmetex == NULL)
        {
            IOT_ErrorLog("mutex create failed \r\n");
            return -1;
        }
    }

    return Ailink_mutex_unlock(Iotmetex);
}


static void GPIO_IRQ_Handler(void *arg)
{
    IOT_InfoLog("gpio irq trigger \r\n");
    AT_RESPONSE("\r\n##boot\r\n");

}



/* ############################################################################################################### */
/* #################################### AT指令处理函数 ##############################################################*/



/**
 * @brief   开发板测试使能
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-11-16
 */
void AT_NodeMCUTEST(char *cmd, uint16_t cmd_len)
{
    hosal_gpio_dev_t boot_gpio = {0};
    uint8_t argnum = 8;

    if(cmd == NULL)
    {
        IOT_ErrorLog("params err \r\n");
        return ;
    }

    if(cmd_len == 2)
    {
        // AT_RESPONSE("OK\r\n");
        if (cmd[0] == '=')
        {
            if(cmd[1] == '1')
            {
                boot_gpio.port = 8;
                boot_gpio.config = INPUT_PULL_DOWN;
                hosal_gpio_init(&boot_gpio);
                IOT_InfoLog("boot gpio init \r\n");
                hosal_gpio_irq_set(&boot_gpio, HOSAL_IRQ_TRIG_POS_PULSE, GPIO_IRQ_Handler, NULL);
                IOT_InfoLog("boot gpio  irq set \r\n");
                AT_RESPONSE("OK\r\n");
            }
            else if(cmd[1] == '0')
            {
                boot_gpio.port = 8;
                boot_gpio.config = INPUT_PULL_DOWN;
                hosal_gpio_finalize(&boot_gpio);
                IOT_InfoLog("boot gpio reset init \r\n");
                AT_RESPONSE("OK\r\n");
            }
            else
            {
                IOT_DebugLog("not find parameter \r\n");
                AT_RESPONSE("ERR\r\n");
            }
        }
        else
        {
            IOT_DebugLog("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
}

/**
 * @brief   开发板 LED 测试指令
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-11-16
 */
void AT_LEDTEST(char *cmd, uint16_t cmd_len)
{

    if(cmd == NULL)
    {
        IOT_ErrorLog("params err \r\n");
        return ;
    }

    if(cmd_len == 2)
    {
        // AT_RESPONSE("OK\r\n");
        if (cmd[0] == '=')
        {
            if(cmd[1] == '1')
            {
                // led_b_gpio.port = LED_PIN_B;
                // led_b_gpio.config = OUTPUT_OPEN_DRAIN_NO_PULL;
                // hosal_gpio_init(&led_b_gpio);
                // led_r_gpio.port = LED_PIN_R;
                // led_r_gpio.config = OUTPUT_OPEN_DRAIN_NO_PULL;
                // hosal_gpio_init(&led_r_gpio);
                // led_g_gpio.port = LED_PIN_G;
                // led_g_gpio.config = OUTPUT_OPEN_DRAIN_NO_PULL;
                // hosal_gpio_init(&led_g_gpio);
                // led_w_gpio.port = LED_PIN_W;
                // led_w_gpio.config = OUTPUT_OPEN_DRAIN_NO_PULL;
                // hosal_gpio_init(&led_w_gpio);
                // led_c_gpio.port = LED_PIN_C;
                // led_c_gpio.config = OUTPUT_OPEN_DRAIN_NO_PULL;
                // hosal_gpio_init(&led_c_gpio);

                bl_gpio_enable_output(LED_PIN_B, 0, 0);
                bl_gpio_enable_output(LED_PIN_R, 0, 0);
                bl_gpio_enable_output(LED_PIN_G, 0, 0);
                // bl_gpio_enable_output(LED_PIN_W, 0, 0);
                // bl_gpio_enable_output(LED_PIN_C, 0, 0);
                IOT_InfoLog("waterfall_light turn on \r\n");
                waterfall_light = true;
                AT_RESPONSE("OK\r\n");
            }
            else if(cmd[1] == '0')
            {
                IOT_InfoLog("waterfall_light turn off \r\n");
                waterfall_light = false;
                waterfall_light_state = false;
                AT_RESPONSE("OK\r\n");
            }
            else
            {
                IOT_DebugLog("not find parameter \r\n");
                AT_RESPONSE("ERR\r\n");
            }
        }
        else
        {
            IOT_DebugLog("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
}


/**
 * @brief   将设备恢复出厂设置，以及向云端发送解绑设备
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-13
 */
void AT_RESTORE(char *cmd, uint16_t cmd_len)
{
    if(cmd == NULL)
    {
        IOT_ErrorLog("parameter err \r\n");
        return;
    }

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    else
    {
        AT_RESPONSE("OK\r\n");
        AilinkEnterPair();
        IOT_InfoLog("Enter Pair \r\n");
        Ailink_thread_delay(20);
        ailinkReboot();
    }
}



/**
 * @brief   将设备复位的AT指令处理函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-19
 */
void AT_RST(char *cmd, uint16_t cmd_len)
{
    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    else
    {
        AT_RESPONSE("OK\r\n");
        Ailink_thread_delay(20);
        ailinkReboot();
    }
    
}

/**
 * @brief   产测时获取MAC地址
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-11-22
 */
void AT_CIPSTAMAC(char *cmd, uint16_t cmd_len)
{
    uint8_t mac[6];
    char macstr[30]; 
    char buff[50] = {0};

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    else
    {
        /* 获取芯片上的mac地址 */
        wifi_hosal_efuse_read_mac(mac);
        memset(macstr, 0, sizeof(macstr));
        snprintf(macstr, sizeof(macstr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2], mac[3], mac[4], mac[5]);
        macstr[sizeof(macstr) - 1] = '\0';
        IOT_InfoLog("macstr = %s \r\n", macstr);
        snprintf(buff, sizeof(buff), "+CIPSTAMAC:\"%s\"\r\n", macstr);
        AT_RESPONSE(buff);
        AT_RESPONSE("OK\r\n");
    }
}


/**
 * @brief   将设置Ailink日志输出等级的AT指令处理函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-17
 */
void At_AILOG(char *cmd, uint16_t cmd_len)
{
    uint8_t level = 0;

    if(cmd == NULL)
    {
        IOT_ErrorLog("params err \r\n");
        return ;
    }

    if(cmd_len == 2)
    {
        AT_RESPONSE("OK\r\n");
        if (cmd[0] == '=')
        {
            level = atoi(&cmd[1]);
            IOT_InfoLog("level = %d \r\n", level);
            if(level == 0)
            {
                bl_sys_logall_disable();
            }
            else
            {
                bl_sys_logall_enable();
            }
            AilinkSetLogLevel(level);
        }
        else
        {
            IOT_DebugLog("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    
}

/**
 * @brief   输入待校验的md5数据，然后返回md5值
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
void AT_MD5(char *cmd, uint16_t cmd_len)
{
    static Md5Context      md5Context;
    MD5_HASH        md5Hash;

    if(cmd == NULL)
    {
        IOT_ErrorLog("params err \r\n");
        return ;
    }

    if(cmd_len > 0)
    {
        AT_RESPONSE("OK\r\n");
        if (cmd[0] == '=')
        {
            Md5Initialise( &md5Context );
            Md5Update( &md5Context, &cmd[1], strlen(&cmd[1]));              // 添加md5校验
            Md5Finalise( &md5Context, &md5Hash );
            IOT_InfoLog("md5 value: \r\n");
            for(uint8_t i=0; i<sizeof(md5Hash); i++)
            {
                printf("%02x", md5Hash.bytes[i]);
            }
            printf("\r\n");
        }
        else
        {
            IOT_DebugLog("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    
}


/**
 * @brief   将设置MCU通信模块日志输出等级的AT指令处理函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-18
 */
void At_MCULOG(char *cmd, uint16_t cmd_len)
{
    uint8_t level = 0;

    if(cmd == NULL)
    {
        IOT_ErrorLog("params err \r\n");
        return ;
    }

    if(cmd_len == 2)
    {
        AT_RESPONSE("OK\r\n");
        if (cmd[0] == '=')
        {
            level = atoi(&cmd[1]);
            IOT_InfoLog("level = %d \r\n", level);
            if(level == 0)
            {
                bl_sys_logall_disable();
            }
            else
            {
                bl_sys_logall_enable();
            }
            McuSetLogLevel(level);
        }
        else
        {
            IOT_InfoLog("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    
}


/**
 * @brief   将设置IOT业务处理模块日志输出等级的AT指令处理函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-18
 */
void At_IOTLOG(char *cmd, uint16_t cmd_len)
{
    uint8_t level = 0;

    if(cmd == NULL)
    {
        IOT_ErrorLog("params err \r\n");
        return ;
    }

    if(cmd_len == 2)
    {
        AT_RESPONSE("OK\r\n");
        if (cmd[0] == '=')
        {
            level = atoi(&cmd[1]);
            IOT_InfoLog("level = %d \r\n", level);
            if(level == 0)
            {
                bl_sys_logall_disable();
            }
            else
            {
                bl_sys_logall_enable();
            }
            IotSetLogLevel(level);
        }
        else
        {
            IOT_InfoLog("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    
}

/**
 * @brief   获取设备启动的时间点
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-18
 */
void At_GDevLTime(char *cmd, uint16_t cmd_len)
{
    char TimeBuff[200] = {0};
    AilinkRTCTimerHal_t RtcTimer = {0};

    if(cmd == NULL)
    {
        IOT_ErrorLog("parameter err \r\n");
        AT_RESPONSE("ERR\r\n");
        return;
    }

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    else
    {
        AilinkLoadTimeInfo(&RtcTimer);
        snprintf(TimeBuff, sizeof(TimeBuff), "the Date:%02d:%02d:%02d\r\nthe time:%02d:%02d:%02d\r\n", 
                RtcTimer.year, RtcTimer.mon, RtcTimer.day, RtcTimer.hour,RtcTimer.minute, RtcTimer.second);
        AT_RESPONSE(TimeBuff);
        IOT_InfoLog("response ok \r\n");
    }
}

/**
 * @brief   获取固件信息
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-25
 */
void AT_GMR(char *cmd, uint16_t cmd_len)
{
    AilinkRTCTimerHal_t RtcTimer = {0};
    iotgoDeviceInfo *IotDeviceInfo = NULL;
    char buff[100] = {0};

    if(cmd == NULL)
    {
        IOT_ErrorLog("parameter err \r\n");
        AT_RESPONSE("ERR\r\n");
        return;
    }

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    else
    {
        IotDeviceInfo = IotGetDeviceInfo();
        printf("build time: %s \r\n", BUILDTIME);
        snprintf(buff, sizeof(buff), "build time: %s\r\n", BUILDTIME);
        AT_RESPONSE(buff);
        memset(buff, 0, sizeof(buff));
        printf("fw version: %s \r\n", DEVICE_FW_VERSION);
        snprintf(buff, sizeof(buff), "fw version: %s\r\n", DEVICE_FW_VERSION);
        AT_RESPONSE(buff);
        memset(buff, 0, sizeof(buff));
        printf("userName: %s \r\n", IotDeviceInfo->userName);
        snprintf(buff, sizeof(buff), "userName: %s\r\n", IotDeviceInfo->userName);
        AT_RESPONSE(buff);
        memset(buff, 0, sizeof(buff));
        printf("userPasswd: %s \r\n", IotDeviceInfo->userPasswd);
        snprintf(buff, sizeof(buff), "userPasswd: %s\r\n", IotDeviceInfo->userPasswd);
        AT_RESPONSE(buff);
        memset(buff, 0, sizeof(buff));
        printf("deviceId: %s \r\n", IotDeviceInfo->deviceId);
        snprintf(buff, sizeof(buff), "deviceId: %s\r\n", IotDeviceInfo->deviceId);
        AT_RESPONSE(buff);
    }
}

/**
 * @brief   将三元组数据写入flash的AT指令函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-07
 */
void At_Key(char *cmd, uint16_t cmd_len)
{
    char *data = NULL;
    char *temp = NULL;
    uint16_t len = 0;

    if(cmd == NULL)
    {
        IOT_ErrorLog("parameter err \r\n");
        AT_RESPONSE("ERR\r\n");
        return;
    }

    if(cmd_len > 0)
    {
        if (cmd[0] == '=')
        {
            data = &cmd[1];
            if(data == NULL)
            {
                IOT_ErrorLog("cmd data is NULL \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            // temp = strrstr(data, "\r\n");
            // if(temp == NULL)
            // {
            //     IOT_ErrorLog("not find enter \r\n");
            //     AT_RESPONSE("ERR\r\n");
            //     return ;
            // }
            // temp[0] = '\0';
            temp= strstr(data, ",");
            if(temp == NULL)
            {
                IOT_ErrorLog("not find ',' symbol \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = temp - data;
            IOT_InfoLog("len = %d \r\n", len);

            memcpy(DeviceInfo.deviceId, data, len);
            IOT_InfoLog("DeviceInfo.deviceId = %s \r\n", DeviceInfo.deviceId);

            temp++;
            data = temp;
            temp = strstr(data, ",");
            if(temp == NULL)
            {
                IOT_ErrorLog("not find ',' symbol \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = temp - data;
            IOT_InfoLog("len = %d \r\n", len);
            memcpy(DeviceInfo.userName, data, len);
            IOT_InfoLog("DeviceInfo.userName = %s", DeviceInfo.userName);

            temp++;
            if(temp == NULL)
            {
                IOT_ErrorLog("password data err \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = strlen(temp);
            IOT_InfoLog("len = %d \r\n", len);
            memcpy(DeviceInfo.userPasswd, temp, len);
            IOT_InfoLog("DeviceInfo.userPasswd = %s", DeviceInfo.userPasswd);

            IotSaveDeviceInfo();
            AT_RESPONSE("OK\r\n");
            Ailink_thread_delay(20);
            ailinkReboot();
        }
        else
        {
            IOT_ErrorLog("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        IOT_ErrorLog("cmd length err \r\n");
        AT_RESPONSE("ERR\r\n");
    }
}

/**
 * @brief   设置串口1波特率，模组重启后生效
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void AT_UARTCFG(char *cmd, uint16_t cmd_len)
{
    uint32_t bound = 0;
    char buff[10] = {0};
    IotDeviceParams params = {0};

    if(cmd == NULL)
    {
        IOT_ErrorLog("params err \r\n");
        return ;
    }

    if(cmd_len > 2)
    {
        if (cmd[0] == '=')
        {
            memcpy(buff, &cmd[1], strlen(&cmd[1]));
            IOT_InfoLog("buff = %s \r\n", buff);
            bound = atoi(buff);
            IOT_InfoLog("bound = %d \r\n", bound);

            IotLoadDeviceParams(&params);
            params.uart_bound = bound;
            IotSaveDeviceParams(&params);

            AT_RESPONSE("OK\r\n");
        }
        else
        {
            IOT_InfoLog("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
}



/**
 * @brief   测试功能函数的AT指令
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-09
 */
void AT_Test(char *cmd, uint16_t cmd_len)
{
    char *temp = NULL;
    char target_version[20] = {0};
    char source_version[20] = {0};
    uint8_t str_len = 0;

    if(cmd == NULL)
    {
        IOT_ErrorLog("parameter err \r\n");
        return;
    }

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        IOT_ErrorLog("response err \r\n");
    }
    else
    {
        AT_RESPONSE("OK\r\n");
        IOT_InfoLog("response ok \r\n");
    }
}

/* ############################################################################################################### */
/* #################################### json 数据处理 ##############################################################*/


static char *PacketDeviceInfoToJson(void)
{
    cJSON  *json_root = NULL;
    char  *json_str = NULL;

    json_root = cJSON_CreateObject();
    if (!json_root)
    {
        IOT_ErrorLog("json create object fail \r\n");
        return NULL;
    }

    cJSON_AddStringToObject(json_root, "username", DeviceInfo.userName);
    cJSON_AddStringToObject(json_root, "password", DeviceInfo.userPasswd);
    cJSON_AddStringToObject(json_root, "deviceid", DeviceInfo.deviceId);

    json_str = cJSON_PrintUnformatted(json_root);
    if(json_str == NULL)
    {
        IOT_ErrorLog("json create str fail \r\n");
        cJSON_Delete(json_root);
        return NULL;
    }

    cJSON_Delete(json_root);
    return json_str;
}

/**
 * @brief   将设备信息数据存储于flash
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-08
 */
static void IotSaveDeviceInfo(void)
{
    char *deviceinfo = NULL;

    deviceinfo = PacketDeviceInfoToJson();
    IOT_InfoLog("deviceinfo = %s \r\n", deviceinfo);

    hosal_flash_raw_erase(DEVICE_INFO_ADDR, strlen(deviceinfo) + 1);
    hosal_flash_raw_write(deviceinfo, DEVICE_INFO_ADDR, strlen(deviceinfo));
}



/**
 * @brief   将配网数据存储于flash
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-08
 */
static void IotSaveOTAInfo(void)
{
    // IotMutexLock();
    AilinkFlashWrite(DEVICE_OTA_INFO_FLASH_BLOCK, (uint8_t *)&OtaProInfo, sizeof(ailinkOtaProInfo));
    // IotMutexUnLock();
}

/**
 * @brief   清除flash上的配网数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-08
 */
static void IotClearOTAInfo(void)
{
    // IotMutexLock();
    memset(&OtaProInfo, 0, sizeof(OtaProInfo));
    AilinkFlashWrite(DEVICE_OTA_INFO_FLASH_BLOCK, (uint8_t *)&OtaProInfo, sizeof(ailinkOtaProInfo));
    // IotMutexUnLock();
}

/**
 * @brief   从flash读取配网数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-08
 */
static void IotLoadOTAInfo(void)
{
    // IotMutexLock();
    memset(&OtaProInfo, 0, sizeof(OtaProInfo));
    AilinkFlashRead(DEVICE_OTA_INFO_FLASH_BLOCK, (uint8_t *)&OtaProInfo, sizeof(ailinkOtaProInfo));
    IOT_DebugLog("OtaProInfo.pointVer = %s \r\n", OtaProInfo.pointVer);
    IOT_DebugLog("OtaProInfo.baseVer = %s \r\n", OtaProInfo.baseVer);
    IOT_DebugLog("OtaProInfo.OtaVer = %s \r\n", OtaProInfo.OtaVer);
    IOT_DebugLog("OtaProInfo.mcuBaseVer = %s \r\n", OtaProInfo.mcuBaseVer);
    IOT_DebugLog("OtaProInfo.otaType = %s \r\n", OtaProInfo.otaType);
    IOT_DebugLog("OtaProInfo.download_url = %s \r\n", OtaProInfo.download_url);
    IOT_DebugLog("OtaProInfo.channel = %d \r\n", OtaProInfo.channel);
    // IotMutexUnLock();
}

