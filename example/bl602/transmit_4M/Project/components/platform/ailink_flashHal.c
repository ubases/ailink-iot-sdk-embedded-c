/**
 * @file    ailink_flash.c
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

#include "ailink_flashHal.h"
#include <hosal_flash.h>
#include <easyflash.h>
#include "iot_log.h"



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
int32_t AilinkFlashWrite(char *block, uint8_t *data, uint16_t data_len)
{
    int32_t ret = -1;

    ret = ef_set_env_blob(block, data, data_len);


    return ret;

}


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
int32_t AilinkFlashRead(char *block, uint8_t *buf, uint16_t buf_max_len)
{
    int32_t ret = -1;
    size_t buf_len = 0;

    ret = ef_get_env_blob(block, buf, buf_max_len, &buf_len);

    return ret;
}



