/**
 * @file    mcu_system.c
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-23
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-06-23|1.0.0  |Ai-Thinker      |创建
 */
#include "mcu_system.h"
#include "ailink_uartHal.h"
#include "mcu_process.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include "cJSON.h"
#include "ailink_osHal.h"
#include "ailink_wifiHal.h"
#include "wifi_hosal.h"
#include "ailink_timerHal.h"
#include "ailink_softtimer.h"
#include "mcu_log.h"
#include "ailink_profile.h"


#define     MCU_DATA_QUEUE_NUM_MAX                       (20)            /**< 最大的发送数据队列 */


#define     DATA_SEND_OK                                    1
#define     DATA_SEND_FAIL                                  2


#define             STR_PID                 "pid"
#define             STR_VER                 "ver"
#define             STR_FLAG                "flag"



/* 串口发送数据缓存 */
static uint8_t sedBuff[MCU_UART_BUFF_MAX_LEN] = {0};
/* 串口接收数据缓存 */
static uint8_t revBuff[MCU_UART_BUFF_MAX_LEN] = {0};
static uint8_t processBuff[MCU_UART_BUFF_MAX_LEN] = {0};
/**
 * @note 缓存MCU发送给模组的数据，其数据一个有4种数据类型，bool、int、enum和string，而这个二维数组便是存储对应的4种类型数据
 * McuPubilcBuff[0][MCU_UART_BUFF_MAX_LEN]：存储bool型数据
 * McuPubilcBuff[1][MCU_UART_BUFF_MAX_LEN]：存储int型数据
 * McuPubilcBuff[2][MCU_UART_BUFF_MAX_LEN]：存储enum数据
 * McuPubilcBuff[3][MCU_UART_BUFF_MAX_LEN]：存储string数据
 * 
 */
static uint8_t McuPubilcBuff[4][MCU_UART_BUFF_MAX_LEN] = {0};           
static uint16_t McuBoolValueLen = 0;
static uint16_t McuIntValueLen = 0;
static uint16_t McuEnumValueLen = 0;
static uint16_t McuStringValueLen = 0;
static uint32_t offset = 0;
static uint8_t  SendHeatBeatCount = 0;
static bool     mcuConnectFlag = false;
static char   productid[65] = {0};
static char   wifiFlag[65] = {0};
static char   mcuVer[10] = {0};
static uint8_t  WifiNetworkState = WIFI_NOT_CONNECTED;
static bool  TestWifiIsStart = false;
static Ailink_mutex_t metex = NULL;
static bool  DeviceIsStart = false;
static uint8_t  *revBuffPt_in = NULL;
static uint8_t  *revBuffPt_out = NULL;
static bool    McuDeviceIsStart = false;
static bool     QuertCmdState = false;



static uint16_t UartProFramePacket(uint8_t *packetBuff, uint8_t cmdType, uint8_t *data, uint16_t dataLen);
static void ParseProductInfo(uint8_t *info, uint16_t data_len);
static void WifiConnectStatus(int32_t err, ailinkWifiInfo_t *wifiInfo);
static void ResponseWifiTestState(bool state);


/**
 * @brief   信号量加锁
 * 
 * @return  int32_t 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-24
 */
static int32_t MutexLock(void)
{
    if(!metex)
    {
        metex = Ailink_mutex_new();
        if(metex == NULL)
        {
            MCU_ErrorLog("mutex create failed \r\n");
            return -1;
        }
    }

    return Ailink_mutex_lock(metex);
}

/**
 * @brief   信号量解锁
 * 
 * @return  int32_t 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-24
 */
static int32_t MutexUnLock(void)
{
    if(!metex)
    {
        metex = Ailink_mutex_new();
        if(metex == NULL)
        {
            MCU_ErrorLog("mutex create failed \r\n");
            return -1;
        }
    }

    return Ailink_mutex_unlock(metex);
}

/**
 * @brief   计算校验值，从帧头至协议内容尾字节累加求和后再对256取余
 * 
 * @param[in]   dataBuff        数据内容
 * @param[in]   dataLen         数据长度
 * @return  uint8_t             返回校验和
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-22
 */
uint8_t UartCheckValue(uint8_t *dataBuff, uint16_t dataLen)
{
    uint8_t sum = 0;
    uint16_t n = 0;

    // for(uint16_t m = 0; m < dataLen; m++)
    // {
    //     MCU_InfoLog("0x%02x ", dataBuff[m]);
    // }
    // MCU_InfoLog("\r\n");

    for(n = 0; n < dataLen; n++)
    {
        sum += dataBuff[n];
    }
    // MCU_InfoLog("sum = 0x%02x \r\n", sum);
    return sum;
}

/**
 * @brief   设置WiFi网络状态
 * 
 * @param[in]   State  网络状态
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
void SetWifiNetWorkState(uint8_t State)
{
    WifiNetworkState = State;

    UartProFrameSend(WIFI_STATE_CMD, &WifiNetworkState, sizeof(WifiNetworkState));

}

/**
 * @brief   获取模组与MCU的连接状态
 * 
 * @return  true    已连接
 * @return  false   未连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
bool GetMCUConnectState(void)
{
    return mcuConnectFlag;
}


/**
 * @brief   串口协议帧打包函数
 * 
 * @note   主要是处理将帧命令以及数据域数据，按照协议格式将数据打包在发送缓冲区中
 * 
 * @param[in]   packetBuff   发送缓冲区
 * @param[in]   cmdType       帧命令
 * @param[in]   data          数据内容
 * @param[in]   dataLen       数据内容长度
 * @return  uint16_t         返回打包好的数据长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-23
 */
static uint16_t UartProFramePacket(uint8_t *packetBuff, uint8_t cmdType, uint8_t *data, uint16_t dataLen)
{
    uint16_t packetLen = 0;

    if(packetBuff == NULL)
    {
        MCU_ErrorLog("param err \r\n");
        return 0;
    }
    packetBuff[HEAD_H] = FRAME_H;
    packetBuff[HEAD_L] = FRAME_L;
    packetBuff[PRO_VER] = SED_VER;
    packetBuff[CMD_TYPE] = cmdType;

    packetLen = dataLen + FRAME_MINI_LEN;  
    packetBuff[LEN_H] = ((dataLen & 0xff00) >> 8);
    packetBuff[LEN_L] =  (dataLen & 0xff);

    if(packetLen > MCU_UART_BUFF_MAX_LEN)
    {
        MCU_ErrorLog("data len is too big\r\n");
        return 0;
    }

    if(data != NULL)
    {
        memcpy(&packetBuff[DATA_LOCAL], data, dataLen);
    }
    packetBuff[packetLen - 1] = UartCheckValue(packetBuff, packetLen - 1);

    return packetLen;
} 

/**
 * @brief   发送协议帧数据的串口函数
 * 
 * @note    将数据域内容、帧命令通过该函数，可将数据包装成协议帧数据，然后通过串口发送出去。
 * 
 * @param[in]   cmdType     帧命令
 * @param[in]   data        数据域内容
 * @param[in]   dataLen     数据长度
 * @return  int8_t          返回数据发送情况
 *          0：发送成功
 *          -1：发送失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-23
 */
int8_t UartProFrameSend(uint8_t cmdType, uint8_t *data, uint16_t dataLen)
{
    uint16_t len = 0;
    int8_t   ret = 0;

    len = UartProFramePacket(sedBuff, cmdType, data, dataLen);
    if(len == 0)
    {
        MCU_ErrorLog("data packet err \r\n");
        return -1;
    }

    MutexLock();
    ret = AilinkUart1Send(sedBuff, len);
    MutexUnLock();

    memset(sedBuff, 0, MCU_UART_BUFF_MAX_LEN);

    return ret;
}

/**
 * @brief   获取产品ID
 * 
 * @return  char*  产品ID数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
char *GetProductID(void)
{
    return productid;
}


/**
 * @brief   获取产品标识
 * 
 * @return  char*  产品标识数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-03
 */
char *GetProductFlag(void)
{
    return wifiFlag;
}

/**
 * @brief   获取MCU版本号
 * 
 * @return  char* MCU版本号数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
char *GetMcuVersion(void)
{
    return mcuVer;
}

/**
 * @brief   处理MCU发送的产品信息是否齐全
 * 
 * @return  true           产品信息未接收齐全
 * @return  false          产品信息已接收齐全
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
static bool ProductInfoIsEmpty(void)
{
    if(strlen(productid) == 0 || strlen(wifiFlag) == 0 || strlen(mcuVer) == 0)
    {
        return true;
    } 

    return false;
}

/**
 * @brief   解析产品信息
 * 
 * @param[in]   info            从MCU上报的json字符串
 * @param[in]   data_len        json字符串数据长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-24
 */
static void ParseProductInfo(uint8_t *info, uint16_t data_len)
{
    char *buff = NULL;
    cJSON  *cjson_root = NULL;
    cJSON  *cjson_value = NULL;
    char  *str_value = NULL;

    buff = malloc(data_len + 1);
    if(buff == NULL)
    {
        MCU_ErrorLog("malloc fail \r\n");
        return ;
    }
    memset(buff, 0, data_len + 1);
    memcpy(buff , info, data_len);
    MCU_InfoLog("buff = %s \r\n", buff);

    cjson_root = cJSON_Parse(buff);
    if(cjson_root == NULL)
    {
        MCU_ErrorLog("json parse err \r\n");
        return ;
    }

    cjson_value = cJSON_GetObjectItem(cjson_root, STR_PID);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(productid, str_value, strlen(str_value));
            MCU_InfoLog("str_value = %s \r\n", str_value);
        }
        str_value = NULL;
    }
    cjson_value = cJSON_GetObjectItem(cjson_root, STR_VER);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(mcuVer, str_value, strlen(str_value));
            MCU_InfoLog("str_value = %s \r\n", str_value);
        }
        str_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_root, STR_FLAG);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(wifiFlag, str_value, strlen(str_value));
            MCU_InfoLog("str_value = %s \r\n", str_value);
        }
        str_value = NULL;
    }

    free(buff);
    cJSON_Delete(cjson_root);
}


/**
 * @brief   该函数主要是逐一字节接收串口数据，若是串口采用缓存方式处理串口数据，可调用UartRevStream该函数处理。
 * 
 * @param[in]   data        串口数据
 * @return  int8_t         返回数据处理情况
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-22
 */
int8_t UartRevOneByte(uint8_t data)
{
    if(revBuffPt_out - revBuffPt_in == 1)
    {
        MCU_ErrorLog("buff is full \r\n");
    }
    else if((revBuffPt_in > revBuffPt_out) && ((revBuffPt_in - revBuffPt_out) >= sizeof(revBuff)))
    {
        MCU_ErrorLog("buff is full \r\n");
    }
    else
    {
        if(revBuffPt_in >= (uint8_t *)(revBuff + sizeof(revBuff)))
        {
            revBuffPt_in = (uint8_t *)(revBuff);
        }

        *revBuffPt_in++ = data;
    }

    return 0;
}


/**
 * @brief   接收串口缓存数据
 * 
 * @param[in]   data            待处理的串口数据
 * @param[in]   data_len        串口数据大小
 * @return  int8_t             返回数据处理情况
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-22
 */
int8_t UartRevStream(uint8_t *data, uint16_t data_len)
{
    uint16_t n = 0;

    if(data == NULL)
    {
        MCU_ErrorLog("param err \r\n");
        return -1;
    }
    for(n = 0; n < data_len; n++)
    {
        UartRevOneByte(data[n]);
    }

    return 0;
}

/**
 * @brief   串口协议初始化
 * 
 * @note    该函数需在mcu初始化时调用该函数初始化串口处理。
 *  
 * @return  int8_t      返回初始化状态 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-22
 */
int8_t UartProtocolInit(void)
{
    memset(revBuff, 0, sizeof(revBuff));

    revBuffPt_in = (uint8_t *)revBuff;
    revBuffPt_out = (uint8_t *)revBuff;

    return 0;
}

/**
 * @brief   检查MCU设备是否重新启动
 * 
 * @return  true        MCU设备已重新启动
 * @return  false       MCU设备未重新启动
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-25
 */
bool CheckMcuDeviceStart(void)
{
    return McuDeviceIsStart;
}


/**
 * @brief   心跳包数据回应
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-23
 */
void UartSendHeatBeat(void)
{
    UartProFrameSend(HEART_BEAT_CMD, NULL, 0);
    if(mcuConnectFlag)
    {
        SendHeatBeatCount++;
        MCU_InfoDebug("SendHeatBeatCount = %d \r\n", SendHeatBeatCount);
    }
}


/**
 * @brief   获取心跳状态
 * 
 * @return  true    心跳发送多次未回复，已超时
 * @return  false   心跳正常
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
bool HeartBeatTimeout(void)
{
    if(SendHeatBeatCount > 20)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief   向MCU回复模组的MAC地址
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-24
 */
static void UartSendMac(void)
{
    uint8_t dataBuf[7] = {0};

    dataBuf[0] = 0;

    wifi_hosal_efuse_read_mac(&dataBuf[1]);
    printf("mac :\r\n");
    for(uint8_t n = 0; n < 7; n++)
    {
        printf("%02x", dataBuf[n]);
    }
    printf("\r\n");
    UartProFrameSend(GET_MAC_CMD, dataBuf, sizeof(dataBuf));
}

/**
 * @brief   向MCU回复本地时间
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-24
 */
static void UartSendLocalTime(void)
{
    AilinkRTCTimerHal_t *timer = NULL;
    uint8_t dataBuf[8] = {0};

    timer = AilinkHalGetRTCTime();
    if(timer == NULL)
    {
        MCU_ErrorLog("timer is NULL \r\n");
        return ;
    }

    dataBuf[0] = 1;
    dataBuf[1] = (timer->year - 2000); 
    dataBuf[2] = timer->mon;
    dataBuf[3] = timer->day;
    dataBuf[4] = timer->hour;
    dataBuf[5] = timer->minute;
    dataBuf[6] = timer->second;
    dataBuf[7] = timer->week;

    UartProFrameSend(GET_LOCAL_TIME_CMD, dataBuf, sizeof(dataBuf));

}

/**
 * @brief   模组向MCU下发命令数据
 * 
 * @param[in]   data        待下发的命令数据
 * @param[in]   dataLen     数据长度
 * @return  int8_t 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
int8_t McuSendQuertCmd(uint8_t *data, uint16_t dataLen)
{
    QuertCmdState = true;
    return UartProFrameSend(DATA_QUERT_CMD, data, dataLen);
}


/**
 * @brief   获取云端发给MCU的状态
 * 
 * @return  true    数据正在发送中
 * @return  false   已发送完成
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
bool McuGetSendCmdState(void)
{
    return QuertCmdState;
}


static void ParseWifiInfo(uint8_t *wifi_info, uint16_t info_len)
{
    char *buff = NULL;
    cJSON  *cjson_root = NULL;
    cJSON  *cjson_value = NULL;
    char  *str_value = NULL;
    char  ssid[33] = {0};
    char  passwd[65] = {0};
    uint8_t status = 0;

    buff = malloc(info_len + 1);
    if(buff == NULL)
    {
        MCU_ErrorLog("malloc fail \r\n");
        ResponseWifiTestState(false);
        return ;
    }
    memset(buff, 0, info_len + 1);
    memcpy(buff , wifi_info, info_len);
    MCU_InfoLog("buff = %s \r\n", buff);

    cjson_root = cJSON_Parse(buff);
    if(cjson_root == NULL)
    {
        MCU_ErrorLog("json parse err \r\n");
        ResponseWifiTestState(false);
        return ;
    }

    cjson_value = cJSON_GetObjectItem(cjson_root, "ssid");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(ssid, str_value, strlen(str_value));
            status = 1;
            MCU_InfoLog("ssid = %s \r\n", ssid);
        }
        str_value = NULL;
    }
    cjson_value = cJSON_GetObjectItem(cjson_root, "password");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(passwd, str_value, strlen(str_value));
            status = 2;
            MCU_InfoLog("passwd = %s \r\n", passwd);
        }
        str_value = NULL;
    }

    if(status == 2)
    {
        ailinkWifiStaInit(WifiConnectStatus);
        Ailink_thread_delay(2000);
        ailinkWifiConnect(ssid, passwd);
        TestWifiIsStart = true;
        ResponseWifiTestState(true);
    }
    else
    {
        ResponseWifiTestState(false);
    }

    free(buff);
    cJSON_Delete(cjson_root);
}

/**
 * @brief   获取WiFi测试启动状态
 * 
 * @return  true  测试WiFi连接已启动
 * @return  false 测试WiFi未启动
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
bool GetWifiTestState(void)
{
    return TestWifiIsStart;
}

/**
 * @brief   回复测试WiFi连接情况
 * 
 * @param[in]   state
 *              true：WiFi已连接成功
 *              false: WiFi未连接成功
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
static void ResponseWifiTestState(bool state)
{
    bool buff = state;

    UartProFrameSend(WIFI_CONNECT_TEST_CMD, (uint8_t *)&buff, sizeof(buff));
    TestWifiIsStart = false;
}




int8_t ProcessReportDataInfo(uint8_t *data, uint16_t data_len)
{
    uint8_t cmd_id = 0;
    uint8_t cmd_type = 0;
    uint8_t ret = 0;
    uint8_t cmd_len = 0;
    uint16_t count = 0;
    uint32_t  value = 0;

    if(data == NULL)
    {
        MCU_ErrorLog("param err \r\n");
        return -1;
    }

    // MCU_InfoLog("data_len = %d \r\n", data_len);
    // for(uint16_t n = 0; n < data_len; n++)
    // {
    //     printf("0x%02x ", data[n]);
    // }
    // printf("\r\n");

    MutexLock();
    for(count = 0; count < data_len; )
    {
        cmd_id = data[count + 0];
        cmd_type = data[count + 1];
        cmd_len = ((data[count + 2] << 8) | data[count + 3]);

        MCU_InfoLog("cmd_id = %d \r\n", cmd_id);
        MCU_InfoLog("cmd_type = %d \r\n", cmd_type);
        MCU_InfoLog("cmd_len = %d \r\n", cmd_len);

        if(cmd_type == DATA_TYPE_BOOL)
        {
            MCU_InfoLog("value = %d \r\n", data[count + 4]);
            McuPubilcBuff[0][McuBoolValueLen + 0] = cmd_id;
            McuPubilcBuff[0][McuBoolValueLen + 1] = data[count + 4];
            McuBoolValueLen += 2;
        }
        else if(cmd_type == DATA_TYPE_VALUE)
        {
            if(cmd_len > 2)
            {
                value = ((data[count + 4] << 24)  | 
                        (data[count + 5] << 16)  | 
                        (data[count + 6] << 8)  |
                        data[count + 7]);
                MCU_InfoLog("value = %ld \r\n", value);
                McuPubilcBuff[1][McuIntValueLen + 0] = cmd_id;
                McuPubilcBuff[1][McuIntValueLen + 1] = data[count + 4];
                McuPubilcBuff[1][McuIntValueLen + 2] = data[count + 5];
                McuPubilcBuff[1][McuIntValueLen + 3] = data[count + 6];
                McuPubilcBuff[1][McuIntValueLen + 4] = data[count + 7];
                McuIntValueLen += 5;
            }
            else
            {
                value = ((data[count + 4] << 8) | data[count + 5]);
                MCU_InfoLog("value = %ld \r\n", value);
                McuPubilcBuff[1][McuIntValueLen + 0] = cmd_id;
                McuPubilcBuff[1][McuIntValueLen + 1] = 0;
                McuPubilcBuff[1][McuIntValueLen + 2] = 0;
                McuPubilcBuff[1][McuIntValueLen + 3] = data[count + 4];
                McuPubilcBuff[1][McuIntValueLen + 4] = data[count + 5];
                McuIntValueLen += 5;
            }

        }
        else if(cmd_type == DATA_TYPE_STRING)
        {
            MCU_InfoLog("value = %s \r\n", &data[count + 4]);
            McuPubilcBuff[3][McuStringValueLen + 0] = cmd_id;
            McuPubilcBuff[3][McuStringValueLen + 1] = ((cmd_len & 0xff00) >> 8);
            McuPubilcBuff[3][McuStringValueLen + 2] = (cmd_len & 0xff);
            memcpy(&McuPubilcBuff[3][McuStringValueLen + 3], &data[count + 4], cmd_len);
            McuStringValueLen += cmd_len;
            McuStringValueLen += 3;
            MCU_InfoLog("McuStringValueLen = %d \r\n", McuStringValueLen);
        }
        else if(cmd_type == DATA_TYPE_ENUM)
        {
            MCU_InfoLog("value = %d \r\n", data[count + 4]);
            McuPubilcBuff[2][McuEnumValueLen + 0] = cmd_id;
            McuPubilcBuff[2][McuEnumValueLen + 1] = data[count + 4];
            McuEnumValueLen += 2;
        }
        else
        {
            ret = -1;
            MCU_ErrorLog("cmd type err \r\n");
        }

        count += (cmd_len + 4);
    }
    MutexUnLock();

    return ret;
}


/**
 * @brief   向云端上报MCU上传的数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-01
 */
void McuPubliReportAttributes(void)
{
    cJSON *json_control = NULL;
    uint16_t num = 0;
    char numBuff[10] = {0};
    uint32_t value = 0;
    uint16_t stringLen = 0;
    char *stringBuff = NULL;
    int32_t ret = 0;

    if(McuBoolValueLen == 0 && McuIntValueLen == 0 && McuEnumValueLen == 0 && McuStringValueLen ==0)
    {
        MCU_ErrorLog("param err \r\n");
        return;
    }

    json_control = cJSON_CreateObject();
    if (!json_control)
    {
        MCU_ErrorLog("json create object fail \r\n");
        return ;
    }

    MutexLock();
    if(McuBoolValueLen > 0)
    {
        for(num = 0; num < McuBoolValueLen; num += 2)
        {
            snprintf(numBuff, sizeof(numBuff), "%d", McuPubilcBuff[0][num]);
            if(McuPubilcBuff[0][num + 1])
            {
                cJSON_AddTrueToObject(json_control, numBuff);
            }
            else
            {
                cJSON_AddFalseToObject(json_control, numBuff);
            }
            memset(numBuff, 0, sizeof(numBuff));
        }
    }

    if(McuIntValueLen > 0)
    {
        for(num = 0; num < McuIntValueLen; num += 5)
        {
            snprintf(numBuff, sizeof(numBuff), "%d", McuPubilcBuff[1][num]);

            value = (uint32_t)McuPubilcBuff[1][num + 1];
            value <<= 8;
            value |= (uint32_t)McuPubilcBuff[1][num + 2];
            value <<= 8;
            value |= (uint32_t)McuPubilcBuff[1][num + 3];
            value <<= 8;
            value |= (uint32_t)McuPubilcBuff[1][num + 4];

            cJSON_AddNumberToObject(json_control, numBuff, value);
            memset(numBuff, 0, sizeof(numBuff));
        }
    }

    if(McuEnumValueLen > 0)
    {
        for(num = 0; num < McuEnumValueLen; num += 2)
        {
            snprintf(numBuff, sizeof(numBuff), "%d", McuPubilcBuff[2][num]);
            cJSON_AddNumberToObject(json_control, numBuff, McuPubilcBuff[2][num + 1]);
            memset(numBuff, 0, sizeof(numBuff));
        }
    }

    if(McuStringValueLen > 0)
    {
        for(num = 0; num < McuStringValueLen; )
        {
            stringLen =  ((McuPubilcBuff[3][num + 1] << 8) | McuPubilcBuff[3][num + 2]);
            MCU_InfoLog("stringLen = %d \r\n", stringLen);
            MCU_InfoLog("num = %d \r\n", num);
            MCU_InfoLog("McuStringValueLen = %d \r\n", McuStringValueLen);
            if(stringLen > 0)
            {
                MCU_InfoLog("id = %d \r\n", McuPubilcBuff[3][num]);
                MCU_InfoLog("string value = %s \r\n", &McuPubilcBuff[3][num + 3]);
                snprintf(numBuff, sizeof(numBuff), "%d", McuPubilcBuff[3][num]);
                stringBuff = malloc(stringLen + 1);       
                if(stringBuff)
                {
                    memset(stringBuff, 0, stringLen+1);
                    memcpy(stringBuff, &McuPubilcBuff[3][num + 3], stringLen);
                    MCU_InfoDebug("stringBuff = %s \r\n", stringBuff);
                    MCU_InfoDebug("cmd_id = %s \r\n", numBuff);
                    cJSON_AddStringToObject(json_control, numBuff, stringBuff);
                }
                else
                {
                    MCU_ErrorLog("malloc fail \r\n");
                }
            }
            num += stringLen;
            num += 3;
            memset(numBuff, 0, sizeof(numBuff));
        }
    }
    MutexUnLock();

    
    ret = AilinkPublicReportAttributes(json_control);
    if(ret != 0)
    {
        cJSON_Delete(json_control);
    }

    memset(McuPubilcBuff[0], 0, sizeof(McuPubilcBuff[0]));
    memset(McuPubilcBuff[1], 0, sizeof(McuPubilcBuff[1]));
    memset(McuPubilcBuff[2], 0, sizeof(McuPubilcBuff[2]));
    memset(McuPubilcBuff[3], 0, sizeof(McuPubilcBuff[3]));
    McuBoolValueLen = 0;
    McuIntValueLen = 0;
    McuEnumValueLen = 0;
    McuStringValueLen = 0;
}



/**
 * @brief   数据帧处理，执行帧命令
 * 
 * @param[in]   DataCmd     数据帧
 * @param[in]   data_len    数据帧长度
 * @return  uint8_t         返回执行结果
 *          0：执行成功
 *          -1：执行失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-22
 */
uint8_t ProcessUartCmd(uint8_t *DataCmd, uint16_t data_len)
{
    uint8_t cmdType = 0;
    uint16_t len = 0;
    uint16_t count = 0;
    uint16_t dp_len = 0;

    if(DataCmd == NULL)
    {
        MCU_ErrorLog("param err \r\n");
        return -1;
    }

    cmdType = DataCmd[CMD_TYPE];

    MCU_InfoLog("###################### receive ######################\r\n");
    MCU_InfoLog("cmdType = %d \r\n", cmdType);
    for(uint16_t n = 0; n < data_len; n++)
    {
        MCU_InfoLog("0x%02x", DataCmd[n]);
    }
    MCU_InfoLog("###################### end ######################\r\n");

    switch (cmdType)
    {
        case HEART_BEAT_CMD:
        {
            MCU_InfoLog("HEART_BEAT_CMD \r\n");
            if((DataCmd[DATA_LOCAL] == 0) || !DeviceIsStart)
            {
                MCU_InfoLog("send PRODUCT_INFO_CMD \r\n");
                UartProFrameSend(PRODUCT_INFO_CMD, NULL, 0);
                DeviceIsStart = true;
                
                if(DataCmd[DATA_LOCAL] == 0)
                {
                    memset(productid, 0, sizeof(productid));
                    memset(wifiFlag, 0, sizeof(wifiFlag));
                    memset(mcuVer, 0, sizeof(mcuVer));
                    McuDeviceIsStart = true;
                }

                if(!mcuConnectFlag)
                {
                    MCU_InfoLog("change heart beat time \r\n");
                    SetHeartBeatTimeout(15 * 1000);
                    mcuConnectFlag = true;
                }
            }
            SendHeatBeatCount = 0;
            MCU_InfoLog("###################### end ######################\r\n");
        }
        break;

        case WORK_MODE_CMD:
        {
            MCU_InfoLog("WORK_MODE_CMD \r\n");
            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);

            MCU_InfoLog("len = %d \r\n", len);
            if(len > 0)
            {
                MCU_InfoLog("send STATE_QUERY_CMD \r\n");
                UartProFrameSend(STATE_QUERY_CMD, NULL, 0);
            }
            else
            {
                MCU_InfoLog("send WIFI_STATE_CMD \r\n");
                UartProFrameSend(WIFI_STATE_CMD, &WifiNetworkState, sizeof(WifiNetworkState));
            }
        }
        break;

        case WIFI_STATE_CMD:
        {
            MCU_InfoLog("WIFI_STATE_CMD \r\n");
            MCU_InfoLog("send STATE_QUERY_CMD \r\n");
            UartProFrameSend(STATE_QUERY_CMD, NULL, 0);
        }
        break;

        case STATE_UPLOAD_CMD:
        {
            MCU_InfoLog("STATE_UPLOAD_CMD \r\n");
            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);
            MCU_InfoLog("len = %d \r\n", len);

            ProcessReportDataInfo(&DataCmd[DATA_LOCAL], len);
            QuertCmdState = false;
            SetPublicDataTimeout(50);

        }
        break;

        case WIFI_RESET_CMD:
        {
            MCU_InfoLog("WIFI_RESET_CMD \r\n");
            if(mcuConnectFlag)
            {
                UartProFrameSend(WIFI_RESET_CMD, NULL, 0);
                AilinkEnterPair();
                MCU_InfoDebug("reset wifi module, and enter pair\r\n");
                Ailink_thread_delay(20);
                ailinkReboot();
            }
        }
        break;

        case WIFI_MODE_CMD:
        {
            MCU_InfoLog("WIFI_MODE_CMD \r\n");
            if(mcuConnectFlag)
            {
                UartProFrameSend(WIFI_MODE_CMD, NULL, 0);
                AilinkEnterPair();
                MCU_InfoDebug("reset wifi module, and enter pair\r\n");
                Ailink_thread_delay(20);
                ailinkReboot();
            }
        }
        break;

        case DATA_QUERT_CMD:
        {
            MCU_InfoLog("DATA_QUERT_CMD \r\n");
        }
        break;

        case GET_MAC_CMD:
        {
            MCU_InfoLog("GET_MAC_CMD \r\n");
            UartSendMac();
        }
        break;

        case GET_WIFI_STATUS_CMD:
        {
            MCU_InfoLog("GET_WIFI_STATUS_CMD \r\n");
            UartProFrameSend(WIFI_STATE_CMD, &WifiNetworkState, sizeof(WifiNetworkState));
        }
        break;

        case UPDATE_START_CMD:
        {
            MCU_InfoLog("UPDATE_START_CMD \r\n");
        }
        break;

        case UPDATE_TRANS_CMD:
        {
            MCU_InfoLog("UPDATE_TRANS_CMD \r\n");
        }
        break;

        case PRODUCT_INFO_CMD:
        {
            MCU_InfoLog("PRODUCT_INFO_CMD \r\n");

            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);

            MCU_InfoLog("len = %d \r\n", len);
            
            ParseProductInfo(&DataCmd[DATA_LOCAL], len);

            if(!ProductInfoIsEmpty())
            {
                MCU_InfoLog("send WORK_MODE_CMD \r\n");
                UartProFrameSend(WORK_MODE_CMD, NULL, 0);
                McuDeviceIsStart = false;
            }
        }
        break;

        case GET_LOCAL_TIME_CMD:
        {
            MCU_InfoLog("GET_LOCAL_TIME_CMD \r\n");
            UartSendLocalTime();
        }
        break;

        case WIFI_TEST_CMD:
        {
            MCU_InfoLog("WIFI_TEST_CMD \r\n");
        }
        break;

        case WIFI_CONNECT_TEST_CMD:
        {
            MCU_InfoLog("WIFI_CONNECT_TEST_CMD \r\n");

            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);
            MCU_InfoLog("len = %d \r\n", len);
            if(mcuConnectFlag)
            {
                ParseWifiInfo(&DataCmd[DATA_LOCAL], len);
            }
            
        }
        break;

        default:
            break;
    }


    return 0;
}



/**
 * @brief   判断接收缓冲区是否为空
 * 
 * @return  true  接收缓冲区为空
 * @return  false 接收缓冲区不为空
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-28
 */
bool revBuffIsEmpty(void)
{
    if(revBuffPt_in != revBuffPt_out)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * @brief   从接收缓冲区获取数据
 * 
 * @return  uint8_t  已获取到的数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-28
 */
uint8_t TakeDataFromBuff(void)
{
    uint8_t value = 0;

    if(revBuffPt_in != revBuffPt_out)
    {
        //有数据
        if(revBuffPt_out >= (uint8_t *)(revBuff + sizeof(revBuff))) {
            //数据已经到末尾
            revBuffPt_out = (uint8_t *)(revBuff);
        }
        
        value = *revBuffPt_out ++;   
    }

    return value;
}


/**
 * @brief   串口数据处理
 * 
 * @note    该函数需在while循环中循环调用
 * 
 * @return  int8_t 返回处理结果
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-22
 */
int8_t UartProcessPro(void)
{
    uint16_t dataLen = 0;
    uint16_t revLen = 0;
    uint16_t offset = 0;

    while ((revLen < sizeof(revBuff)) && (!revBuffIsEmpty()))
    {
        processBuff[revLen++] = TakeDataFromBuff();
    }
    
    if(revLen < FRAME_MINI_LEN)
    {
        // MCU_InfoLog("frame len[%d] err \r\n", revLen);
        return -1;
    }

    while ((revLen - offset) >= FRAME_MINI_LEN)
    {
        // MCU_InfoLog("revLen = %d \r\n", revLen);
        if(processBuff[offset + HEAD_H] != FRAME_H)
        {
            offset++;
            MCU_ErrorLog("frame head err \r\n");
            continue;
        }

        if(processBuff[offset + HEAD_L] != FRAME_L)
        {
            offset++;
            MCU_ErrorLog("frame head err \r\n");
            continue;
        }


        if(processBuff[offset + PRO_VER] != REV_VER )
        {
            offset += 2;
            MCU_ErrorLog("frame ver err \r\n");
            continue;
        }

        dataLen = ((processBuff[offset + LEN_H] << 8) | processBuff[offset + LEN_L]);
        dataLen += FRAME_MINI_LEN;
        if(dataLen > (sizeof(processBuff) + FRAME_MINI_LEN))
        {
            offset += 3;
            MCU_ErrorLog("frame len err \r\n");
            continue;
        }

        if((revLen - offset) < dataLen)
        {
            MCU_ErrorLog("revLen = %d \r\n", revLen);
            MCU_ErrorLog("offset = %d \r\n", offset);
            MCU_ErrorLog("dataLen = %d \r\n", dataLen);
            MCU_ErrorLog("frame len err \r\n");
            return 1;
        }

        if(processBuff[offset + dataLen - 1] != UartCheckValue(&processBuff[offset], dataLen-1))
        {
            offset += 3;
            MCU_ErrorLog("frame check err \r\n");
            continue;
        }
        ProcessUartCmd(&processBuff[offset], dataLen);
        offset += dataLen;
    }
    
    revLen -= offset;
    if(revLen > 0)
    {
        MCU_InfoLog("revLen = %d \r\n", revLen);
        memcpy(processBuff, &processBuff[offset], revLen);
    }




    return 0;
}




/**
 * @brief   WiFi功能回调函数
 * 
 * @note    当启动STA或AP的WiFi功能时，将会注册该函数；并且，WiFi协议栈运行过程中会将WiFi的各种功能事件
 *          通过该函数进行输出。
 * 
 * @param[in]   err             WiFi功能错误码事件
 * @param[in]   wifiInfo        WiFi网络信息
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-12
 */
static void WifiConnectStatus(int32_t err, ailinkWifiInfo_t *wifiInfo)
{
    switch (err)
    {
        case AILINK_WIFI_ON_INIT_DONE:
        {
            MCU_InfoLog("AILINK_WIFI_ON_INIT_DONE \r\n");
        }
        break;

        case AILINK_WIFI_ON_DISCONNECT:
        {
            MCU_InfoLog("AILINK_WIFI_ON_DISCONNECT \r\n");
        }
        break;

        case AILINK_WIFI_ON_CONNECTED:
        {
            MCU_InfoLog("AILINK_WIFI_ON_CONNECTED \r\n");
        }
        break;

        case AILINK_WIFI_ON_GOT_IP:
        {
             MCU_InfoLog("AILINK_WIFI_ON_GOT_IP \r\n");
        }
        break;

        case AILINK_WIFI_ON_AP_STARTED:
        {
            MCU_InfoLog("AILINK_WIFI_ON_AP_STARTED \r\n");
        }
        break;

        case AILINK_WIFI_ON_AP_STOPPED:
        {
             MCU_InfoLog("AILINK_WIFI_ON_AP_STOPPED \r\n");
        }
        break;

        
        default:
            break;
    }
}


