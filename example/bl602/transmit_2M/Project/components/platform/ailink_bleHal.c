/**
 * @file    ailink_blepair.c
 * @brief   该文件用于适配ble配网必备的函数接口
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


#include <zephyr/types.h>
#include "bluetooth.h"
#include "ble_cli_cmds.h"
#include "hci_driver.h"
#include "ble_lib_api.h"
#include <conn.h>
#include <gatt.h>
#include <uuid.h>
#include <blog.h>
#include <stdio.h>
#include "hci_core.h"
#include "bl_efuse.h"
#include <bl_sys_time.h>
#include <bl_timer.h>
#include <mbedtls/md5.h>
#include <mbedtls/aes.h>
#include <mbedtls/cipher.h>
#include <mbedtls/base64.h>


#include "ailink_bleHal.h"
#include "iot_log.h"



#define USER_BT_UUID_WIFI_PROV          BT_UUID_DECLARE_16(0xFFFF)
#define BT_UUID_WIFIPROV_WRITE          BT_UUID_DECLARE_16(0xFF01)
#define BT_UUID_WIFIPROV_READ           BT_UUID_DECLARE_16(0xFF02)

#define     AILINK_BLE_DATA_MAX_LEN             1024


struct bt_conn *ble_conn = NULL;                /**< ble 连接句柄 */
struct bt_gatt_exchange_params exchg_mtu;       /**< 改变ble的mtu值的句柄 */
int tx_mtu_size = 50;                           /**< 设备ble的mtu值 */
static bool ble_connect_status = false;              /**< ble连接状态，true：已连接，false:未连接 */
static bool ble_adv_status = false;         /**< ble广播状态， true：广播已开始，false：广播未开始*/

static uint8_t ble_receive_buf[AILINK_BLE_DATA_MAX_LEN] = {0};
static uint16_t ble_receive_len = 0;
static bool     ble_stack_init = false;
static bool     ble_init       = false;



static struct bt_data user_ad_discov[2] = 
{
	BT_DATA_BYTES(BT_DATA_FLAGS,(BT_LE_AD_NO_BREDR | BT_LE_AD_GENERAL)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, 13),
};



static ssize_t write_data(struct bt_conn *conn,
                   const struct bt_gatt_attr *attr, const void *buf,
                   u16_t len, u16_t offset, u8_t flags);

static ssize_t read_data(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr, void *buf,
                         u16_t len, u16_t offset);

static void wifiprov_ccc_cfg_changed(const struct bt_gatt_attr *attr,u16_t value);


static struct bt_gatt_attr attrs[]= 
{
    BT_GATT_PRIMARY_SERVICE(USER_BT_UUID_WIFI_PROV),
    BT_GATT_CHARACTERISTIC(BT_UUID_WIFIPROV_READ,
                                BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                                BT_GATT_PERM_READ ,
                                read_data,
                                NULL,
                                NULL),
    BT_GATT_CCC(wifiprov_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),


    BT_GATT_CHARACTERISTIC(BT_UUID_WIFIPROV_WRITE,
                                BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                                BT_GATT_PERM_WRITE,
                                NULL,
                                write_data,
                                NULL),
};


const static struct bt_gatt_service ailinkble_server = BT_GATT_SERVICE(attrs);



static ssize_t read_data(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr, void *buf,
                         u16_t len, u16_t offset)
{

    IOT_InfoLog("***************  read_data  **************\n");

    ssize_t ret = BT_GATT_ERR(BT_ATT_ERR_INVALID_PDU);
    char pResp[20] = "ble pair";
    IOT_InfoLog("read_data\r\n");



    ret = bt_gatt_attr_read(conn, attr, buf, len, offset, pResp, strlen(pResp));

    return ret;

}

static ssize_t write_data(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, u16_t len, u16_t offset, u8_t flags)
{

    IOT_InfoLog("***************  write_data  **************\n");

    IOT_InfoLog("write length =  %d\r\n", len);
    IOT_InfoLog("offset  =  %d\r\n", offset);
    IOT_InfoLog("flags  =  %d\r\n", flags);

    if(buf != NULL)
    {
        memcpy(ble_receive_buf + offset, buf, len);
    }

    ble_receive_len = len;
    // IOT_InfoLog("ble_receive_buf = %s \r\n", ble_receive_buf);

    return len;
}



static void ble_tx_mtu_size(struct bt_conn *conn, u8_t err, struct bt_gatt_exchange_params *params)
{
    IOT_InfoLog("***************  ble_tx_mtu_size  **************\n");
       if(!err)
       {
                tx_mtu_size = bt_gatt_get_mtu(ble_conn);
                IOT_DebugLog("ble tp echange mtu size success, mtu size: %d\r\n", tx_mtu_size);
       }
       else
       {
                IOT_ErrorLog("ble tp echange mtu size failure, err: %d\r\n", err);
       }
}

static void wifiprov_ccc_cfg_changed(const struct bt_gatt_attr *attr,u16_t value)
{
    IOT_InfoLog("***************  wifiprov_ccc_cfg_changed  **************\n");
}

static void ble_connected(struct bt_conn *conn, u8_t err)
{
    int32_t ret = 0;

    IOT_InfoLog("***************  ble_connected  **************\n");

    ble_conn = conn;
    ble_connect_status = true;

    exchg_mtu.func = ble_tx_mtu_size;
    ret = bt_gatt_exchange_mtu(ble_conn, &exchg_mtu);
    if (!ret) 
    {
        IOT_DebugLog("ble tp exchange mtu size pending.\r\n");
    } 
    else 
    {
        IOT_ErrorLog("ble tp exchange mtu size failure, err: %d\r\n", ret);
    }

}



static void ble_disconnected(struct bt_conn *conn, u8_t reason)
{ 

    IOT_InfoLog("%s\n",__func__);

    IOT_ErrorLog(" ble_disconnected \n");
    IOT_DebugLog(" reason =  %d \n", reason);

    ble_connect_status = false;
    ble_adv_status = false;

}



static struct bt_conn_cb ble_conn_callbacks = 
{
    .connected    =   ble_connected,
    .disconnected    =   ble_disconnected,
};


/**
 * @brief   该函数是根据不同平台进行适配ble协议栈的初始化。即调用该函数后，
 *          该平台的ble协议栈已完成初始化。
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleStackInit(void)
{

    if(ble_init)
    {
        IOT_ErrorLog("ble stack already init \r\n");
        return ;
    }

    ble_controller_init(configMAX_PRIORITIES - 1);
    hci_driver_init();
    bt_enable(NULL);

    if(!ble_stack_init)
    {
        bt_conn_cb_register(&ble_conn_callbacks);
        ble_stack_init = true;
    }
    bt_gatt_service_register((struct bt_gatt_service *)&ailinkble_server);

    ble_init = true;

}


/**
 * @brief   该函数是根据不同平台进行适配ble协议栈的反初始化。即调用该函数后，
 *          该平台的ble协议栈已完成反初始化，并释放ble功能占用的资源。
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleStackDeinit(void)
{
    int err = 0;

    if(!ble_init)
    {
        IOT_ErrorLog("ble stakc already deinit \r\n");
        return ;
    }

    ble_init = false;
    if(ailinkBleIsConnect())
    {
        IOT_DebugLog("Disable bt successfully \r\n");
        ailinkBleDisconnect();
    }
    else
    {
        ble_conn = NULL;
    }
    bt_gatt_service_unregister((struct bt_gatt_service *)&ailinkble_server);

    err = bt_disable();
    if(err)
    {
        IOT_ErrorLog("Fail to disable \r\n");
        
    }
    else
    {
        IOT_DebugLog("Disable bt successfully \r\n");
    }
}


/**
 * @brief   该函数是根据不同平台进行适配开启ble广播功能。即调用该函数后，ble便开始
 *          广播指定设备名，并等待连接中。
 * 
 * @note    1. 该广播需设置为可连接不定向广播
 *          2. 该广播的间隔间隔时候需由用户根据需求进行设置
 * 
 * @param[in]   advName                 广播名
 * @param[in]   advName_len             广播名长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleAdvStart(char advName[], uint16_t advName_len)
{
    struct bt_le_adv_param param;
	const struct bt_data *ad;
	size_t ad_len;
	int err = 0;

    param.id = BT_ID_DEFAULT;
    param.interval_min = BT_GAP_ADV_FAST_INT_MAX_1;
    param.interval_max = BT_GAP_ADV_FAST_INT_MAX_1;

    param.options = (BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME | BT_LE_ADV_OPT_ONE_TIME);

    // struct bt_data gen_disc_data = (struct bt_data)BT_DATA_BYTES(BT_DATA_FLAGS,(BT_LE_AD_NO_BREDR | BT_LE_AD_GENERAL));
    // user_ad_discov[0] = gen_disc_data;

    IOT_InfoLog("advName = %s\r\n", advName);
    IOT_InfoLog("strlen(advName) = %d\r\n", strlen(advName));
    IOT_InfoLog("advName_len = %d\r\n", advName_len);

    bt_set_name(advName);
    struct bt_data data = (struct bt_data)BT_DATA(BT_DATA_NAME_COMPLETE,advName, advName_len);
    user_ad_discov[0] = data;

    ad = user_ad_discov;
    ad_len = ARRAY_SIZE(user_ad_discov);
    IOT_InfoLog("Advertising ad_len = %d\r\n", ad_len);

    // param.interval_min = 0x100;
    // param.interval_max = 0x100;

    IOT_InfoLog("Advertising started set\r\n");
    err = bt_le_adv_start(&param, ad, 1, NULL, 0);	

    if(err)
    {
        IOT_ErrorLog("Failed to start advertising (err %d) \r\n", err);
    }
    else
    {
        ble_adv_status = true;
        IOT_DebugLog("Advertising started\r\n");
    }

}

/**
 * @brief   该函数是根据不同平台进行适配停止ble广播功能。即调用该函数后，
 *          ble需停止广播；并释放占用的资源。
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleAdvStop(void)
{
    int err = 0;

    err = bt_le_adv_stop();
    if(err)
    {
        ble_adv_status = false;
        IOT_ErrorLog("Failed to stop advertising\r\n");	
    }
    else
    { 
        IOT_DebugLog("Advertising stopped\r\n");
    }

}

/**
 * @brief   该函数是根据不同平台进行适配断开ble连接功能。即调用该函数后，
 *          设备将主动断开ble连接。
 * 
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleDisconnect(void)
{
    if(ble_conn != NULL)
    {
        if(ailinkBleIsConnect())
        {
            if(bt_conn_disconnect(ble_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN))
            {
                IOT_ErrorLog("Disconnection failed\r\n");
            }
            else
            {
                ble_conn = NULL;
                IOT_DebugLog("Disconnect successfully\r\n");
            }
        }
    }
}

/**
 * @brief   该函数将返回ble连接的状态
 * 
 * @return  true        ble已连接
 * @return  false       ble未连接
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
bool ailinkBleIsConnect(void)
{
    return ble_connect_status;
}

/**
 * @brief   该函数将适配ble发送数据接口。即调用该函数，可将数据通过ble发送出去。
 * 
 * @param[in]   data      待发送的数据
 * @param[in]   data_len  待发送数据的长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
void ailinkBleSendData(uint8_t *data, uint16_t data_len)
{
    if(ble_conn == NULL)
    {
        IOT_ErrorLog("ble is not connect\r\n");
        return ;
    }
    if(data == NULL)
    {
        IOT_ErrorLog("param err\r\n");
        return;
    }
    bt_gatt_notify(ble_conn,&attrs[2],data,data_len);
}

/**
 * @brief   该函数用于接收ble数据。
 * 
 * @param[in]   databuf             用于接收ble数据的缓存区
 * @param[in]   max_data_len        接收ble数据缓冲区的最大长度
 * @return  uint16_t                返回ble数据长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-10
 */
uint16_t ailinkBleReceiveData(uint8_t *databuf, uint16_t max_data_len)
{
    uint16_t len = ble_receive_len;

    if(databuf == NULL || ble_receive_len > max_data_len)
    {
        return 0;
    }
    memcpy(databuf, ble_receive_buf, ble_receive_len);

    memset(ble_receive_buf, 0, sizeof(ble_receive_buf));
    ble_receive_len = 0;

    return len;
}



/**
 * @brief   获取ble广播状态
 * 
 * @return  true ble广播已开始
 * @return  false  ble广播已停止
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-05-11
 */
bool ailinkBleAdvStatus(void)
{
    return ble_adv_status;
}




