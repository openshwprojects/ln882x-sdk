/*
 * Tencent is pleased to support the open source community by making IoT Hub
 available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file
 except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software
 distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 KIND,
 * either express or implied. See the License for the specific language
 governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lite-utils.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#include "flash_partition_table.h"
#include "ln_nvds.h"
#include "ln_kv_api.h"
#include "hal/flash.h"

#include "ota_port.h"
#include "ota_image.h"
#include "ota_types.h"

#define FW_RUNNING_VERSION "1.0"
//#define FW_RUNNING_VERSION "1.1"
//#define FW_RUNNING_VERSION "1.2"


#define KEY_REMOTE_VERSION      "version"
#define KEY_DOWNLOADED_SIZE     "downloaded_size"
#define KEY_FW_FILE_SIZE        "fw_file_size"

#define FW_VERSION_MAX_LEN      (32)
#define FW_FILE_PATH_MAX_LEN    (128)
#define OTA_BUF_LEN             (1024 * 4)
#define FW_INFO_FILE_DATA_LEN   (128)

static uint32_t SG_OTA_FLASH_BASE_OFFSET = OTA_SPACE_OFFSET;

typedef struct OTAContextData {
    void *ota_handle;
    void *mqtt_client;
    char  fw_file_path[FW_FILE_PATH_MAX_LEN];
    char  fw_info_file_path[FW_FILE_PATH_MAX_LEN];

    // remote_version means version for the FW in the cloud and to be downloaded
    char     remote_version[FW_VERSION_MAX_LEN];
    uint32_t fw_file_size;

    // for resuming download
    /* local_version means downloading but not running */
    char local_version[FW_VERSION_MAX_LEN];
    int  downloaded_size;

    // to make sure report is acked
    bool report_pub_ack;
    int  report_packet_id;

} OTAContextData;

static DeviceInfo sg_devInfo;

static void _event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg)
{
    uintptr_t       packet_id = (uintptr_t)msg->msg;
    OTAContextData *ota_ctx   = (OTAContextData *)handle_context;

    switch (msg->event_type) {
        case MQTT_EVENT_UNDEF:
            Log_i("undefined event occur.");
            break;

        case MQTT_EVENT_DISCONNECT:
            Log_i("MQTT disconnect.");
            break;

        case MQTT_EVENT_RECONNECT:
            Log_i("MQTT reconnect.");
            break;

        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_SUCCESS:
            Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
            if (ota_ctx->report_packet_id == packet_id)
                ota_ctx->report_pub_ack = true;
            break;

        case MQTT_EVENT_PUBLISH_TIMEOUT:
            Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_NACK:
            Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;
        default:
            Log_i("Should NOT arrive here.");
            break;
    }
}

static int _setup_connect_init_params(MQTTInitParams *initParams, void *ota_ctx, DeviceInfo *device_info)
{
    initParams->region      = device_info->region;
    initParams->product_id  = device_info->product_id;
    initParams->device_name = device_info->device_name;

#ifdef AUTH_MODE_CERT
    char  certs_dir[16] = "certs";
    char  current_path[128];
    char *cwd = getcwd(current_path, sizeof(current_path));

    if (cwd == NULL) {
        Log_e("getcwd return NULL");
        return QCLOUD_ERR_FAILURE;
    }

#ifdef WIN32
    HAL_Snprintf(initParams->cert_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 device_info->dev_cert_file_name);
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 device_info->dev_key_file_name);
#else
    HAL_Snprintf(initParams->cert_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 device_info->dev_cert_file_name);
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 device_info->dev_key_file_name);
#endif

#else
    initParams->device_secret = device_info->device_secret;
#endif

    initParams->command_timeout        = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    initParams->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

    initParams->auto_connect_enable  = 1;
    initParams->event_handle.h_fp    = _event_handler;
    initParams->event_handle.context = ota_ctx;

    return QCLOUD_RET_SUCCESS;
}

static void _wait_for_pub_ack(OTAContextData *ota_ctx, int packet_id)
{
    int wait_cnt              = 10;
    ota_ctx->report_pub_ack   = false;
    ota_ctx->report_packet_id = packet_id;

    while (!ota_ctx->report_pub_ack) {
        HAL_SleepMs(500);
        IOT_MQTT_Yield(ota_ctx->mqtt_client, 500);
        if (wait_cnt-- == 0) {
            Log_e("wait report pub ack timeout!");
            break;
        }
    }
    ota_ctx->report_pub_ack = false;
    return;
}

/**
 * @brief 格式化固件保存区域
 */
static void _reformat_fw_download_area(void)
{
    const uint32_t fw_start_addr = OTA_SPACE_OFFSET;
    const uint32_t fw_area_size = OTA_SPACE_SIZE;

    FLASH_Erase(fw_start_addr, fw_area_size);
}

/**
 * @brief check ota image header, body.
 * @return return LN_TRUE on success, LN_FALSE on failure.
 */
static int ota_verify_download(void)
{
    image_hdr_t ota_header;

    if ( OTA_ERR_NONE != image_header_fast_read(OTA_SPACE_OFFSET, &ota_header) ) {
        LOG(LOG_LVL_ERROR, "failed to read ota header.\r\n");
        return LN_FALSE;
    } else {
        LOG(LOG_LVL_INFO, "OK: image_header_fast_read\r\n");
    }

    if ( OTA_ERR_NONE != image_header_verify(&ota_header) ) {
        LOG(LOG_LVL_ERROR, "failed to verify ota header.\r\n");
        return LN_FALSE;
    } else {
        LOG(LOG_LVL_INFO, "OK: image_header_verify\r\n");
    }

    if ( OTA_ERR_NONE != image_body_verify(OTA_SPACE_OFFSET, &ota_header)) {
        LOG(LOG_LVL_ERROR, "failed to verify ota body.\r\n");
        return LN_FALSE;
    }

    LOG(LOG_LVL_INFO, "Succeed to verify OTA image content.\r\n");
    return LN_TRUE;
}

static int update_ota_state(void)
{
    upg_state_t state = UPG_STATE_DOWNLOAD_OK;

    ln_nvds_set_ota_upg_state((uint32_t)state);
    return LN_TRUE;
}

/**********************************************************************************
 * OTA file operations START
 * these are platform-dependant functions
 * POSIX FILE is used in this sample code
 **********************************************************************************/
// calculate left MD5 for resuming download from break point
static int _cal_exist_fw_md5(OTAContextData *ota_ctx)
{
    char   buff[OTA_BUF_LEN];
    size_t rlen, total_read = 0;
    int    ret = QCLOUD_RET_SUCCESS;

    ret = IOT_OTA_ResetClientMD5(ota_ctx->ota_handle);
    if (ret) {
        Log_e("reset MD5 failed: %d", ret);
        return QCLOUD_ERR_FAILURE;
    }


    size_t size = ota_ctx->downloaded_size;
    // 读取所有内容，计算 MD5
    while (size > 0) {
        rlen = (size > OTA_BUF_LEN) ? OTA_BUF_LEN : size;
        memset(buff, 0, OTA_BUF_LEN);
        FLASH_Read(SG_OTA_FLASH_BASE_OFFSET + total_read, rlen, (uint8_t *)buff);
        IOT_OTA_UpdateClientMd5(ota_ctx->ota_handle, buff, rlen);
        total_read += rlen;
        size -= rlen;
    }

    Log_i("total read: %d", total_read);
    return ret;
}


/**
 * @brief update local firmware info for resuming download from break point.
 * 把 remote_version 和 downloaded_size 持久化
 * @param ota_ctx
 * @return int
 */
static int _update_local_fw_info(OTAContextData *ota_ctx)
{
    int   ret = QCLOUD_RET_SUCCESS;

    char buf[10] = { 0 };
    int len = 0;
    kv_err_t err;

    len = snprintf(buf, 10, "%d", ota_ctx->downloaded_size);
    buf[len] = '\0';

    // Log_i("remote version: [%s], downloaded_size: %d", ota_ctx->remote_version, ota_ctx->downloaded_size);

    err = ln_kv_set(KEY_REMOTE_VERSION, ota_ctx->remote_version, strlen(ota_ctx->remote_version));
    if (err != KV_ERR_NONE) {
        Log_e("Error: set KEY_REMOTE_VERSION!!!");
        ret = QCLOUD_ERR_FAILURE;
    }

    err = ln_kv_set(KEY_DOWNLOADED_SIZE, buf, len);
    if (err != KV_ERR_NONE) {
        Log_e("Error: set KEY_DOWNLOADED_SIZE!!!");
        ret = QCLOUD_ERR_FAILURE;
    }

    return ret;
}

/**
 * @brief 从本地固件信息文件中获取固件版本号，固件大小
 * _get_local_fw_info(ota_ctx->fw_info_file_path, ota_ctx->local_version)
 * @param [in] file_name
 * @param [out] local_version
 * @return int 固件大小
 */
static int _get_local_fw_info(char *file_name, char *local_version)
{
    int local_size = 0;
    uint8_t buff[FW_VERSION_MAX_LEN] = { 0 };
    size_t len = 0;

    if (KV_ERR_NONE == ln_kv_get(KEY_REMOTE_VERSION, buff, FW_VERSION_MAX_LEN, &len)) {
        memcpy(local_version, buff, strlen((const char *)buff));
    } else {
        Log_e("Error: get KEY_REMOTE_VERSION!!!");
    }

    memset(buff, 0, FW_VERSION_MAX_LEN);
    len = 0;

    if (KV_ERR_NONE == ln_kv_get(KEY_DOWNLOADED_SIZE, buff, FW_VERSION_MAX_LEN, &len)) {
        local_size = atoi((const char *)buff);
    } else {
        Log_e("Error: get KEY_DOWNLOADED_SIZE!!!");
    }

    return local_size;
}

/**
 *
 * @brief get local firmware offset for resuming download from break point.
 *
 * @param ota_ctx
 * @return int
 */
static int _update_fw_downloaded_size(OTAContextData *ota_ctx)
{
    int local_size = 0;

    local_size = _get_local_fw_info(ota_ctx->fw_info_file_path, ota_ctx->local_version);
    if (local_size == 0) {
        ota_ctx->downloaded_size = 0;
        return 0;
    }

    if ((0 != strcmp(ota_ctx->local_version, ota_ctx->remote_version)) ||
        (ota_ctx->downloaded_size > ota_ctx->fw_file_size)) {
        ota_ctx->downloaded_size = 0;
        return 0;
    }

    ota_ctx->downloaded_size = local_size;
    Log_i("calc MD5 for resuming download from offset: %d", ota_ctx->downloaded_size);
    int ret = _cal_exist_fw_md5(ota_ctx);
    if (ret) {
        Log_e("regen OTA MD5 error: %d", ret);
        remove(ota_ctx->fw_info_file_path);
        ota_ctx->downloaded_size = 0;
        return 0;
    }
    Log_i("local MD5 update done!");
    return local_size;
}

/**
 * @brief 删除固件下载进度信息
 *
 * @param file_name
 * @return int
 */
static int _delete_fw_info_file(char *file_name)
{
    int ret = QCLOUD_RET_SUCCESS;

    if ( KV_ERR_NONE != ln_kv_del(KEY_REMOTE_VERSION) ) {
        Log_e("Error: del KEY_REMOTE_VERSION!!!");
        ret = QCLOUD_ERR_FAILURE;
    }

    if ( KV_ERR_NONE != ln_kv_del(KEY_DOWNLOADED_SIZE) ) {
        Log_e("Error: del KEY_DOWNLOADED_SIZE!!!");
        ret = QCLOUD_ERR_FAILURE;
    }

    return ret;
}

/**
 * @brief 保存下载的一段固件
 *
 * @param file_name 固件名字
 * @param offset 保存的起始偏移地址
 * @param buf 这一段固件的缓冲区起始地址
 * @param len 这一段固件的大小
 * @return int
 * @retval 0 on success;
 * @retval other on failure.
 */
static int _save_fw_data_to_file(char *file_name, uint32_t offset, char *buf, int len)
{
    if (0 == offset) {
        _reformat_fw_download_area();
    }

    FLASH_Program(SG_OTA_FLASH_BASE_OFFSET + offset, len, (uint8_t *)buf);

    return 0;
}

static char *_get_local_fw_running_version()
{
    // asuming the version is inside the code and binary
    // you can also get from a meta file
    Log_i("FW running version: %s", FW_RUNNING_VERSION);
    return FW_RUNNING_VERSION;
}


/**********************************************************************************
 * OTA file operations END
 **********************************************************************************/

// main OTA cycle
bool process_ota(OTAContextData *ota_ctx)
{
    bool  download_finished     = false;
    bool  upgrade_fetch_success = true;
    bool  ota_verify_result = true;
    char  buf_ota[OTA_BUF_LEN];
    int   rc;
    void *h_ota = ota_ctx->ota_handle;

    /* Must report version first */
    if (0 > IOT_OTA_ReportVersion(h_ota, _get_local_fw_running_version())) {
        Log_e("report OTA version failed");
        return false;
    }

    do {
        IOT_MQTT_Yield(ota_ctx->mqtt_client, 200);

        Log_i("wait for ota upgrade command...");

        // recv the upgrade cmd
        if (IOT_OTA_IsFetching(h_ota)) {
            IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &ota_ctx->fw_file_size, 4);
            IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, ota_ctx->remote_version, FW_VERSION_MAX_LEN);

            HAL_Snprintf(ota_ctx->fw_file_path, FW_FILE_PATH_MAX_LEN, "./FW_%s.bin", ota_ctx->remote_version);
            HAL_Snprintf(ota_ctx->fw_info_file_path, FW_FILE_PATH_MAX_LEN, "./FW_%s.json", ota_ctx->remote_version);

            /* check if pre-downloading finished or not */
            /* if local FW downloaded size (ota_ctx->downloaded_size) is not zero, it
             * will do resuming download */
            _update_fw_downloaded_size(ota_ctx);

            /*set offset and start http connect*/
            rc = IOT_OTA_StartDownload(h_ota, ota_ctx->downloaded_size, ota_ctx->fw_file_size);
            if (QCLOUD_RET_SUCCESS != rc) {
                Log_e("OTA download start err,rc:%d", rc);
                upgrade_fetch_success = false;
                break;
            }

            // download and save the fw
            do {
                int len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);
                if (len > 0) {
                    rc = _save_fw_data_to_file(ota_ctx->fw_file_path, ota_ctx->downloaded_size, buf_ota, len);
                    if (rc) {
                        Log_e("write data to file failed");
                        upgrade_fetch_success = false;
                        break;
                    }
                } else if (len < 0) {
                    Log_e("download fail rc=%d", len);
                    upgrade_fetch_success = false;
                    break;
                }

                /* get OTA information and update local info */
                // ota_ctx->downloaded_size 此时更新为总共的下载大小
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &ota_ctx->downloaded_size, 4);
                rc = _update_local_fw_info(ota_ctx);
                if (QCLOUD_RET_SUCCESS != rc) {
                    Log_e("update local fw info err,rc:%d", rc);
                }

                // quit ota process as something wrong with mqtt
                rc = IOT_MQTT_Yield(ota_ctx->mqtt_client, 100);
                if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
                    Log_e("MQTT error: %d", rc);
                    return false;
                }

            } while (!IOT_OTA_IsFetchFinish(h_ota));

            /* Must check MD5 match or not */
            if (upgrade_fetch_success) {
                // download is finished, delete the fw info file
                _delete_fw_info_file(ota_ctx->fw_info_file_path);

                // NOTE: 校验MD5并上报给服务器
                uint32_t firmware_valid;
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
                if (0 == firmware_valid) {
                    Log_e("The firmware is invalid");
                    upgrade_fetch_success = false;
                } else {
                    Log_i("The firmware is valid");
                    upgrade_fetch_success = true;
                }
            }

            download_finished = true;
        }

        if (!download_finished)
            HAL_SleepMs(1000);

    } while (!download_finished);

    // do some post-download stuff for your need
    if (upgrade_fetch_success) {
        if ( LN_TRUE == ota_verify_download() ) {
            Log_i("OK: ota_verify_download()");
        } else {
            ota_verify_result = false;
        }
    } else {
        ota_verify_result = false;
        Log_e("Error: ota failed!!!");
    }

    // report result
    int packet_id;
    if (upgrade_fetch_success && ota_verify_result) {
        packet_id = IOT_OTA_ReportUpgradeSuccess(h_ota, NULL);
    } else {
        packet_id = IOT_OTA_ReportUpgradeFail(h_ota, NULL);
    }
    _wait_for_pub_ack(ota_ctx, packet_id);

    return ota_verify_result;
}

int ota_mqtt_example(void)
{
    int             rc;
    OTAContextData *ota_ctx     = NULL;
    void *          mqtt_client = NULL;
    void *          h_ota       = NULL;

    IOT_Log_Set_Level(eLOG_DEBUG);
    ota_ctx = (OTAContextData *)HAL_Malloc(sizeof(OTAContextData));
    if (ota_ctx == NULL) {
        Log_e("malloc failed");
        goto exit;
    }
    memset(ota_ctx, 0, sizeof(OTAContextData));

    rc = HAL_GetDevInfo(&sg_devInfo);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("get device info failed: %d", rc);
        goto exit;
    }

    // setup MQTT init params
    MQTTInitParams init_params = DEFAULT_MQTTINIT_PARAMS;
    rc                         = _setup_connect_init_params(&init_params, ota_ctx, &sg_devInfo);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("init params err,rc=%d", rc);
        return rc;
    }

    // create MQTT mqtt_client and connect to server
    mqtt_client = IOT_MQTT_Construct(&init_params);
    if (mqtt_client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
        return QCLOUD_ERR_FAILURE;
    }

    // init OTA handle
    h_ota = IOT_OTA_Init(sg_devInfo.product_id, sg_devInfo.device_name, mqtt_client);
    if (NULL == h_ota) {
        Log_e("initialize OTA failed");
        goto exit;
    }

    ota_ctx->ota_handle  = h_ota;
    ota_ctx->mqtt_client = mqtt_client;

    bool ota_success;
    do {
        // mqtt should be ready first
        rc = IOT_MQTT_Yield(mqtt_client, 500);
        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) {
            HAL_SleepMs(1000);
            continue;
        } else if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
            Log_e("exit with error: %d", rc);
            break;
        }

        // OTA process
        ota_success = process_ota(ota_ctx);
        if (!ota_success) {
            Log_e("OTA failed! Do it again");
            HAL_SleepMs(2000);
        }
    } while (!ota_success);

exit:

    if (NULL != ota_ctx)
        HAL_Free(ota_ctx);

    if (NULL != h_ota)
        IOT_OTA_Destroy(h_ota);

    IOT_MQTT_Destroy(&mqtt_client);

    if (ota_success) {
        update_ota_state();
        ota_get_port_ctx()->chip_reboot();
    }
    return 0;
}
