/**
 * @file    iot_cmd.c
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

#include "iot_cmd.h"
#include "iot_log.h"
#include "iot_protocol.h"

// #include "ailink_appair.h"
// #include "ailink_server.h"
#include "ailink_wifiHal.h"
#include "wifi_hosal.h"



#define AT_CMD_HEADER "AT"




static ATCmdInfo ATCmdInfoTab[] = 
{
    {"AT+AILOG", At_AILOG},
    {"AT+IOTLOG", At_IOTLOG},
    {"AT+MCULOG", At_MCULOG},
    {"AT+KEY", At_Key},
    {"AT+RESTORE", AT_RESTORE},
    {"AT+GDLT", At_GDevLTime},
    {"AT+RST", AT_RST},
    {"AT+NodeMCUTEST", AT_NodeMCUTEST},
    {"AT+LEDTEST", AT_LEDTEST},
    {"AT+GMR", AT_GMR},
    {"AT+MD5", AT_MD5},
    {"AT+UARTCFG", AT_UARTCFG},
    {"AT+CIPSTAMAC?", AT_CIPSTAMAC},
    {"AT", AT_Test},
    {NULL, NULL}
};


/**
 * @brief   at指令处理函数
 * 
 * @param[in]   buff 待处理的AT指令数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-12-06
 */
void atCmdExecute(char *buff)
{
    char *cmdPoint = NULL;
    uint16_t cmd_data_len = 0;
    uint16_t cmd_len = 0;
    uint16_t len = 0;
    bool cmdStatus = false;

    IOT_InfoLog("buff = %s \r\n", buff);
    if (memcmp(buff, AT_CMD_HEADER, strlen(AT_CMD_HEADER)) == 0)
    {
        cmdPoint = strstr(buff, AT_CMD_HEADER);
        IOT_InfoLog("cmdPoint = %s \r\n", cmdPoint);

        if(cmdPoint)
        {
            cmd_data_len = strlen(cmdPoint);
            for(uint16_t num = 0; ATCmdInfoTab[num].cmd != NULL; num++)
            {
                cmd_len = strlen(ATCmdInfoTab[num].cmd);
                // if(ARGV_CMDEQUAL(argv[0], ATCmdInfoTab[num].cmd))
                if(memcmp(cmdPoint, ATCmdInfoTab[num].cmd, cmd_len) == 0)
                {
                    len = (cmd_data_len - cmd_len);
                    ATCmdInfoTab[num].func((cmdPoint + cmd_len), len);
                    cmdStatus = true;
                    return;
                }
            }
            if(!cmdStatus)
            {
                IOT_ErrorLog("Can not find the cmd[%s] \r\n", cmdPoint);
                AT_RESPONSE("ERR\r\n");
            }
        }
        else
        {
            AT_RESPONSE("ERR\r\n");
            IOT_ErrorLog("cmd parse err \r\n");
        }
    }
    else
    {
        IOT_ErrorLog("Can not find the cmd[%s] \r\n", buff);
        AT_RESPONSE("ERR\r\n");
    }
}
