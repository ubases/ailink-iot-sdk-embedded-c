



#include "ailink_mdnsHal.h"

#include <lwip/sockets.h>
#include <utils_sha256.h>
#include <lwip/apps/mdns_priv.h>
#include <lwip/apps/mdns.h>
#include <lwip/netif.h>

#include "mdns_server.h"
#include <wifi_mgmr_ext.h>
#include <lwip/netifapi.h>
#include <hal_sys.h>
#include "iot_log.h"


static bool         mdns_init = false;
static struct       netif *netif = NULL;
static uint32_t     pre_ip = 0, cur_ip = 0;

static uint8_t      g_mdns_num = 0;
static char         g_data[TXT_KEY_MAX_LEN + TXT_VALUE_MAX_LEN] = {0};
static AilinkMdnsTxtItem g_txt[TXT_MAX_NUM] = {0};

AilinkMdnsConfig_t  mdns_config = {0};


/**
 * @brief   添加回复数据的回调函数
 * 
 * @param[in]   service             mdns服务
 * @param[in]   txt_userdata        
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
static void AilinkMdnsAddSrvTxt(struct mdns_service *service, void *txt_userdata);

/**
 * @brief   添加mdns服务
 * 
 * @param[in]   my_netif        网络句柄
 * @return  err_t 
 *          0：添加成功
 *          -1：添加失败
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
static err_t AilinkMdnsServiceStart(struct netif *my_netif);



/**
 * @brief   mdns初始化函数
 *
 *@note     初始化mdns,打开mdns服务。
 * 
 * @param[in]   config          mdns配置参数
 * @return  int32_t             
 *          0：初始化成功
 *          -1：初始化失败
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
int32_t AilinkMdnsInit(AilinkMdnsConfig_t config)
{
    int32_t ret = -1;

    if(mdns_init)
    {
        IOT_ErrorLog("mdns already init \r\n");
        return -1;
    }

    if ((NULL == config.host_name)  || (NULL == config.service_type))
    {
        IOT_ErrorLog("param err \r\n");
        return -1;
    }
    IOT_DebugLog("host name : %s\r\n",     config.host_name);
    IOT_DebugLog("port : %d\r\n",          config.port);
    IOT_DebugLog("server type : %s\r\n",   config.service_type);

    memset(&mdns_config, 0, sizeof(AilinkMdnsConfig_t));

    mdns_config.host_name = malloc(strlen(config.host_name) + 1);
    if(mdns_config.host_name == NULL)
    {
        IOT_ErrorLog("malloc failed\r\n");
        return -1;
    }
    mdns_config.service_type = malloc(strlen(config.service_type) + 1);
    if(mdns_config.service_type == NULL)
    {
        IOT_ErrorLog("malloc failed\r\n");
        free(mdns_config.host_name);
        mdns_config.host_name  = NULL;
        return -1;
    }

    memset(mdns_config.host_name, 0, strlen(config.host_name) + 1);
    memcpy(mdns_config.host_name, config.host_name, strlen(config.host_name));

    memset(mdns_config.service_type, 0, strlen(config.service_type) + 1);
    memcpy(mdns_config.service_type, config.service_type, strlen(config.service_type));

    mdns_config.port = config.port;

    IOT_DebugLog("host name : %s\r\n",     mdns_config.host_name);
    IOT_DebugLog("port : %d\r\n",          mdns_config.port);
    IOT_DebugLog("server type : %s\r\n",   mdns_config.service_type);

    netif = NULL;
    netif = wifi_mgmr_sta_netif_get();
    if (netif == NULL) 
    {
        IOT_ErrorLog("find failed\r\n");
        free(mdns_config.service_type);
        mdns_config.service_type = NULL;
        free(mdns_config.host_name);
        mdns_config.host_name  = NULL;
		return -1;
    }
    cur_ip = netif->ip_addr.addr;  
    pre_ip = cur_ip;

    IOT_InfoLog("pre_ip = 0x%08x \r\n", pre_ip);
    IOT_InfoLog("cur_ip = 0x%08x \r\n", cur_ip);

    ret = netifapi_netif_common(netif, NULL, AilinkMdnsServiceStart);
    if (ret < 0) 
    {
        IOT_ErrorLog("start mdns failed\r\n"); 
        mdns_resp_remove_netif(netif); 
        mdns_resp_deinit();
        netif = NULL;
        return ret;
    }

    mdns_init = true;

    return ret;

}


/**
 * @brief   mdns反初始化函数
 * @note    反初始化mdns，并释放相关资源
 * 
 * @return  int32_t 
 *          0：反初始化成功
 *          -1：反初始化失败
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
int32_t AilinkMdnsDeInit(void)
{
    if(!mdns_init)
    {
        IOT_ErrorLog("mdns already DeInit \r\n");
        return -1;
    }

    mdns_resp_remove_netif(netif); 
    mdns_resp_deinit();
    netif = NULL;

    if(mdns_config.service_type)
    {
        free(mdns_config.service_type);
        mdns_config.service_type = NULL;
    }

    if(mdns_config.host_name)
    {
        free(mdns_config.host_name);
        mdns_config.host_name  = NULL;
    }
    memset(&mdns_config, 0, sizeof(AilinkMdnsConfig_t));

    mdns_init = false;

    return 0;
}


/**
 * @brief   更新txt记录函数
 * 
 * @param[in]   service_type        服务类型
 * @param[in]   protocol            协议类型，例如：_tcp,_udp等
 * @param[in]   txt                 txt键值对数组
 * @param[in]   num_items           键值对数组长度
 * @return  int32_t 
 *          0：处理成功
 *          -1：处理失败
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
int32_t AilinkMdnsUpdateTxt(char *service_type, uint16_t protocol, AilinkMdnsTxtItem txt[], uint8_t num_items)
{
    uint16_t i = 0;

    if((NULL == service_type) || (NULL == txt))
    {
        IOT_ErrorLog("arg error \r\n");
        return -1;
    }
    if(NULL == netif)
    {
        IOT_ErrorLog("netif is NULL \r\n");
        return -1;
    }
    cur_ip = netif->ip_addr.addr;
    if (pre_ip != cur_ip) 
    {
        IOT_InfoLog("pre_ip = 0x%08x \r\n", pre_ip);
        IOT_InfoLog("cur_ip = 0x%08x \r\n", cur_ip);
        netifapi_netif_common(netif, mdns_resp_announce, NULL);
        pre_ip = cur_ip;
    }

    for (i = 0; i < TXT_MAX_NUM; i++)
    {
        memset(g_txt[i].key, 0, TXT_KEY_MAX_LEN);
        memset(g_txt[i].value, 0, TXT_VALUE_MAX_LEN);
    }
    g_mdns_num = num_items;
    for (i = 0; i < num_items; i++)
    {
        if(txt[i].key)
        {
            strncpy(g_txt[i].key, txt[i].key, sizeof(g_txt[i].key));
            strncpy(g_txt[i].value, txt[i].value, sizeof(g_txt[i].value));
        }
    }
    netifapi_netif_common(netif, mdns_resp_announce, NULL);
    netifapi_netif_common(netif, mdns_resp_announce, NULL);
    netifapi_netif_common(netif, mdns_resp_announce, NULL);

    return 0;
}


/**
 * @brief   添加mdns服务
 * 
 * @param[in]   my_netif        网络句柄
 * @return  int32_t 
 *          0：添加成功
 *          -1：添加失败
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
static err_t AilinkMdnsServiceStart(struct netif *my_netif)
{
    int32_t ret = 1;

    if ((NULL == mdns_config.host_name) ||  (NULL == mdns_config.service_type))
    {
        IOT_ErrorLog("param err \r\n");
        return -1;
    }

    if(my_netif == NULL)
    {
        IOT_ErrorLog("netif is NULL \r\n");
        return -1;
    }
    mdns_resp_init(); 
	ret = mdns_resp_add_netif(my_netif, mdns_config.host_name, 3600);
    if (ret != 0) 
    {
        mdns_resp_deinit();
        IOT_ErrorLog("add my_netif failed:%d\r\n", ret);
        return -1;
    }
    
	ret = mdns_resp_add_service(my_netif, mdns_config.host_name, mdns_config.service_type, 
                                DNSSD_PROTO_UDP, mdns_config.port, 3600, AilinkMdnsAddSrvTxt, NULL);
	IOT_InfoLog("mdns_responder_start ret = %d \r\n", ret);

	if (ret < 0) 
    {
        mdns_resp_remove_netif(my_netif); 
        mdns_resp_deinit();
        IOT_ErrorLog("add server failed:%d\r\n", ret);
        return -1;
    }
	
    return ret;

}

/**
 * @brief   添加回复数据的回调函数
 * 
 * @param[in]   service             mdns服务
 * @param[in]   txt_userdata        
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-05-15
 */
static void AilinkMdnsAddSrvTxt(struct mdns_service *service, void *txt_userdata)
{
    uint8_t i = 0;
    int32_t ret = 0;
    
    for (i = 0; i < g_mdns_num; i++)
    {
        if((strlen(g_txt[i].key) > 0) && (strlen(g_txt[i].value) > 0))
        {
            memset(g_data, 0, TXT_KEY_MAX_LEN + TXT_VALUE_MAX_LEN);
            snprintf(g_data, sizeof(g_data), "%s=%s", g_txt[i].key, g_txt[i].value);
            IOT_InfoLog("g_data = %s\r\n", g_data);
            ret = mdns_resp_add_service_txtitem(service, g_data, strlen(g_data));
            if(ret < 0)
            {
                IOT_ErrorLog("mdns update txt error \r\n");
            }
        }
    }
}
