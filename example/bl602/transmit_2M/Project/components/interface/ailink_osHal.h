/**
 * @file    ailink_os.h
 * @brief   声明ailink必备的OS函数
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-11|1.0.0  |Ai-Thinker     |创建
 */
#ifndef __AILINK_OSHAL_H
#define __AILINK_OSHAL_H
#include <stdint.h>


/**< 统一互锁变量的声明 */
typedef void *Ailink_mutex_t;
/**< 统一邮箱队列变量的声明 */
typedef void *Ailink_mbox_t;
/**< 统一任务句柄变量的声明 */
typedef void *Ailink_thread_t;


/**
 * @brief   线程函数的函数原型
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
typedef void (*Ailink_thread_fn)(void *arg);


/**
 * @brief   创建互锁变量
 * 
 * @return  Ailink_mutex_t*  创建的互锁变量指针，NULL表示创建失败，非NULL表示创建成功
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
Ailink_mutex_t Ailink_mutex_new(void);

/**
 * @brief   操作互锁变量
 * 
 * @param[in]   mutex  待操作的互锁变量
 * @return  int32_t    返回操作互锁的状态
 *          0：操作互锁成功
 *          非0：操作互锁失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
int32_t Ailink_mutex_lock(Ailink_mutex_t mutex);

/**
 * @brief   释放互锁变量
 * 
 * @param[in]   mutex  待释放的互锁变量
 * 
 * @return  int32_t    返回释放互锁的状态
 *          0：释放互锁成功
 *          非0：释放互锁失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
int32_t Ailink_mutex_unlock(Ailink_mutex_t mutex);

/**
 * @brief   删除互锁变量
 * 
 * @param[in]   mutex  待删除互锁变量
 * 
 * @return  int32_t    返回删除互锁的状态
 *          0：删除互锁成功
 *          非0：删除互锁失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
int32_t Ailink_mutex_free(Ailink_mutex_t mutex);


/**
 * @brief   创建邮箱队列
 * 
 * @param[in]   size    需创建邮箱队列的大小
 * @return  Ailink_mbox_t*  返回邮箱队列的句柄指针，
 *          NULL：创建邮箱队列失败
 *          非NULL：创建邮箱成功
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
Ailink_mbox_t Ailink_mbox_new(uint16_t size);

/**
 * @brief   通过邮箱队列发送消息
 * 
 * @param[in]   mbox        邮箱队列句柄
 * @param[in]   msg         待发送的消息
 * 
 * @return  int32_t    邮箱消息发送情况
 *          0：邮箱消息发送成功
 *          非0：邮箱消息发送失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
int32_t Ailink_mbox_post(Ailink_mbox_t mbox, void *msg);

/**
 * @brief   通过邮箱队列接收消息
 * 
 * @param[in]   mbox        邮箱队列句柄
 * @param[in]   msg         待接收的数据
 * @return  int32_t         返回邮箱接收情况
 *          0：邮箱消息接收成功
 *          非0：邮箱消息接收失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
int32_t Ailink_mbox_Rev(Ailink_mbox_t mbox, void *msg);


/**
 * @brief   删除邮箱队列
 * 
 * @param[in]   mbox  待邮箱队列的句柄
 * @return  int32_t     返回邮箱删除情况
 *          0：删除成功 
 *          非0：删除失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
int32_t Ailink_mbox_free(Ailink_mbox_t mbox);


/**
 * @brief   创建线程函数
 * 
 * @param[in]   name            线程函数名
 * @param[in]   thread          线程函数
 * @param[in]   arg             传给线程函数的参数
 * @param[in]   stacksize       线程函数的任务栈大小
 * @param[in]   prio            线程的优先级
 * @return  Ailink_thread_t     返回线程函数的句柄
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
Ailink_thread_t Ailink_thread_new(const char *name, Ailink_thread_fn thread, void *arg, int stacksize, int prio);


/**
 * @brief   OS延迟函数
 * 
 * @param[in]   delay_ms        延迟时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
void Ailink_thread_delay(uint32_t delay_ms);

/**
 * @brief   删除线程函数
 * 
 * @param[in]   thread_handle   待删除的线程句柄
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
void Ailink_thread_delete(Ailink_thread_t * thread_handle);

#endif
