/**
 * @file    ailink_bleHal.h
 * @brief   该文件的API需有用户适配好BLE模式下必要的功能。
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
#ifndef __AILINK_BLEHAL_H
#define __AILINK_BLEHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



#define  BLE_PAIR_PROCOTOL_VERSION              1           /**< ble 配网协议版本 */


#define BLE_CONNECT_EVENT                       0           /**< ble 连接事件 */
#define BLE_DISCONNECT_EVENT                    1           /**< ble 断开连接事件 */


typedef void (*ailinkBleConnectStatus)(int32_t event);


/**
 * @brief   该函数是根据不同平台进行适配ble协议栈的初始化。即调用该函数后，
 *          该平台的ble协议栈已完成初始化。
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleStackInit(void);


/**
 * @brief   该函数是根据不同平台进行适配ble协议栈的反初始化。即调用该函数后，
 *          该平台的ble协议栈已完成反初始化，并释放ble功能占用的资源。
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleStackDeinit(void);


/**
 * @brief   该函数是根据不同平台进行适配开启ble广播功能。即调用该函数后，ble便开始
 *          广播指定设备名，并等待连接中。
 * 
 * @note    1. 该广播需设置为可连接不定向广播
 *          2. 该广播的间隔间隔时候需由用户根据需求进行设置
 * 
 * @param[in]   advName                 广播名
 * @param[in]   advName_len             广播名长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleAdvStart(char advName[], uint16_t advName_len);


/**
 * @brief   获取ble广播状态
 * 
 * @return  true ble广播已开始
 * @return  false  ble广播已停止
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
bool ailinkBleAdvStatus(void);

/**
 * @brief   该函数是根据不同平台进行适配停止ble广播功能。即调用该函数后，
 *          ble需停止广播；并释放占用的资源。
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleAdvStop(void);

/**
 * @brief   该函数是根据不同平台进行适配断开ble连接功能。即调用该函数后，
 *          设备将主动断开ble连接。
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleDisconnect(void);

/**
 * @brief   该函数将返回ble连接的状态
 * 
 * @return  true        ble已连接
 * @return  false       ble未连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
bool ailinkBleIsConnect(void);

/**
 * @brief   该函数将适配ble发送数据接口。即调用该函数，可将数据通过ble发送出去。
 * 
 * @param[in]   data      待发送的数据
 * @param[in]   data_len  待发送数据的长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleSendData(uint8_t *data, uint16_t data_len);

/**
 * @brief   该函数用于接收ble数据。
 * 
 * @param[in]   databuf             用于接收ble数据的缓存区
 * @param[in]   max_data_len        接收ble数据缓冲区的最大长度
 * @return  uint16_t                返回ble数据长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
uint16_t ailinkBleReceiveData(uint8_t *databuf, uint16_t max_data_len);



#endif