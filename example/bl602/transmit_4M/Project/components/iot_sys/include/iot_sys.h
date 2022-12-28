/**
 * @file    iot_sys.h
 * @brief   项目逻辑业务函数的声明和宏定义
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
#ifndef __IOT_SYS_H
#define __IOT_SYS_H
#include <stdint.h>


#define DEVICE_LOG_ENABLE


#define DEVICE_INFO_ADDR            (0x315000)
#define DEVICE_FLAG                 "AIR"
#define DEVICE_PRODUCT_ID           ("bl602_transmit_4M")
#define DEVICE_MDNS_SERVER_TYPE     ("_iotaithings")



#define DEVICE_FW_VERSION           ("1.0.70")
#define DEVICE_MCU_VERSION          ("1.0.0")
#define DEVICE_TYPE                 ("bl602")


#define _MACROSTR(x) #x
#define MACROSTR(x) _MACROSTR(x)
#define BUILDTIME MACROSTR(XUE_COMPILE_TIME)



typedef struct 
{
    char userName[65];
    char userPasswd[65];
    char deviceId[64];

}iotgoDeviceInfo;


typedef struct 
{
    uint32_t uart_bound;            /**< 设置设备上串口1的波特率 */
    uint8_t  work_mode;             /**< 设置设备的工作模式，是AT指令的工作模式，还是MCU SDK的工作模式*/

}IotDeviceParams;



void iotgo_sys_entry(void);


/**
 * @brief   获取设备的三元组数据
 * 
 * @return  iotgoDeviceInfo* 三元组数据结构
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-10-27
 */
iotgoDeviceInfo *IotGetDeviceInfo(void);


/**
 * @brief   存储设备参数数据
 * 
 * @param[in]   params      待存储数据参数
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void IotSaveDeviceParams(IotDeviceParams *params);

/**
 * @brief   清除设备存储的参数数据
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void IotClearDeviceParams(void);


/**
 * @brief   读取设备存储的参数数据
 * 
 * @param[in]   params  已读取的参数数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void IotLoadDeviceParams(IotDeviceParams *params);


#endif
