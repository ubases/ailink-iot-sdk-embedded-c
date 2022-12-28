/**
 * @file    mcu_system.h
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
#ifndef __MCU_SYSTEM_H
#define __MCU_SYSTEM_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/* 串口缓存最大长度， 根据MCU可支配的内存进行分配给串口缓存 */
#define  MCU_UART_BUFF_MAX_LEN              (1024)


//=============================================================================
//数据帧字段位
//=============================================================================
#define         HEAD_H                          0
#define         HEAD_L                          1        
#define         PRO_VER                         2
#define         CMD_TYPE                        3
#define         LEN_H                           4
#define         LEN_L                           5
#define         DATA_LOCAL                      6

//=============================================================================
//数据帧类型
//=============================================================================
#define         HEART_BEAT_CMD                  0                               //心跳包
#define         PRODUCT_INFO_CMD                1                               //产品信息
#define         WORK_MODE_CMD                   2                               //查询MCU 设定的模块工作模式	
#define         WIFI_STATE_CMD                  3                               //wifi工作状态	
#define         WIFI_RESET_CMD                  4                               //重置wifi
#define         WIFI_MODE_CMD                   5                               //选择ble/AP模式	
#define         DATA_QUERT_CMD                  6                               //命令下发
#define         STATE_UPLOAD_CMD                7                               //状态上报	 
#define         STATE_QUERY_CMD                 8                               //状态查询   
#define         UPDATE_START_CMD                0x0a                            //升级开始
#define         UPDATE_TRANS_CMD                0x0b                            //升级传输
#define         GET_ONLINE_TIME_CMD             0x0c                            //获取系统时间(格林威治时间)
#define         FACTORY_MODE_CMD                0x0d                            //进入产测模式    
#define         WIFI_TEST_CMD                   0x0e                            //wifi功能测试
#define         GET_LOCAL_TIME_CMD              0x1c                            //获取本地时间
#define         WEATHER_OPEN_CMD                0x20                            //打开天气          
#define         WEATHER_DATA_CMD                0x21                            //天气数据
#define         STATE_UPLOAD_SYN_CMD            0x22                            //状态上报（同步）
#define         STATE_UPLOAD_SYN_RECV_CMD       0x23                            //状态上报结果通知（同步）
#define         HEAT_BEAT_STOP                  0x25                            //关闭WIFI模组心跳
#define         STREAM_TRANS_CMD                0x28                            //流数据传输
#define         GET_WIFI_STATUS_CMD             0x2b                            //获取当前wifi联网状态
#define         WIFI_CONNECT_TEST_CMD           0x2c                            //wifi功能测试(连接指定路由)
#define         GET_MAC_CMD                     0x2d                            //获取模块mac
#define         GET_IR_STATUS_CMD               0x2e                            //红外状态通知
#define         IR_TX_RX_TEST_CMD               0x2f                            //红外进入收发产测
#define         MAPS_STREAM_TRANS_CMD           0x30                            //流数据传输(支持多张地图)
#define         FILE_DOWNLOAD_START_CMD         0x31                            //文件下载启动
#define         FILE_DOWNLOAD_TRANS_CMD         0x32                            //文件下载数据传输
#define         MODULE_EXTEND_FUN_CMD           0x34                            //模块拓展服务
#define         BLE_TEST_CMD                    0x35                            //蓝牙功能性测试（扫描指定蓝牙信标）
#define         GET_VOICE_STATE_CMD             0x60                            //获取语音状态码
#define         MIC_SILENCE_CMD                 0x61                            //MIC静音设置
#define         SET_SPEAKER_VOLUME_CMD          0x62                            //speaker音量设置
#define         VOICE_TEST_CMD                  0x63                            //语音模组音频产测
#define         VOICE_AWAKEN_TEST_CMD           0x64                            //语音模组唤醒产测
#define         VOICE_EXTEND_FUN_CMD            0x65                            //语音模组扩展功能


//=============================================================================
#define         SED_VER                  0x10                                            //模块发送帧协议版本号
#define         REV_VER                  0x20                                            //MCU 发送帧协议版本号(默认)
#define         FRAME_MINI_LEN           0x07                                            //固定协议头长度
#define         FRAME_H                  0x5a                                            //帧头高位
#define         FRAME_L                  0xa5                                            //帧头低位
//============================================================================= 

//=============================================================================
//物模型数据类型
//=============================================================================
#define         DATA_TYPE_BOOL                    0x01        //bool 类型
#define         DATA_TYPE_VALUE                   0x02        //value 类型
#define         DATA_TYPE_STRING                  0x03        //string 类型
#define         DATA_TYPE_ENUM                    0x04        //enum 类型

//=============================================================================




//=============================================================================
//WIFI工作状态
//=============================================================================
#define         BLE_CONFIG_STATE                        0x00
#define         AP_CONFIG_STATE                         0x01
#define         WIFI_NOT_CONNECTED                      0x02
#define         WIFI_CONNECTED                          0x03
#define         WIFI_CONN_CLOUD                         0x04
#define         WIFI_LOW_POWER                          0x05
#define         BLE_AND_AP_STATE                        0x06
#define         WIFI_SATE_UNKNOW                        0xff
//=============================================================================


//=============================================================================
// 命令结构
//=============================================================================
typedef struct 
{
  unsigned char cmd_id;                              //dp序号
  unsigned char cmd_type;                            //dp类型

}McuCmdInfo;



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
int8_t UartProtocolInit(void);


/**
 * @brief   获取模组与MCU的连接状态
 * 
 * @return  true    已连接
 * @return  false   未连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
bool GetMCUConnectState(void);


/**
 * @brief   获取WiFi测试启动状态
 * 
 * @return  true  测试WiFi连接已启动
 * @return  false 测试WiFi未启动
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
bool GetWifiTestState(void);


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
int8_t UartProFrameSend(uint8_t cmdType, uint8_t *data, uint16_t dataLen);


/**
 * @brief   设置WiFi网络状态
 * 
 * @param[in]   State  网络状态
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
void SetWifiNetWorkState(uint8_t State);


/**
 * @brief   获取心跳状态
 * 
 * @return  true    心跳发送多次未回复，已超时
 * @return  false   心跳正常
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
bool HeartBeatTimeout(void);


/**
 * @brief   检查MCU设备是否重新启动
 * 
 * @return  true        MCU设备已重新启动
 * @return  false       MCU设备未重新启动
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-25
 */
bool CheckMcuDeviceStart(void);

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
int8_t UartRevStream(uint8_t *data, uint16_t data_len);

/**
 * @brief   心跳包数据回应
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-23
 */
void UartSendHeatBeat(void);


/**
 * @brief   获取产品ID
 * 
 * @return  char*  产品ID数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
char *GetProductID(void);

/**
 * @brief   获取产品标识
 * 
 * @return  char*  产品标识数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-03
 */
char *GetProductFlag(void);

/**
 * @brief   获取MCU版本号
 * 
 * @return  char* MCU版本号数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
char *GetMcuVersion(void);


/**
 * @brief   向云端上报MCU上传的数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-01
 */
void McuPubliReportAttributes(void);


/**
 * @brief   该函数主要是逐一字节接收串口数据，若是串口采用缓存方式处理串口数据，可调用UartRevStream该函数处理。
 * 
 * @param[in]   data        串口数据
 * @return  int8_t         返回数据处理情况
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-22
 */
int8_t UartRevOneByte(uint8_t data);

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
int8_t UartProcessPro(void);

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
int8_t McuSendQuertCmd(uint8_t *data, uint16_t dataLen);

/**
 * @brief   获取云端发给MCU的状态
 * 
 * @return  true    数据正在发送中
 * @return  false   已发送完成
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
bool McuGetSendCmdState(void);

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
uint8_t UartCheckValue(uint8_t *dataBuff, uint16_t dataLen);


#endif
