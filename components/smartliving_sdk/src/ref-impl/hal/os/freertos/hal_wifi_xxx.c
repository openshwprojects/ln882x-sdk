#include <string.h>
#include "iot_import.h"

// LN headers
#include "ln_utils.h"
#include "wifi/wifi.h"
#include "wifi_manager/wifi_manager.h"
#include "netif/ethernetif.h"

// `HAL_Wifi_Enable_Mgmt_Frame_Filter()` monitor adapter
static  awss_wifi_mgmt_frame_cb_t   sg_monitor_cb = NULL;
static void ln_wifi_promisc_callback(void *buf, uint16_t len, uint8_t pkt_type);

// `HAL_Wifi_Scan()` 是一个阻塞型的API，必须把扫描到的每个AP信息都处理一遍然后才能退出当前API。
static             OS_Semaphore_t   sg_sta_scan_deal_sem = { 0 };
static awss_wifi_scan_result_cb_t   sg_scan_result_cb = NULL;

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////  LN WiFi adapter  ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int auth_mode_to_ln(int auth_type);
int auth_mode_from_ln(int auth_mode);


/**
 * @brief Authenticate mode conversion from LN `wifi_auth_mode_enum_t` to
 * IOT `enum AWSS_AUTH_TYPE`.
 *
 * @param auth_mode
 * @return int
 */
int auth_mode_from_ln(int auth_mode)
{
    enum AWSS_AUTH_TYPE ret = AWSS_AUTH_TYPE_INVALID;

    switch ( (wifi_auth_mode_enum_t) auth_mode)
    {
        case WIFI_AUTH_OPEN: {
            ret = AWSS_AUTH_TYPE_OPEN;
            break;
        }
        case WIFI_AUTH_WEP: {
            ret = AWSS_AUTH_TYPE_SHARED;
            break;
        }
        case WIFI_AUTH_WPA_PSK: {
            ret = AWSS_AUTH_TYPE_WPAPSK;
            break;
        }
        case WIFI_AUTH_WPA2_PSK: {
            ret = AWSS_AUTH_TYPE_WPA2PSK;
            break;
        }
        case WIFI_AUTH_WPA_WPA2_PSK: {
            ret = AWSS_AUTH_TYPE_WPAPSKWPA2PSK;
            break;
        }
        case WIFI_AUTH_WPA2_ENTERPRISE: {
            break;
        }
        case WIFI_AUTH_MAX:
        default:
            break;
    }

    return (int)ret;
}


int auth_mode_to_ln(int auth_type)
{
    wifi_auth_mode_enum_t ret = WIFI_AUTH_MAX;
    switch ( (enum AWSS_AUTH_TYPE)auth_type ) {
        case AWSS_AUTH_TYPE_OPEN: {
            ret = WIFI_AUTH_OPEN;
            break;
        }
        case AWSS_AUTH_TYPE_SHARED: {
            ret = WIFI_AUTH_WEP;
            break;
        }
        case AWSS_AUTH_TYPE_WPAPSK: {
            ret = WIFI_AUTH_WPA_PSK;
            break;
        }
        case AWSS_AUTH_TYPE_WPA8021X: {

            break;
        }
        case AWSS_AUTH_TYPE_WPA2PSK: {
            ret = WIFI_AUTH_WPA2_PSK;
            break;
        }
        case AWSS_AUTH_TYPE_WPA28021X: {

            break;
        }
        case AWSS_AUTH_TYPE_WPAPSKWPA2PSK: {
            ret = WIFI_AUTH_WPA_WPA2_PSK;
            break;
        }
        default:
            break;
    }

    return (int)ret;
}


static void ln_wifi_promisc_callback(void *buf, uint16_t len, uint8_t pkt_type)
{
    int buffer_type = 0; // 0 -- 80211 frame; 1 -- only contain IE info.
    char rssi = 0;

    if ( (NULL == buf) || (0 == len)) {
        return;
    }

    if (NULL == sg_monitor_cb) {
        return;
    }

    switch (pkt_type) {
        case WIFI_PKT_MGMT: {

            break;
        }
        case WIFI_PKT_CTRL: {

            break;
        }
        case WIFI_PKT_DATA: {
            rssi = *(char*)((char *)buf + 0);
            len -= 1;
            sg_monitor_cb( (uint8_t *) ((uint8_t *)buf + 1), (int) len, (signed char) rssi, buffer_type);
            break;
        }
        default: {

            break;
        }
    }
}

/**
 * @brief   使能或禁用对管理帧的过滤
 *
 * @param[in] filter_mask @n see mask macro in enum HAL_Awss_frame_type,
 *                      currently only FRAME_PROBE_REQ_MASK & FRAME_BEACON_MASK is used
 * @param[in] vendor_oui @n oui can be used for precise frame match, optional
 * @param[in] callback @n see awss_wifi_mgmt_frame_cb_t, passing 80211
 *                      frame or ie to callback. when callback is NULL
 *                      disable sniffer feature, otherwise enable it.
 * @return
   @verbatim
   =  0, success
   = -1, fail
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API to filter specific mgnt frame in wifi station mode
 */
int HAL_Wifi_Enable_Mgmt_Frame_Filter(
    _IN_ uint32_t filter_mask,
    _IN_OPT_ uint8_t vendor_oui[3],
    _IN_ awss_wifi_mgmt_frame_cb_t callback)
{
    LN_UNUSED_ARG(filter_mask);
    LN_UNUSED_ARG(vendor_oui);

    sg_monitor_cb = callback;
    if (callback != NULL) { // enable
        wifi_set_promiscuous_filter(WIFI_PROMIS_FILTER_MASK_DATA);
        wifi_set_promiscuous_rx_cb(ln_wifi_promisc_callback);
        wifi_set_promiscuous(true);
    } else { // disable
        wifi_set_promiscuous(false);
        wifi_set_promiscuous_rx_cb(NULL);
    }
    return 0;
}

/**
 * @brief   获取所连接的热点(Access Point)的信息
 *
 * @param[out] ssid: array to store ap ssid. It will be null if ssid is not required.
 * @param[out] passwd: array to store ap password. It will be null if ap password is not required.
 * @param[out] bssid: array to store ap bssid. It will be null if bssid is not required.
 * @return
   @verbatim
     = 0: succeeded
     = -1: failed
   @endverbatim
 * @see None.
 * @note
 *     If the STA dosen't connect AP successfully, HAL should return -1 and not touch the ssid/passwd/bssid buffer.
 */
int HAL_Wifi_Get_Ap_Info(char ssid[HAL_MAX_SSID_LEN], char passwd[HAL_MAX_PASSWD_LEN], uint8_t bssid[ETH_ALEN])
{
    int ret = 0;
    wifi_mode_enum_t mode = WIFI_MODE_MAX;
    wifi_config_t config = { 0 };


    if ( (NULL == ssid) || (NULL == passwd) || (NULL == bssid) ) {
        ret = -1;
        return ret;
    }

    mode = wifi_get_mode_current();
    if (WIFI_MODE_STATION != mode) {
        ret = -1;
        return ret;
    }

    if (LINK_UP != ethernetif_get_link_state()) {
        ret = -1;
        return ret;
    }

    if (0 != wifi_get_config_current(STATION_IF, &config)) {
        ret = -1;
        return ret;
    }
    strcpy((char *)ssid, (const char *)config.ap.ssid);
    strcpy((char *)passwd, (const char *)config.ap.password);
    // TODO: bssid not touched.

    return ret;
}

/**
 * @brief   获取Wi-Fi网口的IP地址, 点分十进制格式保存在字符串数组出参, 二进制格式则作为返回值, 并以网络字节序(大端)表达
 *
 * @param   ifname : 指定Wi-Fi网络接口的名字
 * @param   ip_str : 存放点分十进制格式的IP地址字符串的数组
 * @return  二进制形式的IP地址, 以网络字节序(大端)组织
 */
uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
    LN_UNUSED_ARG(ifname);

    wifi_mode_enum_t mode = WIFI_MODE_MAX;
    wifi_interface_enum_t if_index;
    tcpip_ip_info_t ip_info = { 0 };
    char *ip_format = NULL;
    uint32_t addr = 0;

    mode = wifi_get_mode_current();

    if (WIFI_MODE_STATION == mode) {
        if_index = STATION_IF;
    } else if(WIFI_MODE_AP == mode) {
        if_index = SOFT_AP_IF;
    }

    if (LINK_UP != ethernetif_get_link_state()) {
        return addr;
    }

    ethernetif_get_ip_info(if_index, &ip_info);
    ip_format = ip4addr_ntoa(&ip_info.ip);
    if (NULL != ip_format) {
        strcpy(ip_str, ip_format);
    }
    addr = ipaddr_addr(ip_format);
    return addr;
}

/**
 * @brief   获取Wi-Fi网口的MAC地址, 格式应当是"XX:XX:XX:XX:XX:XX"
 *
 * @param   mac_str : 用于存放MAC地址字符串的缓冲区数组
 * @return  指向缓冲区数组起始位置的字符指针
 */
char *HAL_Wifi_Get_Mac(char mac_str[HAL_MAC_LEN])
{
    wifi_interface_enum_t if_index;
    wifi_mode_enum_t mode = WIFI_MODE_MAX; //wifi_get_mode_current();

    if (NULL == mac_str) {
        return (char *)NULL;
    }

    if (WIFI_MODE_STATION == mode) {
        if_index = STATION_IF;
    } else if(WIFI_MODE_AP == mode) {
        if_index = SOFT_AP_IF;
    }

    // wifi_get_macaddr_current(if_index, (uint8_t *)mac_str);
    wifi_get_macaddr(if_index, (uint8_t *)mac_str);
    return (char *)mac_str;
}

static void sta_scan_done_msg_cb(wifi_msg_t *msg)
{
    if(WIFI_MSG_ID_STA_SCAN_DONE == msg->msg_id){
        reg_wifi_msg_callbcak(WIFI_MSG_ID_STA_SCAN_DONE, NULL);

        int i = 0;
        ap_info_t *item_iterator = NULL;
        ln_list_t scan_list, *iterator;
        int ap_count = wifi_station_get_scan_list_size();
        if (ap_count <= 0) {
            return;
        }

        // prepare scan_list space
        ln_list_init(&scan_list);
        for (i = 0; i < ap_count; i++) {
            item_iterator = OS_Malloc(sizeof(ap_info_t));
            if (item_iterator) {
                ln_list_add(&scan_list, &(item_iterator->list));
            }
        }

        // get AP list
        wifi_station_get_scan_list(&scan_list, ap_count, true);

        for (i = 0, iterator = scan_list.next; iterator != &scan_list; iterator = iterator->next, i++) {
            item_iterator = ln_list_entry(iterator, ap_info_t, list);

            // callback on every AP info.
            char *ssid = (char *)item_iterator->ssid;
            char *bssid = (char *)item_iterator->bssid;
            enum AWSS_AUTH_TYPE auth = (enum AWSS_AUTH_TYPE) auth_mode_from_ln( (int)item_iterator->authmode );
            enum AWSS_ENC_TYPE encry = 0; // TODO: 用哪一个类型？
            uint8_t channel = item_iterator->channel;
            signed char rssi = item_iterator->rssi;
            int is_last_ap = (i == ap_count) ? 1 : 0;

            if (sg_scan_result_cb) {
                sg_scan_result_cb( (const char*)ssid, (const uint8_t*)bssid,
                            (enum AWSS_AUTH_TYPE)auth, (enum AWSS_ENC_TYPE)encry,
                            channel, rssi, is_last_ap);
            }

        }

        OS_SemaphoreRelease(&sg_sta_scan_deal_sem);
    }
}

/**
 * @brief   启动一次Wi-Fi的空中扫描(Scan)
 *
 * @param[in] cb @n pass ssid info(scan result) to this callback one by one
 * @return 0 for wifi scan is done, otherwise return -1
 * @see None.
 * @note
 *      This API should NOT exit before the invoking for cb is finished.
 *      This rule is something like the following :
 *      HAL_Wifi_Scan() is invoked...
 *      ...
 *      for (ap = first_ap; ap <= last_ap; ap = next_ap){
 *        cb(ap)
 *      }
 *      ...
 *      HAL_Wifi_Scan() exit...
 */
int HAL_Wifi_Scan(awss_wifi_scan_result_cb_t cb)
{
    int ret = 0;
    wifi_mode_enum_t    mode;
    wifi_scan_config_t  scan_cfg = { 0 };

    mode = wifi_get_mode();
    if (WIFI_MODE_STATION != mode) {
        ret = -1;
        return ret;
    }

    if (!OS_SemaphoreIsValid(&sg_sta_scan_deal_sem)) {
        if (OS_OK != OS_SemaphoreCreateBinary(&sg_sta_scan_deal_sem)) {
            ret = -1;
            return ret;
        }
    }

    if (LINK_UP != ethernetif_get_link_state()) {
        ret = -1;
        return ret;
    }

    sg_scan_result_cb = cb;

    scan_cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    scan_cfg.channel = 0;
    scan_cfg.show_hidden = false;
    scan_cfg.scan_time.active = 30;
    scan_cfg.scan_time.passive = 120;

    reg_wifi_msg_callbcak(WIFI_MSG_ID_STA_SCAN_DONE, sta_scan_done_msg_cb);

    wifi_station_scan(&scan_cfg);

    OS_SemaphoreWait(&sg_sta_scan_deal_sem, OS_WAIT_FOREVER);

    return ret;
}

/**
 * @brief   在当前信道(channel)上以基本数据速率(1Mbps)发送裸的802.11帧(raw 802.11 frame)
 *
 * @param[in] type @n see enum HAL_Awss_frame_type, currently only FRAME_BEACON
 *                      FRAME_PROBE_REQ is used
 * @param[in] buffer @n 80211 raw frame, include complete mac header & FCS field
 * @param[in] len @n 80211 raw frame length
 * @return
   @verbatim
   =  0, send success.
   = -1, send failure.
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API send raw frame in wifi monitor mode & station mode
 */
int HAL_Wifi_Send_80211_Raw_Frame(_IN_ enum HAL_Awss_Frame_Type type,
                                  _IN_ uint8_t *buffer, _IN_ int len)
{
    LN_UNUSED_ARG(type);

    return wifi_send_80211_mgmt_raw_frame(buffer, len);
}
