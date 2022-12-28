/**
 * @file    ailink_flash.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-20
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-20|1.0.0  |Ai-Thinker     |创建
 */
#ifndef __AILINK_FLASHHAL_H
#define __AILINK_FLASHHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



#define     AILINK_PAIR_FLAG_BLOCK                  "pair_flag"
#define     AILINK_SSID_BLOCK                       "ssid"
#define     AILINK_PASSWD_BLCK                      "passwd"






/**
 * @brief   向flash写入数据进行存储
 * 
 * @param[in]   block           数据存储key
 * @param[in]   data            待写入的数据
 * @param[in]   data_len        数据长度
 * @return  int32_t 
 *          0：写入成功
 *          -1：写入失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-20
 */
int32_t AilinkFlashWrite(char *block, uint8_t *data, uint16_t data_len);


/**
 * @brief   从flash读取数据
 * 
 * @param[in]   block           数据存储key
 * @param[in]   buf             缓存buf
 * @param[in]   buf_max_len     缓存buf的最大空间
 * @return  int32_t 
 *          0：读取成功
 *          -1：读取失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-20
 */
int32_t AilinkFlashRead(char *block, uint8_t *buf, uint16_t buf_max_len);





#endif
