
#ifndef __AILINK_MDNSHAL_H
#define __AILINK_MDNSHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


#define TXT_MAX_NUM                             (11)
#define TXT_KEY_MAX_LEN                         (10)
#define TXT_VALUE_MAX_LEN                       (250)




/** mdns 配置结构 */
typedef struct
{
    char *host_name;                    /**< 主机名 */                
    char *service_type;                 /**< 服务类型 */
    uint16_t port;                      /**< 端口号 */
}AilinkMdnsConfig_t;


/** 键值对结构 */
typedef struct
{
    char key[TXT_KEY_MAX_LEN];                      /**< 键名 */
    char value[TXT_VALUE_MAX_LEN];                    /**< 键值 */
}AilinkMdnsTxtItem;


/**
 * @brief   mdns初始化函数
 *
 *@note     初始化mdns,打开mdns服务。
 * 
 * @param[in]   config          mdns配置参数
 * @return  int32_t             
 *          0：初始化成功
 *          -1：初始化失败
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
int32_t AilinkMdnsInit(AilinkMdnsConfig_t config);


/**
 * @brief   mdns反初始化函数
 * @note    反初始化mdns，并释放相关资源
 * 
 * @return  int32_t 
 *          0：反初始化成功
 *          -1：反初始化失败
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
int32_t AilinkMdnsDeInit(void);


/**
 * @brief   更新txt记录函数
 * 
 * @param[in]   service_type        服务类型
 * @param[in]   protocol            协议类型，例如：_tcp,_udp等
 * @param[in]   txt                 txt键值对数组
 * @param[in]   num_items           键值对数组长度
 * @return  int32_t 
 *          0：处理成功
 *          -1：处理失败
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
int32_t AilinkMdnsUpdateTxt(char *service_type, uint16_t protocol, AilinkMdnsTxtItem txt[], uint8_t num_items);


#endif
