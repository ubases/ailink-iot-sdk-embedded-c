/**
 * @file    ailink_ota_hal.c
 * @brief   
 * 
 * @author  Julius (juliuswy@163.com)
 * @date    2022-06-12
 * 
 * @copyright Copyright (c) 2022  Personal
 * 
 * @par 修改日志
 * Date      |Version|Author          |Description
 * ----------|-------|----------------|--------------
 * 2022-06-12|1.0.0  |Julius          |创建
 */
#include "ailink_otaHal.h"
#include "ailink_ota.h"
#include <utils_sha256.h>
#include <hal_boot2.h>
#include <hal_sys.h>
#include <bl_sys_ota.h>
#include <bl_mtd.h>
#include "ailink_profile.h"
#include "ailink_osHal.h"
#include "ailink_flashHal.h"
#include "ailink_wdtHal.h"
#include "iot_log.h"
#include "iot_protocol.h"

#define OTA_PROGRAM_SIZE                    (512)

#define   OTA_PROGRESS_DOWNLOADING              "Downloading"
#define   OTA_PROGRESS_INSTALLING               "Installing"


#define   OTA_INSTALL_FLAG                      "ota_flag"


static bl_mtd_handle_t handle = NULL;
static Ailink_mutex_t ailink_metex = NULL;
static iot_sha256_context ctx;
static uint8_t sha256_result[32];
static uint8_t sha256_img[32];
static HALPartition_Entry_Config ptEntry = {0};
static uint32_t ota_addr = 0;
static uint32_t bin_size = 0;
static uint32_t bin_max_size = 0;
static uint32_t part_size = 0;
static uint32_t ota_rate = 0;
static uint32_t last_ota_rate = 0;
static bool otaConfigState = false;
static  char reportTopic[100] = {0};
static ailinkOtaProInfo  *OtaProInfo = NULL;

typedef struct ota_header {
    union {
        struct {
            uint8_t header[16];

            uint8_t type[4];//RAW XZ
            uint32_t len;//body len
            uint8_t pad0[8];

            uint8_t ver_hardware[16];
            uint8_t ver_software[16];

            uint8_t sha256[32];
        } s;
        uint8_t _pad[512];
    } u;
} ota_header_t;


#define OTA_HEADER_SIZE (sizeof(ota_header_t))




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
 * @brief   获取ota安装状态
 * 
 * @return  true        ota已安装成功
 * @return  false       ota安装失败
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-20
 */
bool AilinkGetOtaInstallStatus(void)
{
    char status[3] = {0};

    AilinkFlashRead(OTA_INSTALL_FLAG, (uint8_t *)status, 2);
    if(strcmp(status, "ok") == 0)
    {
        IOT_DebugLog("ota already install ok \r\n");
        return true;
    }
    else
    {
        IOT_DebugLog("not ota install \r\n");
        return false;
    }
}

/**
 * @brief   设置ota安装状态
 * 
 * @param[in]   ota_status
 *              true: ota已安装成功
 *              false：ota未安装成功
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-20
 */
void AilinkSetOtaInstallStatus(bool ota_status)
{
    char status[3] = {0};

    if(ota_status)
    {
        memcpy(status, "ok", 2);
        IOT_DebugLog("set ota status \r\n");
        AilinkFlashWrite(OTA_INSTALL_FLAG, (uint8_t *)status, sizeof(status));
    }
    else
    {
        memset(status, 0, sizeof(status));
        IOT_DebugLog("clear ota status \r\n");
        AilinkFlashWrite(OTA_INSTALL_FLAG, (uint8_t *)status, sizeof(status));
    }
}



static int32_t otaStartConfig(void)
{
    uint8_t activeID;
    uint8_t ret = 0;
    profile_info_t *profile_info = NULL;


    memset(&ctx, 0, sizeof(ctx));
    memset(&sha256_result, 0, sizeof(sha256_result));
    memset(&sha256_img, 0, sizeof(sha256_img));
    memset(&ptEntry, 0, sizeof(ptEntry));
    ota_addr = 0;
    bin_size = 0;
    bin_max_size = 0;
    part_size = 0;
    ota_rate = 0;

    profile_info = AilinkGetProfileInfo();
    if(profile_info == NULL)
    {
        IOT_ErrorLog("get profile info err \r\n");
        return -1;
    }

    snprintf(reportTopic, 100, "%s/%s/upgrade/report",profile_info->productid, profile_info->deviceid);
    IOT_InfoLog("reportTopic = %s \r\n", reportTopic);

    AilinkMutexLock();
    ret = bl_mtd_open(BL_MTD_PARTITION_NAME_FW_DEFAULT, &handle, BL_MTD_OPEN_FLAG_BACKUP);
    if (ret) 
    {
        IOT_ErrorLog("Open Default FW partition failed\r\n");
        return -1;
    }
    activeID = hal_boot2_get_active_partition();                                    //初始化OTA分区
    IOT_InfoLog("activeID =  %d \r\n", activeID);
    if (hal_boot2_get_active_entries(BOOT2_PARTITION_TYPE_FW, &ptEntry))            //获取OTA分区
    {
        IOT_ErrorLog("hal_boot2_get_active_entries fail\r\n");
        bl_mtd_close(handle);
        return -1;
    }
    ota_addr = ptEntry.Address[!ptEntry.activeIndex];//OTA的地址
    bin_size = ptEntry.maxLen[!ptEntry.activeIndex];//bin的大小的最大值
    bin_max_size = ptEntry.maxLen[!ptEntry.activeIndex];//bin的大小的最大值
    part_size = ptEntry.maxLen[!ptEntry.activeIndex];//页的大小的最大值

    IOT_DebugLog("ota_addr =  0x%08lx \r\n", ota_addr);
    IOT_DebugLog("code addr =  0x%08lx \r\n", ptEntry.Address[ptEntry.activeIndex]);
    IOT_DebugLog("bin_size =  %ld \r\n", bin_size);
    IOT_DebugLog("bin_max_size =  %ld \r\n", bin_max_size);
    IOT_DebugLog("part_size =  %ld \r\n", part_size);
    IOT_DebugLog("ptEntry.activeIndex =  %d \r\n", ptEntry.activeIndex);

    bl_mtd_erase_all(handle);   //清除某个页
    IOT_InfoLog("Done\r\n");
    utils_sha256_init(&ctx);//sha256初始化
    utils_sha256_starts(&ctx);//开始加密

    AilinkMutexUnLock();

    otaConfigState = true;
    IOT_InfoLog("Done\r\n");
    return 0;

}

static int32_t AilinkCheckOtaHeader(ota_header_t *ota_header, uint32_t *ota_len, int16_t *use_xz)
{
    char str[33];//assume max segment size
    int i;

    memcpy(str, ota_header->u.s.header, sizeof(ota_header->u.s.header));
    str[sizeof(ota_header->u.s.header)] = '\0';
    IOT_InfoLog("[OTA] [HEADER] ota header is ");
    IOT_InfoLog("str = %s \r\n", str);
    IOT_InfoLog("\r\n");

    memcpy(str, ota_header->u.s.type, sizeof(ota_header->u.s.type));
    str[sizeof(ota_header->u.s.type)] = '\0';
    IOT_InfoLog("[OTA] [HEADER] file type is ");
    IOT_InfoLog("str = %s \r\n", str);
    IOT_InfoLog("\r\n");
    if (strstr(str, "XZ")) 
    {
        *use_xz = 1;
    } 
    else if(strstr(str, "RAW"))
    {
        *use_xz = 0;
    }
    else
    {
        IOT_ErrorLog("ota file format err\r\n");
        return -1;
    }

    memcpy(ota_len, &(ota_header->u.s.len), 4);
    IOT_InfoLog("[OTA] [HEADER] file length (exclude ota header) is %lu\r\n", *ota_len);

    memcpy(str, ota_header->u.s.ver_hardware, sizeof(ota_header->u.s.ver_hardware));
    str[sizeof(ota_header->u.s.ver_hardware)] = '\0';
    IOT_InfoLog("[OTA] [HEADER] ver_hardware is ");
    IOT_InfoLog("str = %s \r\n", str);
    IOT_InfoLog("\r\n");

    memcpy(str, ota_header->u.s.ver_software, sizeof(ota_header->u.s.ver_software));
    str[sizeof(ota_header->u.s.ver_software)] = '\0';
    IOT_InfoLog("[OTA] [HEADER] ver_software is ");
    IOT_InfoLog("str = %s \r\n", str);
    IOT_InfoLog("\r\n");

    memcpy(str, ota_header->u.s.sha256, sizeof(ota_header->u.s.sha256));
    str[sizeof(ota_header->u.s.sha256)] = '\0';
    IOT_InfoLog("[OTA] [HEADER] sha256 is ");
    for (i = 0; i < sizeof(ota_header->u.s.sha256); i++) {
        printf("%02X", str[i]);
    }
    printf("\r\n");

    return 0;
}

/**
 * @brief   ota状态回调函数
 * 
 * @param[in]   status  ota状态参数
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-16
 */
void AilinkOtaStatusCallback(uint8_t status)
{
    switch (status)
    {
        case AILINK_OTA_STATUS_SUCCESS:
        {
            IOT_InfoLog("AILINK_OTA_STATUS_SUCCESS \r\n");
            AilinkOtaDeInit();
            otaConfigState = false;
        }
        break;

        case AILINK_OTA_STATUS_FAIL:
        {
            IOT_InfoLog("AILINK_OTA_STATUS_FAIL \r\n");

            OtaProInfo = IotGetOtaProInfo();
            AilinkPublicReportProgress(1, NULL, OtaProInfo->OtaVer,OtaProInfo->PubId,0xffffffff);
            if(otaConfigState)
            {
                utils_sha256_free(&ctx);
                bl_mtd_close(handle);
            }
            AilinkWdtInit();
            Ailink_thread_delay(2000);
            AilinkOtaDeInit();
            otaConfigState = false;
        }
        break;

        case AILINK_OTA_STATUS_TIMEOUT:
        {
            IOT_InfoLog("AILINK_OTA_STATUS_TIMEOUT \r\n");
            OtaProInfo = IotGetOtaProInfo();
            AilinkPublicReportProgress(1, NULL, OtaProInfo->OtaVer,OtaProInfo->PubId,0xffffffff);
            if(otaConfigState)
            {
                utils_sha256_free(&ctx);
                bl_mtd_close(handle);
            }
            AilinkWdtInit();
            Ailink_thread_delay(2000);
            AilinkOtaDeInit();
            otaConfigState = false;
        }
        break;

        case AILINK_OTA_STATUS_STRART:
        {
            IOT_InfoLog("AILINK_OTA_STATUS_STRART \r\n");
            AilinkWdtDeinit();
            otaStartConfig();

        }
        break;
        
        default:
            break;
    }
}

/**
 * @brief   ota固件数据的输出回调函数
 * 
 * @param[in]   offset          ota固件数据偏移大小
 * @param[in]   data            ota固件数据
 * @param[in]   length          ota固件数据长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-16
 */
void AilinkOtaDownloadCallback(uint32_t offset, uint8_t *data, uint32_t length)
{
    uint8_t *recv_buffer = NULL;
    int8_t flag = 0;
    int16_t use_xz = 0;
    ota_header_t *ota_header;
    uint32_t flash_offset = 0;
    uint32_t ret = 0;
    uint32_t i = 0;

    if(data == NULL)
    {
        IOT_ErrorLog("param err \r\n");
        return ;
    }
    // IOT_InfoLog("length = %ld \r\n", length);
    // IOT_InfoLog("offset = %ld \r\n", offset);

    if(offset == 0)
    {
        recv_buffer = (uint8_t *)malloc(OTA_PROGRAM_SIZE + 1);
        if(recv_buffer == NULL)
        {
            IOT_ErrorLog("malloc fail \r\n");
            return ;
        }
        memset(recv_buffer, 0, OTA_PROGRAM_SIZE + 1);
        memcpy(recv_buffer, data, OTA_PROGRAM_SIZE);
        ota_header = (ota_header_t*)recv_buffer;

        AilinkMutexLock();
        flag = AilinkCheckOtaHeader(ota_header, &bin_size, &use_xz);
        AilinkMutexUnLock();
        IOT_InfoLog("flag = %d \r\n", flag);
        IOT_InfoLog("bin_size = %ld \r\n", bin_size);
        IOT_InfoLog("use_xz = %d \r\n", use_xz);
        IOT_InfoLog("bin_max_size = %ld \r\n", bin_max_size);

        if(flag < 0 || use_xz != 1)
        {
            IOT_ErrorLog("ota file format err \r\n");
            free(recv_buffer);
            return ;
        }

        memcpy(sha256_img, ota_header->u.s.sha256, sizeof(sha256_img));
        IOT_InfoLog("[OTA] [TCP] Update bin_size to %lu, file status %s\r\n", bin_size, use_xz ? "XZ" : "RAW");
        IOT_InfoLog("sha256_img : \r\n");
        for (i = 0; i < sizeof(sha256_img); i++) 
        {
            printf("%02X", sha256_img[i]);
        }
        printf("\r\n");
        if(bin_size > bin_max_size)
        {
            IOT_ErrorLog("ota file too big \r\n");
            free(recv_buffer);
            return ;
        }
        free(recv_buffer);
        ret += OTA_PROGRAM_SIZE;
        flash_offset = 0;
    }
    else
    {
    	flash_offset = offset - 512;

        // hosal_ota_erase(flash_offset, length);
        // IOT_InfoLog("flash_offset = %ld \r\n", flash_offset);
    }

    while (ret < length)
    {
        if(length - ret >= OTA_PROGRAM_SIZE)
        {
            AilinkMutexLock();
            utils_sha256_update(&ctx, &data[ret], OTA_PROGRAM_SIZE);//加密OTA 每次加密512字节
            bl_mtd_write(handle, flash_offset, OTA_PROGRAM_SIZE, &data[ret]);//开始往分区写
            flash_offset += OTA_PROGRAM_SIZE;
            ret += OTA_PROGRAM_SIZE;
            AilinkMutexUnLock();
            // IOT_InfoLog("flash_offset = %ld \r\n", flash_offset);
            // IOT_InfoLog("ret = %ld \r\n", ret);
        }
        else
        {
            AilinkMutexLock();
            utils_sha256_update(&ctx, &data[ret], length - ret);//加密OTA 每次加密512字节
            bl_mtd_write(handle, flash_offset, length - ret, &data[ret]);//开始往分区写
            flash_offset += length - ret;
            ret += length - ret;
            AilinkMutexUnLock();
            // IOT_InfoLog("flash_offset = %ld \r\n", flash_offset);
            // IOT_InfoLog("ret = %ld \r\n", ret);
        }
    }

    if(bin_size == flash_offset)
    {
        IOT_InfoLog("flash_offset = %ld \r\n", flash_offset);
        IOT_InfoLog("receive all ota file data \r\n");
        ota_rate = (flash_offset * 100) / bin_size;
        if(ota_rate % 10 == 0)
        {
            IOT_InfoLog("ota_rate = %ld \r\n", ota_rate);
            IOT_InfoLog("flash_offset = %ld \r\n", flash_offset);
            IOT_InfoLog("bin_size = %ld \r\n", bin_size);

            OtaProInfo = IotGetOtaProInfo();
            AilinkPublicReportProgress(0, OTA_PROGRESS_DOWNLOADING, OtaProInfo->OtaVer,OtaProInfo->PubId,ota_rate);
        }
        
        utils_sha256_finish(&ctx, sha256_result);//加密完成
        IOT_InfoLog("\r\nCalculated SHA256 Checksum: \r\n");
        for (i = 0; i < sizeof(sha256_result); i++) 
        {
            printf("%02X", sha256_result[i]);
        }
        printf("\r\n");
        printf("\r\n");
        printf("\r\n");
        printf("\r\nHeader SET SHA256 Checksum:");
        for (i = 0; i < sizeof(sha256_img); i++) 
        {
            printf("%02X", sha256_img[i]);
        }
        printf("\r\n");
        printf("\r\n");
        printf("\r\n");
        if (memcmp(sha256_img, sha256_result, sizeof(sha256_img))) 
        {
            /*Error found*/
            IOT_ErrorLog("[OTA] [TCP] SHA256 NOT Correct\r\n");
            OtaProInfo = IotGetOtaProInfo();
            AilinkPublicReportProgress(4, NULL, OtaProInfo->OtaVer,OtaProInfo->PubId,0xffffffff);
            return ;
        }
        utils_sha256_free(&ctx);

        OtaProInfo = IotGetOtaProInfo();
        AilinkPublicReportProgress(0, OTA_PROGRESS_INSTALLING, OtaProInfo->OtaVer,OtaProInfo->PubId,0xffffffff);
        AilinkSetOtaInstallStatus(true);
        // bl_mtd_close(handle);
        // Ailink_thread_delay(2000);

        ptEntry.len = bin_size;
        IOT_DebugLog("--------------SUCCES------------------\r\n");  
        hal_boot2_update_ptable(&ptEntry);//结束。
        IOT_DebugLog("--------------SUCCES------------------\r\n");  

        hal_reboot();
    }
    else
    {
        if(bin_size > 0)
        {
            ota_rate = (flash_offset * 100) / bin_size;
            if(ota_rate % 10 == 0)
            {
                if(ota_rate != last_ota_rate)
                {
                    IOT_InfoLog("ota_rate = %ld \r\n", ota_rate);
                    IOT_InfoLog("flash_offset = %ld \r\n", flash_offset);
                    IOT_InfoLog("bin_size = %ld \r\n", bin_size);

                    OtaProInfo = IotGetOtaProInfo();
                    AilinkPublicReportProgress(0, OTA_PROGRESS_DOWNLOADING, OtaProInfo->OtaVer,OtaProInfo->PubId,ota_rate);
                    last_ota_rate = ota_rate;
                }

            }

        }
        
    }

}



/**
 * @brief   MCU ota固件数据的输出回调函数
 * 
 * @param[in]   offset          ota固件数据偏移大小
 * @param[in]   data            ota固件数据
 * @param[in]   length          ota固件数据长度
 * 
 * @author  Ai-Thinker (zhuolm@tech-now.com)
 * @date    2022-06-16
 */
void AilinkMcuOtaDownloadCallback(uint32_t offset, uint8_t *data, uint32_t length)
{

}


