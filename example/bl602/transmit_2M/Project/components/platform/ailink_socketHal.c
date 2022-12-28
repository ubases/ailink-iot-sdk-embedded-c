/**
 * @file    ailink_socket.c
 * @brief   
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-05-13|1.0.0  |Ai-Thinker     |创建
 */
#include "ailink_socketHal.h"
#include "iot_log.h"
#include "ailink_osHal.h"


#include <stdint.h>
#include "lwip/err.h"
#include <lwip/sockets.h>
#include <lwip/udp.h>
#include <aos/kernel.h>

#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/opt.h"

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"



struct sockaddr_storage source_addr;        /**< 该变量用于存储客户端的IP地址，即当服务器接收到客户端发送来的
                                                 消息时，将客户端的IP地址存储于该变量上 */

struct sockaddr_in server_dest_addr;        /**< 该变量用于存储待连接服务器的IP地址，即当客户端需向服务器发送
                                                 数据时，可用该地址进行向服务器发送数据 */

struct sockaddr_in group_dest_addr;        /**< 该变量用于存储组播地址 */

static Ailink_mutex_t ailink_metex = NULL;


#define     AILINK_VERIFY

#if defined(AILINK_VERIFY)

#if 1
const char ailink_server_crt[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIICbjCCAdegAwIBAgIJAPsKI5XggYKPMA0GCSqGSIb3DQEBCwUAME8xCzAJBgNV\r\n"
"BAYTAkNOMQswCQYDVQQIDAJITjERMA8GA1UEBwwIY2hhbmdzaGExDDAKBgNVBAoM\r\n"
"A0JBVDESMBAGA1UEAwwJbG9jYWxob3N0MCAXDTIyMDkwMjAwNDYzN1oYDzIwNTAw\r\n"
"MTE4MDA0NjM3WjBPMQswCQYDVQQGEwJDTjELMAkGA1UECAwCSE4xETAPBgNVBAcM\r\n"
"CGNoYW5nc2hhMQwwCgYDVQQKDANCQVQxEjAQBgNVBAMMCWxvY2FsaG9zdDCBnzAN\r\n"
"BgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA9PHdrbBDhvHmH9Yqy55wu+P5ozN5X1Va\r\n"
"AGlX+sYgZgAiQlVDz3+NuKxkH7GVBshr/dGcke9tplMDqATcldTI7WiqnCD52Dbk\r\n"
"yuvc9aD86in9pqDAbWNQ5CfSET4IUCFDsM44jthbR0v68OUnrlCmK00GvGh6u/2c\r\n"
"JRLY+A7Kr7UCAwEAAaNQME4wHQYDVR0OBBYEFMU18cx73uqHiNMGfPN+DpxGE7aS\r\n"
"MB8GA1UdIwQYMBaAFMU18cx73uqHiNMGfPN+DpxGE7aSMAwGA1UdEwQFMAMBAf8w\r\n"
"DQYJKoZIhvcNAQELBQADgYEAJp0TbWHdw5hlVj/NlnjAEZPcUzqdj38d5RZVjVvl\r\n"
"VieGe949KmhNh8YyvrRdfCMpy54JSiQ0pRUXnCzK6ABv5G9PzWXT3xVxr+Uir47E\r\n"
"IXQO9Q7MMUQp2CctsyOyo+byjQ9cH8KVdZ2XUBXf8IMquRgedvvQV31Dw6fjwJTg\r\n"
"Yhg=\r\n"
"-----END CERTIFICATE-----\r\n";

#else

#if 0
const char ailink_server_crt[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\r\n"
"BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\r\n"
"A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\r\n"
"BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\r\n"
"by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\r\n"
"BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\r\n"
"MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\r\n"
"dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\r\n"
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\r\n"
"UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\r\n"
"Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\r\n"
"s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\r\n"
"3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\r\n"
"E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\r\n"
"MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\r\n"
"6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\r\n"
"BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\r\n"
"6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\r\n"
"+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\r\n"
"sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\r\n"
"LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\r\n"
"m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\r\n"
"-----END CERTIFICATE-----\r\n";

#else
const char ailink_server_crt[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIErjCCA5agAwIBAgIQBYAmfwbylVM0jhwYWl7uLjANBgkqhkiG9w0BAQsFADBh\r\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
"QTAeFw0xNzEyMDgxMjI4MjZaFw0yNzEyMDgxMjI4MjZaMHIxCzAJBgNVBAYTAkNO\r\n"
"MSUwIwYDVQQKExxUcnVzdEFzaWEgVGVjaG5vbG9naWVzLCBJbmMuMR0wGwYDVQQL\r\n"
"ExREb21haW4gVmFsaWRhdGVkIFNTTDEdMBsGA1UEAxMUVHJ1c3RBc2lhIFRMUyBS\r\n"
"U0EgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCgWa9X+ph+wAm8\r\n"
"Yh1Fk1MjKbQ5QwBOOKVaZR/OfCh+F6f93u7vZHGcUU/lvVGgUQnbzJhR1UV2epJa\r\n"
"e+m7cxnXIKdD0/VS9btAgwJszGFvwoqXeaCqFoP71wPmXjjUwLT70+qvX4hdyYfO\r\n"
"JcjeTz5QKtg8zQwxaK9x4JT9CoOmoVdVhEBAiD3DwR5fFgOHDwwGxdJWVBvktnoA\r\n"
"zjdTLXDdbSVC5jZ0u8oq9BiTDv7jAlsB5F8aZgvSZDOQeFrwaOTbKWSEInEhnchK\r\n"
"ZTD1dz6aBlk1xGEI5PZWAnVAba/ofH33ktymaTDsE6xRDnW97pDkimCRak6CEbfe\r\n"
"3dXw6OV5AgMBAAGjggFPMIIBSzAdBgNVHQ4EFgQUf9OZ86BHDjEAVlYijrfMnt3K\r\n"
"AYowHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUwDgYDVR0PAQH/BAQD\r\n"
"AgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAG\r\n"
"AQH/AgEAMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYYaHR0cDovL29jc3Au\r\n"
"ZGlnaWNlcnQuY29tMEIGA1UdHwQ7MDkwN6A1oDOGMWh0dHA6Ly9jcmwzLmRpZ2lj\r\n"
"ZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwTAYDVR0gBEUwQzA3Bglg\r\n"
"hkgBhv1sAQIwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29t\r\n"
"L0NQUzAIBgZngQwBAgEwDQYJKoZIhvcNAQELBQADggEBAK3dVOj5dlv4MzK2i233\r\n"
"lDYvyJ3slFY2X2HKTYGte8nbK6i5/fsDImMYihAkp6VaNY/en8WZ5qcrQPVLuJrJ\r\n"
"DSXT04NnMeZOQDUoj/NHAmdfCBB/h1bZ5OGK6Sf1h5Yx/5wR4f3TUoPgGlnU7EuP\r\n"
"ISLNdMRiDrXntcImDAiRvkh5GJuH4YCVE6XEntqaNIgGkRwxKSgnU3Id3iuFbW9F\r\n"
"UQ9Qqtb1GX91AJ7i4153TikGgYCdwYkBURD8gSVe8OAco6IfZOYt/TEwii1Ivi1C\r\n"
"qnuUlWpsF1LdQNIdfbW3TSe0BhQa7ifbVIfvPWHYOu3rkg1ZeMo6XRU9B4n5VyJY\r\n"
"RmE=\r\n"
"-----END CERTIFICATE-----\r\n";

#endif


#endif


const int32_t ailink_server_crt_len = sizeof(ailink_server_crt);

#endif

char ip_list[1][20];

mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_net_context server_fd;

#if defined(AILINK_VERIFY)
mbedtls_x509_crt cacert;
#endif



static int32_t AilinkMutexLock(void)
{
    if(!ailink_metex)
    {
        ailink_metex = Ailink_mutex_new();
        if(ailink_metex == NULL)
        {
            IOT_ErrorLog("mutex create failed \r\n");
            return -1;
        }
    }

    return Ailink_mutex_lock(ailink_metex);
}


static int32_t AilinkMutexUnLock(void)
{
    if(!ailink_metex)
    {
        ailink_metex = Ailink_mutex_new();
        if(ailink_metex == NULL)
        {
            IOT_ErrorLog("mutex create failed \r\n");
            return -1;
        }
    }

    return Ailink_mutex_unlock(ailink_metex);
}



/**
 * @brief   获取系统tick时间
 * 
 * @return  uint32_t  已获得的系统tick时间
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-13
 */
uint32_t ailinkGetSystemTick(void)
{
    return xTaskGetTickCount() * (1000 / configTICK_RATE_HZ);
}


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
uint32_t AilinkGetRand(void)
{
    return (rand() % RAND_MAX);
}


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
int32_t ailinkCreateTcpServer(uint16_t port)
{
    int sockfd;
    int32_t ret = -1;
    struct sockaddr_in address;
    int flags;
    int reuse;

    /* create a TCP socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        IOT_ErrorLog("can not create socket\r\n");
        ret = -1;
    }
    else
    {
        flags = fcntl(sockfd, F_GETFL, 0);

        if (flags < 0 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            IOT_ErrorLog("fcntl: %s\n", strerror(errno));
            close(sockfd);
            ret = -1;
        }
        else
        {
            reuse = 1;

            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                        (const char*) &reuse, sizeof(reuse)) != 0)
            {
                close(sockfd);
                IOT_ErrorLog("set SO_REUSEADDR failed\n");
                ret = -1;
            }
            else
            {
                /* bind to port 80 at any interface */
                address.sin_family = AF_INET;
                address.sin_port = htons(port);
                address.sin_addr.s_addr = INADDR_ANY;
                if (bind(sockfd, (struct sockaddr *)&address, sizeof (address)) < 0)
                {
                    IOT_ErrorLog("can not bind socket\r\n");
                    close(sockfd);
                    ret = -1;
                }
                else
                {
                    /* listen for connections (TCP listen backlog = 1) */
                    listen(sockfd, 1);
                    ret = sockfd;
                }
            }
        }
    }

    return ret;

}



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
int32_t ailinkTcpServerGetState(int32_t fd)
{
    int ret = 0;
    int tcp_fd = fd;
    int new_fd = -1;
    int len = (int) sizeof(int);
    fd_set rset, wset;
    int ready_n;
    struct timeval timeout;

    if(fd < 0)
    {
        IOT_ErrorLog("fd error \r\n");
        return -1;
    }
    else
    {
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_CLR(tcp_fd, &rset);
        FD_CLR(tcp_fd, &wset);
        FD_SET(tcp_fd, &rset);
        FD_SET(tcp_fd, &wset);

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        ready_n = select(tcp_fd + 1, &rset, &wset, NULL, &timeout);

        if (0 == ready_n)
        {
            ret = 1;
        }
        else if (ready_n < 0)
        {
            IOT_ErrorLog("select error\n");
            ret = -1;
        }
        else
        {
            if (FD_ISSET(tcp_fd, &wset) != 0)
            {
                new_fd = accept(tcp_fd, NULL, NULL);
                if (0 <= new_fd)
                {
                    ret = new_fd;
                }
                else
                {
                    ret = -1;
                }
            }
            else
            {
                if (0 != getsockopt(tcp_fd, SOL_SOCKET, SO_ERROR, &ret, (socklen_t *)&len))
                {
                    IOT_ErrorLog("getsocketopt failed\r\n");
                    ret = -1;
                }

                if (0 != ret)
                {
                    ret = -1;
                }

                new_fd = accept(tcp_fd, NULL, NULL);
                if (0 <= new_fd)
                {
                    ret = new_fd;
                }
                else
                {
                    ret = -1;
                }
            }
        }
    }

    return ret;
}


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
int32_t ailinkTcpConnect(const char* dst, uint16_t port)
{
    struct sockaddr_in servaddr;
    int fd;
    int ret;
    int flags;
    int reuse;

    if(dst == NULL)
    {
        IOT_ErrorLog("param error \r\n");
        return -1;
    }
    else
    {
        IOT_InfoLog("create socket \r\n");
        AilinkMutexLock();
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd < 0) 
        {
            IOT_ErrorLog("creat socket fd failed\r\n");
            ret = -1;
        }
        else
        {
            flags = 0;
            IOT_InfoLog("fd = %d \r\n", fd);
            if (ioctlsocket(fd, FIONBIO, &flags) != 0)
            {
                IOT_ErrorLog("fcntl: %s\r\n", strerror(errno));
                close(fd);
                ret = -1;
            }
            else
            {
                IOT_InfoLog("setsockopt \r\n");
                reuse = 1000;
                if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,(const char*) &reuse, sizeof(reuse)) != 0)
                {
                    close(fd);
                    IOT_ErrorLog("set SO_REUSEADDR failed\r\n");
                    ret = -1;
                }
                else
                {
                    memset(&servaddr, 0, sizeof(servaddr));

                    servaddr.sin_family = AF_INET;
                    servaddr.sin_addr.s_addr = inet_addr(dst);
                    servaddr.sin_port = htons(port);

                    IOT_InfoLog("connect tcp \r\n");
                    if ((ret = connect(fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in))) == 0)
                    {
                        IOT_InfoLog("dst %s errno %d ret %d \r\n", dst, errno, ret);
                        IOT_InfoLog("fd = %d \r\n", fd);
                        ret = fd;
                        IOT_InfoLog("ioctlsocket \r\n");
                        flags = 1;
                        if (ioctlsocket(fd, FIONBIO, &flags) != 0)
                        {
                            IOT_ErrorLog("fcntl: %s\r\n", strerror(errno));
                            close(fd);
                            ret = -1;
                        }
                    }
                    else
                    {
                        IOT_ErrorLog("dst %s port %d errno %d ret %d\r\n", dst, port, errno, ret);

                        close(fd);
                        ret = -1;
                    }
                }
            }
        }
        AilinkMutexUnLock();
        IOT_InfoLog("tcp connect finish \r\n");
    }

    return ret;
}


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
int32_t ailinkTcpState(int32_t fd)
{
    int errcode = -1;
    int tcp_fd = fd;
    fd_set rset, wset;
    int ready_n;
    struct timeval timeout;
    int ret;
    int len = (int) sizeof(int);


    if(fd < 0)
    {
        IOT_ErrorLog("fd error  \r\n");
        return -1;
    }
    else
    {
        AilinkMutexLock();
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_CLR(tcp_fd, &rset);
        FD_CLR(tcp_fd, &wset);
        FD_SET(tcp_fd, &rset);
        FD_SET(tcp_fd, &wset);

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        ready_n = select(tcp_fd + 1, &rset, &wset, NULL, &timeout);

        if (0 == ready_n)
        {
            errcode = 1;
        }
        else if (ready_n < 0)
        {
            IOT_ErrorLog("select error\r\n");
            errcode = -1;
        }
        else
        {
            if (FD_ISSET(tcp_fd, &wset) != 0)
            {
                errcode = 0;
            }
            else
            {
                if (0 != getsockopt(tcp_fd, SOL_SOCKET, SO_ERROR, &ret, (socklen_t *)&len))
                {
                    IOT_ErrorLog("getsocketopt failed\r\n");
                    errcode = -1;
                }

                if (0 != ret)
                {
                    errcode = -1;
                }

                errcode = -1;
            }
        }
        AilinkMutexUnLock();
    }

    return errcode;
}

/**
 * @brief   断开tcp连接
 * 
 * @param[in]   fd  [IN] tcp套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
void ailinkTcpDisconnect(int32_t fd)
{
    AilinkMutexLock();
    close(fd);
    AilinkMutexUnLock();

    IOT_DebugLog("tcp disconnect ok\r\n");
}


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
int32_t ailinkTcpSend(int32_t fd, const uint8_t* buf, uint16_t len)
{
    int ret = -1;

    if(buf == NULL || fd < 0)
    {
        IOT_ErrorLog("param err \r\n");
        return -1;
    }

    AilinkMutexLock();
    ret = (int)(send(fd, buf, len, MSG_DONTWAIT));
    AilinkMutexUnLock();

    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            return 0;
        }
        else
        {
            IOT_ErrorLog("ret=%d errno=%d\r\n", ret, errno);
            return -1;
        }
    }
    return ret;

}


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
int32_t ailinkTcpRead(int32_t fd, uint8_t* buf, uint16_t len)
{
    int ret = -1;

    if(buf == NULL || fd < 0)
    {
        IOT_ErrorLog("param err \r\n");
        return -1;
    }

    AilinkMutexLock();
    ret = (int)(recv(fd, buf, len, MSG_DONTWAIT));
    AilinkMutexUnLock();

    if (ret <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            ret = 0;
        }
        else
        {
            IOT_ErrorLog("ret=%d errno=%d\r\n", ret, errno);
        }
    }

    return ret;
}



static void mbedtls_ssl_dbg_callBack( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    if ((NULL == file) || (NULL == str))
    {
        return;
    }

    IOT_InfoLog("%s:%04d: %s", file, line, str);
}

static void AilinkMbedtlsSslFree()
{
    mbedtls_ssl_close_notify(&ssl);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    #if defined(AILINK_VERIFY)
    mbedtls_x509_crt_free(&cacert);
    #endif
    mbedtls_net_free(&server_fd);
}



/**
 * @brief   创建加密tcp连接
 * 
 * @par 描述:
 * 创建非阻塞模式加密tcp连接
 * 
 * @param[in]   dst             [IN] 接收方ip地址。
 * @param[in]   port            [IN] 接收方端口号。
 * @return  int32_t             
 *          大于0  连接成功,返回tcp套接字
 *          -1：连接失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSslConnect(const char* dst, uint16_t port)
{
    int ret;
    struct sockaddr_in servaddr;
    int flags;
    int reuse = 1;
    int err = 0;


    AilinkMutexLock();

    if (NULL == dst)
    {
        ret = -1;
    }
    else
    {
        mbedtls_ssl_init(&ssl);                 // 初始化ssl
        #if defined(AILINK_VERIFY)
        mbedtls_x509_crt_init( &cacert );       // 初始化根证书链表
        #endif
        mbedtls_ctr_drbg_init(&ctr_drbg);       // 初始化ssl随机字节发生器
        mbedtls_ssl_config_init(&conf);         // 初始化ssl配置
        mbedtls_entropy_init(&entropy);
        // mbedtls_debug_set_threshold(0);         // 设置调试级别，即输出不同级别的日志


        if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,NULL, 0)) != 0) 
        {
            ret = -1;
        }
        else
        {
#if defined(AILINK_VERIFY)
            // 解析根证书
            ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char *)ailink_server_crt,
                                        ailink_server_crt_len);
            if (ret < 0)
            {
                    IOT_ErrorLog("\r\n mbedtls_x509_crt_parse returned -0x%x\r\n", -ret);
            }
#endif
            if ((ret = mbedtls_ssl_config_defaults(&conf,
                                                MBEDTLS_SSL_IS_CLIENT,
                                                MBEDTLS_SSL_TRANSPORT_STREAM,
                                                MBEDTLS_SSL_PRESET_DEFAULT)) != 0) 
            {
                IOT_ErrorLog("mbedtls_ssl_config_defaults returned %d\r\n", ret);
                ret = -1;
            }
            else
            {
#if defined(AILINK_VERIFY)
                mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
                // mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
                mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );                  // 将受信的证书链配置到SSL配置结构体对象中
#else
                mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
#endif
                mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);        // 设置随机数生成器
                mbedtls_ssl_conf_read_timeout(&conf, 0);
                mbedtls_ssl_conf_dbg( &conf, mbedtls_ssl_dbg_callBack, stdout );

                if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) 
                {
                    IOT_ErrorLog("mbedtls_ssl_setup returned -0x%x\r\n", -ret);
                    ret = -1;
                }
                else
                {
                    mbedtls_net_init(&server_fd);           // 初始化网络环境，创建套接字fd
                    server_fd.fd = socket(AF_INET, SOCK_STREAM, 0);
                    IOT_ErrorLog("server_fd.fd:%d\r\n",server_fd.fd);
                
                    if (server_fd.fd < 0) 
                    {
                        IOT_ErrorLog("ssl creat socket fd failed\r\n");
                        mbedtls_ssl_close_notify(&ssl);  
                        mbedtls_net_free((mbedtls_net_context*)(ssl.p_bio));
#if defined(AILINK_VERIFY)
                        mbedtls_x509_crt_free( &cacert );
#endif
                        mbedtls_ssl_free( &ssl );
                        mbedtls_ssl_config_free( &conf );
                        mbedtls_ctr_drbg_free( &ctr_drbg );
                        mbedtls_entropy_free( &entropy );
                        ret = -1;
                    }
                    else
                    {
                        // flags = fcntl(server_fd.fd, F_GETFL, 0);

                        // if (flags < 0 || fcntl(server_fd.fd, F_SETFL, flags | O_NONBLOCK) < 0) 
                        // {
                        //     IOT_ErrorLog("ssl fcntl: %s\r\n", strerror(errno));
                        //     AilinkMbedtlsSslFree();
                        //     ret = -1;
                        // }
                        flags = 0;
                        IOT_InfoLog("server_fd.fd = %d \r\n", server_fd.fd);
                        if (ioctlsocket(server_fd.fd, FIONBIO, &flags) != 0)
                        {
                            IOT_ErrorLog("fcntl: %s\r\n", strerror(errno));
                            AilinkMbedtlsSslFree();
                            ret = -1;
                        }
                        else
                        {
                            if (setsockopt(server_fd.fd, SOL_SOCKET, SO_REUSEADDR,(const char*) &reuse, sizeof(reuse)) != 0) 
                            {
                                AilinkMbedtlsSslFree();
                                IOT_ErrorLog("ssl set SO_REUSEADDR failed\r\n");
                                ret = -1;
                            }
                            else
                            {
                                memset(&servaddr, 0, sizeof(struct sockaddr_in));
                                servaddr.sin_family = AF_INET;
                                servaddr.sin_addr.s_addr = inet_addr(dst);
                                servaddr.sin_port = htons(port);
                                // servaddr.sin_port = htons(1883);
                                // servaddr.sin_port = htons(8883);

                                err = connect(server_fd.fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in));
                                IOT_InfoLog("connect ret = %d\r\n",ret);
                                IOT_InfoLog("connect err = %d\r\n",err);
                                if (err == 0) 
                                {
                                    IOT_ErrorLog("dst %s errno %d ret %d \r\n", dst, errno, ret);
                                    IOT_InfoLog("server_fd.fd = %d \r\n", server_fd.fd);
                                    ret = (int32_t)&ssl;
                                    IOT_InfoLog("ioctlsocket \r\n");
                                    flags = 1;
                                    if (ioctlsocket(server_fd.fd, FIONBIO, &flags) != 0)
                                    {
                                        IOT_ErrorLog("fcntl: %s\r\n", strerror(errno));
                                        AilinkMbedtlsSslFree();
                                        ret = -1;
                                    }
                                } 
                                else 
                                {
                                    IOT_ErrorLog("ssl dst %s errno %d\r\n", dst, errno);
                                    // IOT_ErrorLog("dst %s port %d errno %d ret %d\r\n", dst, port, errno, ret);
                                    // AilinkMbedtlsSslFree();
                                    // ret = -1;
                                    if (EALREADY != errno && EINPROGRESS != errno) 
                                    {
                                        IOT_ErrorLog("connect fail\r\n");
                                        AilinkMbedtlsSslFree();
                                        ret = -1;
                                    }
                                    else
                                    {
                                        IOT_InfoLog("tcp ssl conncet noblock\r\n");
                                        IOT_InfoLog("ioctlsocket \r\n");
                                        flags = 1;
                                        if (ioctlsocket(server_fd.fd, FIONBIO, &flags) != 0)
                                        {
                                            IOT_ErrorLog("fcntl: %s\r\n", strerror(errno));
                                            AilinkMbedtlsSslFree();
                                            ret = -1;
                                        }
                                        else
                                        {
                                            ret = (int32_t)&ssl;
                                        }
                                        
                                    }

                                }
                                mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
                            }
                        }
                    }
                }
            }
        }
    }

    AilinkMutexUnLock();

    return ret;
}


/**
 * @brief   加密tcp连接状态
 * 
 * @par 描述:
 * 查询加密tcp连接状态
 * 
 * @param[in]   fd          [IN] tcp套接字。
 * @return  int32_t         返回tcp连接状态
 *          0：tcp已建立连接
 *          -1：参数错误或连接失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSslState(int32_t fd)
{
    if (fd < 0)
    {
        return -1;
    }
    
    int err = 0;
    mbedtls_ssl_context *pssl = (mbedtls_ssl_context *)fd;
    int tcp_fd = ((mbedtls_net_context*)(pssl->p_bio))->fd;
    int ret;
    fd_set rset, wset;
    int ready_n;
    struct timeval timeout;

    AilinkMutexLock();

    if (tcp_fd < 0 || NULL == pssl) 
    {
        IOT_ErrorLog("fd or ssl err\r\n");
        err = -1;
    }
    else
    {
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_CLR(tcp_fd, &rset);
        FD_CLR(tcp_fd, &wset);
        FD_SET(tcp_fd, &rset);
        FD_SET(tcp_fd, &wset);


        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        ready_n = select(tcp_fd + 1, &rset, &wset, NULL, &timeout);

        if (0 == ready_n)
        {
            err = -1;
        }
        else if (ready_n < 0)
        {
            err = -1;
        }
        else
        {
            if (FD_ISSET(tcp_fd, &wset) != 0)
            {
                err = -1;
                if(pssl->state != MBEDTLS_SSL_HANDSHAKE_OVER)
                {
                    ret = mbedtls_ssl_handshake_step( pssl );
                    if (0 != ret)
                    {
                        if (MBEDTLS_ERR_SSL_WANT_READ != ret) 
                        {
                            IOT_ErrorLog("mbedtls_ssl_handshake_step return = 0X%X\r\n", -ret);
                            err = -1;
                        }
                    }
                }
                else
                {
                    err = 0;
                }
            }
            else
            {
                int len = (int) sizeof(int);

                if (0 != getsockopt(tcp_fd, SOL_SOCKET, SO_ERROR, &ret, (uint32_t*)&len))
                {
                    err = -1;
                }

                if (0 != ret)
                {
                    err = -1;
                }
                err = -1;
            }
        }
    }

    AilinkMutexUnLock();

    return err;
}


/**
 * @brief   断开加密tcp连接。
 * 
 * @param[in]   fd   [IN] 套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
void ailinkTcpSslDisconnect(int32_t fd)
{
    if (fd < 0) 
    {
        return;
    }
    
    mbedtls_ssl_context *pssl = (mbedtls_ssl_context *)fd;

    if (NULL == pssl)
    {
        IOT_ErrorLog("param err \r\n");
        return ;
    }

    AilinkMutexLock();

    mbedtls_ssl_close_notify(pssl);  
    mbedtls_net_free((mbedtls_net_context*)(pssl->p_bio));
#if defined(AILINK_VERIFY)
    mbedtls_x509_crt_free( &cacert );
#endif
    mbedtls_ssl_free( pssl);
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    IOT_InfoLog("ssl disconnect ok\r\n");
    AilinkMutexUnLock();
}


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
 *          大于0：数据已发送成功
 *          小于或等于0：数据发送失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSslSend(int32_t fd, const uint8_t* buf, uint16_t len)
{
    if (fd < 0) 
    {
       return -1;
    }
    
    int ret = 0;
    mbedtls_ssl_context *pssl = (mbedtls_ssl_context *)fd;

    if ((NULL == buf) || (NULL == pssl))
    {
        return -1;
    }

    AilinkMutexLock();

    ret = mbedtls_ssl_write(pssl, buf, len);

    AilinkMutexUnLock();
    
    if(ret > 0)
    {
        return ret;
    }
    else if(MBEDTLS_ERR_SSL_TIMEOUT == ret)
    {
        IOT_ErrorLog("send timeout ret = 0X%X\r\n", -MBEDTLS_ERR_SSL_TIMEOUT);
        return MBEDTLS_ERR_SSL_TIMEOUT;
    }
    else if(MBEDTLS_ERR_SSL_WANT_WRITE == ret)
    {
        IOT_ErrorLog("send error = 0X%X\r\n", -MBEDTLS_ERR_SSL_WANT_WRITE);
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }
    else
    {
        IOT_ErrorLog("send error[%X]\r\n", -ret);
        return ret;;
    }

    return ret;
}


/**
 * @brief   读取加密tcp数据
 * 
 * @par 描述:
 * 非阻塞读取加密tcp数据
 * 
 * @param[in]   fd          [IN] tcp套接字。
 * @param[in]   buf         [IN] 指向存放接收数据缓冲区的指针。
 * @param[in]   len         [IN] 存放接收数据缓冲区的最大长度，范围为[0，512)。
 * @return  int32_t         
 *              大于0：返回数据长度
 *              0：表示未读取到数据
 *              小于0：参数错误
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkTcpSslRead(int32_t fd, uint8_t* buf, uint16_t len)
{
    int ret = 0;
    mbedtls_ssl_context *pssl = (mbedtls_ssl_context *)fd;

    if ((NULL == buf) || (NULL == pssl))
    {
        return -1;
    }

    AilinkMutexLock();

    ret = mbedtls_ssl_read(pssl, buf, len);

    AilinkMutexUnLock();

    if(ret > 0)
    {
        return ret;
    }
    else if((MBEDTLS_ERR_SSL_TIMEOUT == ret) || \
            (MBEDTLS_ERR_SSL_WANT_READ == ret))
    {
        return 0;
    }
    else
    {
        IOT_ErrorLog("ailinkTcpSslRead ret = 0X%X\n", -ret);
        return -1;
    }
}



/**
 * @brief   创建udp服务
 * 
 * @note    创建非阻塞模式udp服务
 * 
 * @param[in]   port     [IN] 接收方端口号。
 * @return  int32_t      等于0  建立成功，返回tcp套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
int32_t ailinkCreateUDPServer(uint16_t port)
{
    int32_t sock = -1;
    struct sockaddr_in dest_addr;
    int flags;
    int reuse;

    AilinkMutexLock();
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        AilinkMutexUnLock();
        IOT_ErrorLog("socket create fail[%d] \r\n", sock);
        return sock;
    }
    IOT_InfoLog("sock = %d \r\n", sock);

    
    flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        IOT_ErrorLog("fcntl: %s\n", strerror(errno));
        close(sock);
        AilinkMutexUnLock();
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                (const char*) &reuse, sizeof(reuse)) != 0)
    {
        close(sock);
        IOT_ErrorLog("set SO_REUSEADDR failed\n");
        AilinkMutexUnLock();
        return -2;
    }

    /* bind to port 80 at any interface */
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = INADDR_ANY;
    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if(err < 0)
    {
        IOT_ErrorLog("bind err \r\n");
        AilinkMutexUnLock();
        return err;
    }
    AilinkMutexUnLock();

    return sock;
}




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
int32_t ailinkCreateUDPClient(char *ip, uint16_t port)
{
    int32_t sock = -1;
    int flags;
    int reuse;

    AilinkMutexLock();
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        IOT_ErrorLog("socket create fail \r\n");
        AilinkMutexUnLock();
        return sock;
    }
    flags = fcntl(sock, F_GETFL, 0);

    if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        IOT_ErrorLog("fcntl: %s\n", strerror(errno));
        close(sock);
        AilinkMutexUnLock();
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                (const char*) &reuse, sizeof(reuse)) != 0)
    {
        close(sock);
        IOT_ErrorLog("set SO_REUSEADDR failed\n");
        AilinkMutexUnLock();
        return -2;
    }


    /* bind to port 80 at any interface */
    server_dest_addr.sin_family = AF_INET;
    server_dest_addr.sin_port = htons(port);
    server_dest_addr.sin_addr.s_addr = inet_addr(ip);

    AilinkMutexUnLock();

    return sock;
}




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
int32_t ailinkCreateUDPGroupClient(char *ip, uint16_t port)
{
    int32_t sock = -1;
    int flags;
    struct in_addr group_add;

    AilinkMutexLock();
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        IOT_ErrorLog("socket create fail \r\n");
        AilinkMutexUnLock();
        return sock;
    }
    flags = fcntl(sock, F_GETFL, 0);

    if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        IOT_ErrorLog("fcntl: %s\n", strerror(errno));
        close(sock);
        AilinkMutexUnLock();
        return -1;
    }

    group_add.s_addr = inet_addr("203.106.93.94");
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF,(const char*) &group_add, sizeof(group_add)) != 0)
    {
        close(sock);
        IOT_ErrorLog("set IP_MULTICAST_IF failed\n");
        AilinkMutexUnLock();
        return -2;
    }


    /* bind to port 80 at any interface */
    group_dest_addr.sin_family = AF_INET;
    group_dest_addr.sin_port = htons(port);
    group_dest_addr.sin_addr.s_addr = inet_addr(ip);

    AilinkMutexUnLock();

    return sock;
}



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
int32_t ailinkUDPServerSend(int32_t fd, const uint8_t* buf, uint16_t len)
{
    int32_t err = 0;

    AilinkMutexLock();
    err = sendto(fd, buf, len, 0, 
                    (struct sockaddr *)&source_addr, sizeof(source_addr));
    AilinkMutexUnLock();

    return err;
}


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
int32_t ailinkUDPServerRead(int32_t fd, uint8_t* buf, uint16_t len)
{
    socklen_t socklen = sizeof(source_addr);
    int32_t ret = 0;

    AilinkMutexLock();
    ret = recvfrom(fd, buf, len, 0, 
                        (struct sockaddr *)&source_addr, &socklen);
    AilinkMutexUnLock();

    return ret;
}




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
int32_t ailinkUDPClientSend(int32_t fd, const uint8_t* buf, uint16_t len)
{
    int32_t err = 0;

    AilinkMutexLock();
    err = sendto(fd, buf, len, 0, 
                    (struct sockaddr *)&server_dest_addr, sizeof(server_dest_addr));
    AilinkMutexUnLock();

    return err;
}


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
int32_t ailinkUDPClientRead(int32_t fd, uint8_t* buf, uint16_t len)
{
    socklen_t socklen = sizeof(server_dest_addr);
    int32_t ret = 0;

    AilinkMutexLock();
    ret = recvfrom(fd, buf, len, 0, 
                        (struct sockaddr *)&server_dest_addr, &socklen);
    AilinkMutexUnLock();

    return ret;
}


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
int32_t ailinkUDPGroupSend(int32_t fd, const uint8_t* buf, uint16_t len)
{
    int32_t err = 0;

    err = sendto(fd, buf, len, 0, 
                    (struct sockaddr *)&group_dest_addr, sizeof(group_dest_addr));

    return err;
}


/**
 * @brief  关闭UDP功能
 * 
 * @param[in]   fd  [IN] UDP套接字
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-12
 */
void ailinkUDPDisconnect(int32_t fd)
{
    close(fd);
}




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
char *ailinkGethostbyname(const char* hostname, uint8_t len)
{
    char* host_ip = NULL;
    ip_addr_t dns_ip = {0};
    int32_t err = 0;

    if(hostname == NULL)
    {
        IOT_ErrorLog("params err \r\n");
        return NULL;
    }

    IOT_InfoLog("host name : %s \n",hostname);
    AilinkMutexLock();
    err = netconn_gethostbyname(hostname, &dns_ip);
    AilinkMutexUnLock();
    if(err < 0)
    {
         IOT_ErrorLog("err = %d \r\n", err);
        return NULL;
    }
    AilinkMutexLock();
    IOT_InfoLog("err = %d \r\n", err);
    host_ip = ip_ntoa(&dns_ip);
    AilinkMutexUnLock();
    if(host_ip == NULL)
    {
        IOT_ErrorLog("Get ip err \r\n");
        return NULL;
    }
    IOT_InfoLog("host name : %s , host_ip : %s\n",hostname,host_ip);

    return host_ip;
}

