/**
 * @file    ailink_socketHal.h
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-12|1.0.0  |Ai-Thinker     |创建
 */
#ifndef __AILINK_SOCKETHAL_H
#define __AILINK_SOCKETHAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>




/**
 * @brief   获取系统tick时间
 * 
 * @return  uint32_t  已获得的系统tick时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
uint32_t ailinkGetSystemTick(void);


/**
 * @brief 随机数发生器的初始化函数。
 *
 * @par 描述:
 * 根据系统提供的种子值，产生随机数。
 *
 * @param 无
 *
 * @retval 随机数
 */
uint32_t AilinkGetRand(void);


/**
 * @brief   创建tcp服务
 * 
 * @note    创建非阻塞模式tcp服务
 * 
 * @param[in]   port     [IN] 接收方端口号。
 * @return  int32_t      大于等于0  建立成功，返回tcp套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkCreateTcpServer(uint16_t port);



/**
 * @brief   获取tcp服务状态
 * 
 * @param[in]   fd   tcp套接字
 * @return  int32_t         返回tcp服务与客户端建立连接状态
 *          0：与客户端建立连接成功
 *          1：等待建立连接
 *          -1：tcp连接有错误
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpServerGetState(int32_t fd);


/**
 * @brief   创建tcp连接
 * 
 * @note    创建非阻塞模式tcp连接
 * 
 * @param[in]   dst         [IN] 接收方ip地址。
 * @param[in]   port        [IN] 接收方端口号。
 * @return  int32_t     大于0  连接成功,返回tcp套接字，小于或等于0，创建失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpConnect(const char* dst, uint16_t port);


/**
 * @brief   tcp连接状态
 * 
 * @par 描述:
 * 查询tcp连接状态
 * 
 * @param[in]   fd    [IN] tcp套接字。
 * @return  int32_t   返回tcp连接状态
 *          0：tcp已建立连接
 *          1：tcp等待连接
 *          2：tcp已断开连接
 *          -1：tcp连接状态错误
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpState(int32_t fd);

/**
 * @brief   断开tcp连接
 * 
 * @param[in]   fd  [IN] tcp套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
void ailinkTcpDisconnect(int32_t fd);


/**
 * @brief   发送tcp数据
 * 
 * @par 描述:
 * 非阻塞发送tcp数据
 * 
 * @param fd      [IN] tcp套接字。
 * @param buf     [IN] 指向待发送数据缓冲区的指针。
 * @param len     [IN] 待发送数据的长度，范围为[0，1024]。
 * @return  int32_t  返回tcp发送状态
 *          0：tcp发送成功
 *          1：tcp发送失败
 *          -1：参数错误
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSend(int32_t fd, const uint8_t* buf, uint16_t len);


/**
 * @brief   读取tcp数据
 * 
 * @par 描述:
 * 非阻塞读取tcp数据
 * 
 * @param fd      [IN] tcp套接字。
 * @param buf     [IN] 指向存放接收数据缓冲区的指针。
 * @param len     [IN] 存放接收数据缓冲区的最大长度，范围为[0，1024]。
 * @return  int32_t  返回已读取的数据长度。
 *                  0：表示未读到数据，需继续等待
 *                  1：表示参数错误
 *                  小于0：表示接收数据错误或是连接已断开
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpRead(int32_t fd, uint8_t* buf, uint16_t len);


/**
 * @brief   创建加密tcp连接
 * 
 * @par 描述:
 * 创建非阻塞模式加密tcp连接
 * 
 * @param[in]   dst             [IN] 接收方ip地址。
 * @param[in]   port            [IN] 接收方端口号。
 * @return  int32_t             大于0  连接成功,返回tcp套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSslConnect(const char* dst, uint16_t port);


/**
 * @brief   加密tcp连接状态
 * 
 * @par 描述:
 * 查询加密tcp连接状态
 * 
 * @param[in]   fd          [IN] tcp套接字。
 * @return  int32_t         返回tcp连接状态
 *          0：tcp已建立连接
 *          1：tcp等待建立连接
 *          2：tcp已断开连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSslState(int32_t fd);


/**
 * @brief   断开加密tcp连接。
 * 
 * @param[in]   fd   [IN] 套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
void ailinkTcpSslDisconnect(int32_t fd);


/**
 * @brief   发送加密tcp数据
 * 
 * @par 描述:
 * 非阻塞发送加密tcp数据
 * 
 * @param[in]   fd          [IN] tcp套接字。
 * @param[in]   buf         [IN] 指向待发送数据缓冲区的指针。
 * @param[in]   len         [IN] 待发送数据的长度，范围为[0，1024]。
 * @return  int32_t         返回数据发送情况
 *          0：数据已发送成功
 *          1：数据发送失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSslSend(int32_t fd, const uint8_t* buf, uint16_t len);


/**
 * @brief   读取加密tcp数据
 * 
 * @par 描述:
 * 非阻塞读取加密tcp数据
 * 
 * @param[in]   fd          [IN] tcp套接字。
 * @param[in]   buf         [IN] 指向存放接收数据缓冲区的指针。
 * @param[in]   len         [IN] 存放接收数据缓冲区的最大长度，范围为[0，512)。
 * @return  int32_t         返回数据长度，0：表示未读取到数据
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSslRead(int32_t fd, uint8_t* buf, uint16_t len);



/**
 * @brief   创建udp服务
 * 
 * @note    创建非阻塞模式udp服务
 * 
 * @param[in]   port     [IN] 接收方端口号。
 * @return  int32_t      大于等于0  建立成功，返回tcp套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkCreateUDPServer(uint16_t port);



/**
 * @brief   创建UDP客户端
 * 
 * @param[in]   ip          与该客户端通信的服务器Ip地址
 * @param[in]   port        与该客户端通信的服务器端口
 * @return  int32_t         返回客户端建立情况
 *          0：创建成功
 *          非0：创建失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-14
 */
int32_t ailinkCreateUDPClient(char *ip, uint16_t port);


/**
 * @brief   创建UDP组播客户端
 * 
 * @param[in]   ip          与该客户端通信的服务器Ip地址
 * @param[in]   port        与该客户端通信的服务器端口
 * @return  int32_t         返回客户端建立情况
 *          0：创建成功
 *          非0：创建失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-14
 */
int32_t ailinkCreateUDPGroupClient(char *ip, uint16_t port);


/**
 * @brief   udp服务回复数据
 * 
 * @par 描述:
 * 非阻塞发送udp数据
 * 
 * @param buf     [IN] 指向待发送数据缓冲区的指针。
 * @param len     [IN] 待发送数据的长度，范围为[0，1024]。
 * @return  int32_t  返回udp发送状态
 *          大于或等于0：udp发送成功
 *          小于：udp发送失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkUDPServerSend(int32_t fd, const uint8_t* buf, uint16_t len);


/**
 * @brief   udp服务读取数据
 * 
 * @par 描述:
 * 非阻塞读取udp数据
 * 
 * @param fd      [IN] udp套接字。
 * @param buf     [IN] 指向存放接收数据缓冲区的指针。
 * @param len     [IN] 存放接收数据缓冲区的最大长度，范围为[0，1024]。
 * @return  int32_t  返回已读取的数据长度。0：表示未读到数据，-1：表示udp已断开连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkUDPServerRead(int32_t fd, uint8_t* buf, uint16_t len);


/**
 * @brief   udp客户端发送数据
 * 
 * @par 描述:
 * 非阻塞发送udp数据
 * 
 * @param buf     [IN] 指向待发送数据缓冲区的指针。
 * @param len     [IN] 待发送数据的长度，范围为[0，1024]。
 * @return  int32_t  返回udp发送状态
 *          大于或等于0：udp发送成功
 *          小于：udp发送失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkUDPClientSend(int32_t fd, const uint8_t* buf, uint16_t len);



/**
 * @brief   udp客户端接收数据
 * 
 * @par 描述:
 * 非阻塞读取udp数据
 * 
 * @param fd      [IN] udp套接字。
 * @param buf     [IN] 指向存放接收数据缓冲区的指针。
 * @param len     [IN] 存放接收数据缓冲区的最大长度，范围为[0，1024]。
 * @return  int32_t  返回已读取的数据长度。0：表示未读到数据，-1：表示udp已断开连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkUDPClientRead(int32_t fd, uint8_t* buf, uint16_t len);


/**
 * @brief   udp发送组播数据
 * 
 * @par 描述:
 * 非阻塞发送udp数据
 * 
 * @param buf     [IN] 指向待发送数据缓冲区的指针。
 * @param len     [IN] 待发送数据的长度，范围为[0，1024]。
 * @return  int32_t  返回udp发送状态
 *          大于或等于0：udp发送成功
 *          小于：udp发送失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkUDPGroupSend(int32_t fd, const uint8_t* buf, uint16_t len);


/**
 * @brief  关闭UDP功能
 * 
 * @param[in]   fd  [IN] UDP套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
void ailinkUDPDisconnect(int32_t fd);


/**
 * @brief   获取远端主机ip地址
 * 
 * @par 描述:
 * 通过DNS域名解析，获取远端主机ip地址，此接口实现为非阻塞。
 * 
 * @param[in]   hostname            [IN] 远端主机名称。
 * @param[in]   len                 [IN] 远端主机名称长度。
 * @return  char*                   返回通过DNS解析后，返回解析后的Ip地址
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
char *ailinkGethostbyname(const char* hostname, uint8_t len);






#endif
