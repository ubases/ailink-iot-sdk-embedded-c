/**
 * @file    mcu_process.c
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
#include "mcu_process.h"
#include "ailink_uartHal.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include "mcu_system.h"
#include "ailink_softtimer.h"
#include "ailink_wifiHal.h"
#include "mcu_log.h"
#include <bl602_glb.h>
#include <bl_uart.h>
#include "iot_cmd.h"
#include "ringbuff.h"
#include "iot_uart.h"
#include "iot_sys.h"

#define AT_CMD_PROCESS          0x55
#define HEX_CMD_PROCESS         0xaa

#define AT_UART_NUM 1


static uint8_t  uart_rx_buffer[MCU_UART_BUFF_MAX_LEN] = {0};
static char  uart_cmd_buffer[MCU_UART_BUFF_MAX_LEN] = {0};
static uint32_t heart_beat_timeout = 1000;
static AILINK_TIMER   heart_beat_timer = -1;
static AILINK_TIMER   mcu_public_timer = -1;
static uint16_t uart_rx_len = 0;
static uint16_t dataLen = 0;
static bool  AtStartProcess = false;
static uint8_t  cmdProcessState = HEX_CMD_PROCESS;
static ring_buff_t uart_ring_buff_hd = {0};



static void McuProcessTask(void *arg);
static void IotUart1DataRecevCb(uint8_t data);

/**
 * @brief   初始化MCU通信处理任务
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-25
 */
void McuProcessInit(void)
{
    IotUartConfig   uart_config = {0};
    IotDeviceParams params = {0};

    UartProtocolInit();
    McuLogInit();
    ring_buff_init(&uart_ring_buff_hd, (char *)uart_rx_buffer, MCU_UART_BUFF_MAX_LEN);
    IotLoadDeviceParams(&params);
    if(params.uart_bound == 0)
    {
        params.uart_bound = 9600;
    }

    IotUartDeinit();
    uart_config.params.uart_rx_pin = 7;
    uart_config.params.uart_tx_pin = 16;
    uart_config.params.uart_bound = params.uart_bound;
    uart_config.IotUartDataRecvCb = IotUart1DataRecevCb;
    IotUartInit(&uart_config);

    // 创建发送定时器
    heart_beat_timer = AilinkTimerAdd(AILINK_REPEAT_TIMER);
    if(heart_beat_timer == -1)
    {
        MCU_ErrorLog("add heart beat timer fail \r\n");
        return ;
    }
    AilinkTimerStop(heart_beat_timer);

    // 创建发送定时器
    mcu_public_timer = AilinkTimerAdd(AILINK_SINGLE_TIMER);
    if(mcu_public_timer == -1)
    {
        MCU_ErrorLog("add mcu_public_timer fail \r\n");
        return ;
    }
    AilinkTimerStop(mcu_public_timer);

    xTaskCreate(McuProcessTask, (char*)"McuProcessTask", 4*1024/4, NULL, 15, NULL);
}


/**
 * @brief   重新设备发送心跳包的间隔时间
 * 
 * @param[in]   SetTimeout   设置发送心跳包的间隔时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-24
 */
void  SetHeartBeatTimeout(uint32_t SetTimeout)
{
    heart_beat_timeout = SetTimeout;
    AilinkTimerStart(heart_beat_timer, heart_beat_timeout);
}

/**
 * @brief   重新设置向云端发送数据的超时时间
 * 
 * @param[in]   SetTimeout      超时时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-09-01
 */
void SetPublicDataTimeout(uint32_t SetTimeout)
{
    AilinkTimerStart(mcu_public_timer, SetTimeout);
}


static void McuProcessTask(void *arg)
{
    int32_t ret = 0, len = 0;
    uint32_t count = 0;
    uint16_t cmdlen = 0;
    char IsHexCom = false;

    AilinkTimerStart(heart_beat_timer, heart_beat_timeout);

    while (1)
    {

        UartProcessPro();

        if(AilinkCheckTimerTimeout(heart_beat_timer) == AILINK_TIMER_TIMEOUT)
        {
            MCU_InfoDebug("send heart beat data \r\n");
            UartSendHeatBeat();
        }

        if(AilinkCheckTimerTimeout(mcu_public_timer) == AILINK_TIMER_TIMEOUT)
        {
            MCU_InfoDebug("mcu pubilc data \r\n");
            McuPubliReportAttributes();
        }

        if(HeartBeatTimeout())
        {
            MCU_InfoDebug("send mcu heart beat timeout, then reboot \r\n");
            vTaskDelay(20);
            ailinkReboot();
        }

        if(ring_buff_get_size(&uart_ring_buff_hd) <= 0)
        {
            vTaskDelay(20);
            continue;
        }

        ring_buff_pop_data(&uart_ring_buff_hd, (uart_cmd_buffer + cmdlen), 1);
        cmdlen += 1;
        if(cmdlen > sizeof(uart_cmd_buffer))
        {
            MCU_ErrorLog("rev data[%d]: %s \r\n", cmdlen, uart_cmd_buffer);
            ring_buff_flush(&uart_ring_buff_hd);
            memset(uart_cmd_buffer, 0, sizeof(uart_cmd_buffer));
            cmdlen = 0;
            continue;
        }

        if((cmdlen >= 4) && ('\r' == uart_cmd_buffer[cmdlen-2]) && ('\n' == uart_cmd_buffer[cmdlen-1]))
        {
            // printf("\r\n");
            // for(uint8_t i = 0; i < cmdlen; i++)
            // {
            //     printf("%02x", DataBuff[i]);
            // }
            // printf("\r\n");
            // MCU_InfoDebug("rev:%s \r\n", DataBuff);
            // MCU_InfoDebug("cmdlen:%d \r\n", cmdlen);
            uart_cmd_buffer[cmdlen-2] = '\0';
            atCmdExecute((char *)uart_cmd_buffer);
            cmdlen = 0;
            memset(uart_cmd_buffer, 0, sizeof(uart_cmd_buffer));
        }
        else
        {
            if((cmdlen >= 7) && (uart_cmd_buffer[0] == 0x5a && uart_cmd_buffer[1] == 0xa5) && (UartCheckValue((uint8_t *)uart_cmd_buffer, cmdlen -1) == uart_cmd_buffer[cmdlen - 1]))
            {
                // MCU_InfoDebug("receive data: \r\n");
                // for(uint8_t i = 0; i < cmdlen; i++)
                // {
                //     printf("0x%02x ", DataBuff[i]);
                // }
                // printf("\r\n");
                // UartProtocolInit();
                UartRevStream((uint8_t *)uart_cmd_buffer, cmdlen);
                memset(uart_cmd_buffer, 0, sizeof(uart_cmd_buffer));
                cmdlen = 0;
            }
        }
    }
    
}

static void IotUart1DataRecevCb(uint8_t data)
{
    char value = (char )data;

    // printf("%c", value);
    // printf("%02x", data);
    ring_buff_push_data(&uart_ring_buff_hd, &value, 1);
}
