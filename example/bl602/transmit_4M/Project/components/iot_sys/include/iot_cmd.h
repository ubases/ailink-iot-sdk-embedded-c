/**
 * @file    iot_cmd.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-02
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-09-02|1.0.0  |Ai-Thinker      |创建
 */

#ifndef __IOT_CMD_H
#define __IOT_CMD_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include "iot_uart.h"


#define CMD_ARGC_MAX    10



/*命令处理回调函数指针*/
typedef void (*Cmd_CallBack)(int argc, char *argv[]);
typedef void (*Cmd_ProcessCb)(char *cmd, uint16_t cmd_len);

/*命令处理入口*/
typedef struct CmdEntrance
{
    char const *cmd;
    Cmd_CallBack pCB;
    struct CmdEntrance *next;

} CmdEntrance_t;


typedef struct
{
    char *cmd;
    Cmd_ProcessCb func;
} ATCmdInfo;



#define         CMD_LOG_ENABLE                  0
#define         CMD_RESPONSE_ENABLE             1


#define AT_RESPONSE(x)\
				do{\
						if(CMD_RESPONSE_ENABLE)\
						{\
								if(CMD_LOG_ENABLE)\
									printf("[IOT Info : %s, %d, %s] ",  filename(__FILE__), __LINE__,__FUNCTION__);\
								IotUartSend((uint8_t *)x, strlen(x));\
						}\
					}while(0)

#define DEVICE_INFO_OUTPUT(x)\
				do{\
						if(CMD_RESPONSE_ENABLE)\
						{\
								if(CMD_LOG_ENABLE)\
									printf("[IOT Info : %s, %d, %s] ",  filename(__FILE__), __LINE__,__FUNCTION__);\
								IotUartSend((uint8_t *)x, strlen(x));\
						}\
					}while(0)

#define CMD_ENT_DEF(cmd, ent) \
    static CmdEntrance_t CmdEntrance_##cmd = {#cmd, ent, NULL}

#define CMD_ENT(cmd)  \
        &CmdEntrance_##cmd

/*参数比较*/
#define ARGV_EQUAL(token)       (strcmp(argv[0], (char*)token) == 0)
#define ARGV_LEGAL(argv)        (argv != NULL && isalnum(argv[0]))
#define ARGV_CMDEQUAL(token, cmd)           (memcmp(token, cmd, strlen(cmd)) == 0)



/**
 * @brief   at指令处理函数
 * 
 * @param[in]   buff 待处理的AT指令数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void atCmdExecute(char *buff);




#endif
