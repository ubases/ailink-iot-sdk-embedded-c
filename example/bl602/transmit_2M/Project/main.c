/**
 * @file    main.c
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-08-17
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-08-17|1.0.0  |Ai-Thinker      |创建
 */
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <hosal_uart.h>
#include <easyflash.h>
#include <bl_sys.h>


#include <lwip/tcpip.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/tcp.h>
#include <lwip/err.h>
#include <http_client.h>
#include <netutils/netutils.h>

#include "iot_sys.h"
#include "iot_uart.h"
#include "ailink_uartHal.h"
#include "ailink_timerHal.h"



HOSAL_UART_DEV_DECL(uart0_stdio, 0, 3, 4, 115200);



/**
 * @brief   该函数是任务处理程序，用于处理进入应用程序的调用
 * 
 * @param[in]   pvParameters   任务处理参数，该参数在创建该任务是为NULL
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
static void proc_iotgo_entry(void *pvParameters)
{

    easyflash_init();

    iotgo_sys_entry();
    
    vTaskDelete(NULL);
}



/**
 * @brief   main函数处理程序
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void main(void)
{
    char print_info[40] = {0};
    int32_t ret = 0;
    AilinkRTCTimerHal_t RtcTimer = {0};

    bl_sys_rstinfo_init();
    /*Init UART In the first place*/
    hosal_uart_init(&uart0_stdio);
    IotUartInit(NULL);

    
    bl_sys_rstinfo_getsting(print_info);
    printf("############################## build info  ##################################\r\n");
    printf("build time: %s \r\n", BUILDTIME);
    printf("fw version: %s \r\n", DEVICE_FW_VERSION);
    printf("Boot Reason:<%s> \r\n", print_info);
    AilinkLoadTimeInfo(&RtcTimer);
    printf("the Date : %02d:%02d:%02d \r\n", RtcTimer.year, RtcTimer.mon, RtcTimer.day);
    printf("the time : %02d:%02d:%02d \r\n", RtcTimer.hour, RtcTimer.minute, RtcTimer.second);
    printf("############################## end  ##################################\r\n");

    bl_sys_init();

    printf("############################## task create  ##################################\r\n");

    printf("[OS] Starting proc_iotgo_entry task...\r\n");
    xTaskCreate(proc_iotgo_entry, (char*)"iotgo_entry", 5*1024/4, NULL, 15, NULL);

    printf("############################## task create end ##################################\r\n");

    printf("[OS] Starting TCP/IP Stack...\r\n");
    tcpip_init(NULL, NULL);
}