/**
 * @file    iot_protocol.h
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

#ifndef  __IOT_PROTOCOL_H
#define  __IOT_PROTOCOL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "ailink_ota.h"




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
void AilinkRecvControlInfo(char *data, char *mid, uint32_t ts);

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
void AilinkRecvOtaInfo(char *data, char *mid, uint32_t ts);


/**
 * @brief   事件处理函数
 * @note    循环调用该函数，时刻处理事件
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
void IotProcotolProcess(void);


/**
 * @brief   获取Ota信息数据
 * 
 * @return  ailinkOtaProInfo*   返回Ota信息数据缓存buf的指针
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-24
 */
ailinkOtaProInfo *IotGetOtaProInfo(void);


/**
 * @brief   接收ailink IOT协议栈中的事件回调函数
 * 
 * @param[in]   event        回调事件
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-16
 */
void AilinkEvent(uint8_t event);


/**
 * @brief   测试功能函数的AT指令
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-09
 */
void AT_Test(char *cmd, uint16_t cmd_len);

/**
 * @brief   将设备恢复出厂设置，以及向云端发送解绑设备
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-13
 */
void AT_RESTORE(char *cmd, uint16_t cmd_len);

/**
 * @brief   将设备复位的AT指令处理函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-19
 */
void AT_RST(char *cmd, uint16_t cmd_len);

/**
 * @brief   将三元组数据写入flash的AT指令函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-07
 */
void At_Key(char *cmd, uint16_t cmd_len);

/**
 * @brief   将设置Ailink日志输出等级的AT指令处理函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-19
 */
void At_AILOG(char *cmd, uint16_t cmd_len);

/**
 * @brief   将设置MCU通信模块日志输出等级的AT指令处理函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-18
 */
void At_MCULOG(char *cmd, uint16_t cmd_len);

/**
 * @brief   将设置IOT业务处理模块日志输出等级的AT指令处理函数
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-18
 */
void At_IOTLOG(char *cmd, uint16_t cmd_len);

/**
 * @brief   获取设备启动的时间点
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-18
 */
void At_GDevLTime(char *cmd, uint16_t cmd_len);



/**
 * @brief   获取固件信息
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-25
 */
void AT_GMR(char *cmd, uint16_t cmd_len);

/**
 * @brief   输入待校验的md5数据，然后返回md5值
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
void AT_MD5(char *cmd, uint16_t cmd_len);


/**
 * @brief   开发板测试使能
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-11-16
 */
void AT_NodeMCUTEST(char *cmd, uint16_t cmd_len);


/**
 * @brief   开发板 LED 测试指令
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-11-16
 */
void AT_LEDTEST(char *cmd, uint16_t cmd_len);


/**
 * @brief   产测时获取MAC地址
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-11-22
 */
void AT_CIPSTAMAC(char *cmd, uint16_t cmd_len);


/**
 * @brief   设置串口1波特率，模组重启后生效
 * 
 * @param[in]   cmd             AT指令参数
 * @param[in]   cmd_len         参数长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void AT_UARTCFG(char *cmd, uint16_t cmd_len);

#endif

