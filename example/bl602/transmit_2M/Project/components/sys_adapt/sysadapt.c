
/**
 * @file    sysadapt.c
 * @brief   适配bl602上SDK里需定义的函数，这些函数是用于SDK的FreeRTOS的。
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
#include "sysadapt.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>




/* TODO: const */
volatile uint32_t uxTopUsedPriority __attribute__((used)) =  configMAX_PRIORITIES - 1;





/********************************************************************************************/
                /*   以下为系统的回调函数     */

/*********************************************************************************************/

void __attribute__((weak)) vApplicationMallocFailedHook(void)
{
    printf("Memory Allocate Failed. Current left size is %d bytes\r\n",
        xPortGetFreeHeapSize()
    );
    while (1) {
        /*empty here*/
    }
}

void __attribute__((weak)) vApplicationIdleHook(void)
{
    __asm volatile(
            "   wfi     "
    );
    /*empty*/
}

void __attribute__((weak)) vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}



void vApplicationSleep( TickType_t xExpectedIdleTime_ms )
{
#if defined(CFG_BLE_PDS)
    int32_t bleSleepDuration_32768cycles = 0;
    int32_t expectedIdleTime_32768cycles = 0;
    eSleepModeStatus eSleepStatus;
    bool freertos_max_idle = false;


    if(xExpectedIdleTime_ms + xTaskGetTickCount() == portMAX_DELAY){
        freertos_max_idle = true;
    }else{   
        xExpectedIdleTime_ms -= 1;
        expectedIdleTime_32768cycles = 32768 * xExpectedIdleTime_ms / 1000;
    }

    if((!freertos_max_idle)&&(expectedIdleTime_32768cycles < TIME_5MS_IN_32768CYCLE)){
        return;
    }

    /*Disable mtimer interrrupt*/
    *(volatile uint8_t*)configCLIC_TIMER_ENABLE_ADDRESS = 0;

    eSleepStatus = eTaskConfirmSleepModeStatus();
    if(eSleepStatus == eAbortSleep || ble_controller_sleep_is_ongoing())
    {
        /*A task has been moved out of the Blocked state since this macro was
        executed, or a context siwth is being held pending.Restart the tick 
        and exit the critical section. */
        /*Enable mtimer interrrupt*/
        *(volatile uint8_t*)configCLIC_TIMER_ENABLE_ADDRESS = 1;
        //printf("%s:not do ble sleep\r\n", __func__);
        return;
    }

    bleSleepDuration_32768cycles = ble_controller_sleep();

	if(bleSleepDuration_32768cycles < TIME_5MS_IN_32768CYCLE)
    {
        /*BLE controller does not allow sleep.  Do not enter a sleep state.Restart the tick 
        and exit the critical section. */
        /*Enable mtimer interrrupt*/
        //printf("%s:not do pds sleep\r\n", __func__);
        *(volatile uint8_t*)configCLIC_TIMER_ENABLE_ADDRESS = 1;
    }
    else
    {
        printf("%s:bleSleepDuration_32768cycles=%ld\r\n", __func__, bleSleepDuration_32768cycles);
        if(eSleepStatus == eStandardSleep && ((!freertos_max_idle) && (expectedIdleTime_32768cycles < bleSleepDuration_32768cycles)))
        {
           hal_pds_enter_with_time_compensation(1, expectedIdleTime_32768cycles - 40);//40);//20);
        }
        else
        {
           hal_pds_enter_with_time_compensation(1, bleSleepDuration_32768cycles - 40);//40);//20);
        }
    }
#endif
}
