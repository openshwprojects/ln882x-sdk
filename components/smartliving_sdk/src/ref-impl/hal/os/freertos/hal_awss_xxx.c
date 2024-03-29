#include <stdio.h>
#include "iot_import.h"
#include "ln_utils.h"
#include "wifi/wifi.h"
#include "netif/ethernetif.h"
#include "hal/hal_sleep.h"
#include "utils/system_parameter.h"
#include "utils/art_string.h"

typedef struct {
    uint8_t started; // 1 -- started; 0 -- not started.
} wifi_mgnt_t;

static wifi_mgnt_t sg_wifi_mgnt = { 0 };


static awss_recv_80211_frame_cb_t sg_awss_recv_80211_cb = NULL;

/**
 * @brief 中间层，其目的是为了调用 awss_recv_80211_frame_cb_t 类型的回调函数
 *
 * @param buffer
 * @param len
 * @param rssi_dbm
 * @param buffer_type
 */
static void wifi_mgmt_frame_monitor (uint8_t *buffer, int len, signed char rssi_dbm, int buffer_type)
{
    enum AWSS_LINK_TYPE link_type = AWSS_LINK_TYPE_NONE;
    int with_fcs = 0;

    if (NULL != sg_awss_recv_80211_cb) {
        sg_awss_recv_80211_cb((char *)buffer, len, link_type, with_fcs, rssi_dbm);
    }
}


static void wifi_init_sta(void)
{
    uint8_t macaddr[6] = {0}, macaddr_default[6] = {0};
    wifi_config_t wifi_config = {0};

    wifi_init_type_t init_param = {
        .wifi_mode = WIFI_MODE_STATION,
        .sta_ps_mode = WIFI_NO_POWERSAVE,
        .dhcp_mode = WLAN_DHCP_CLIENT,
        .scanned_ap_list_size = SCANNED_AP_LIST_SIZE,
    };

    //Set sleep mode
    hal_sleep_set_mode(ACTIVE);

    //Set wifi mode
    wifi_set_mode(init_param.wifi_mode);

    //Check mac address
    system_parameter_get_wifi_macaddr_default(STATION_IF, macaddr_default);
    wifi_get_macaddr(STATION_IF, macaddr);
    if(ln_is_valid_mac((const char *)macaddr) && memcmp(macaddr, macaddr_default, 6) != 0){
        //If there is a valid MAC in flash, use it
        wifi_set_macaddr_current(STATION_IF, macaddr);
    }else{
        //generate random macaddr
        ln_generate_random_mac(macaddr);
        wifi_set_macaddr(STATION_IF, macaddr);
    }

    //Check config
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    wifi_set_config(STATION_IF, &wifi_config);

    //Startup WiFi.
    if(!wifi_start(&init_param, true)){//WIFI_MAX_POWERSAVE
        LOG(LOG_LVL_ERROR, "[%s, %d]wifi_start() fail.\r\n", __func__, __LINE__);
    }
}

/**
 * @brief   设置Wi-Fi网卡工作在监听(Monitor)模式, 并在收到802.11帧的时候调用被传入的回调函数
 *
 * @param[in] cb @n A function pointer, called back when wifi receive a frame.
 */
void HAL_Awss_Open_Monitor(_IN_ awss_recv_80211_frame_cb_t cb)
{
    if (NULL == cb) {
        return;
    }

    wifi_init_sta();
    sg_awss_recv_80211_cb = cb;
    HAL_Wifi_Enable_Mgmt_Frame_Filter(0, NULL, wifi_mgmt_frame_monitor);
    HAL_Awss_Switch_Channel(1, 0, NULL);
}

/**
 * @brief   设置Wi-Fi网卡离开监听(Monitor)模式, 并开始以站点(Station)模式工作
 */
void HAL_Awss_Close_Monitor(void)
{
    HAL_Wifi_Enable_Mgmt_Frame_Filter(0, NULL, NULL);
    sg_awss_recv_80211_cb = NULL;

    // TODO: 以站点模式工作
}

/**
 * @brief   要求Wi-Fi网卡连接指定热点(Access Point)的函数
 *
 * @param[in] connection_timeout_ms @n AP connection timeout in ms or HAL_WAIT_INFINITE
 * @param[in] ssid @n AP ssid
 * @param[in] passwd @n AP passwd
 * @param[in] auth @n optional(AWSS_AUTH_TYPE_INVALID), AP auth info
 * @param[in] encry @n optional(AWSS_ENC_TYPE_INVALID), AP encry info
 * @param[in] bssid @n optional(NULL or zero mac address), AP bssid info
 * @param[in] channel @n optional, AP channel info
 * @return
   @verbatim
     = 0: connect AP & DHCP success
     = -1: connect AP or DHCP fail/timeout
   @endverbatim
 * @see None.
 * @note
 *      If the STA connects the old AP, HAL should disconnect from the old AP firstly.
 *      If bssid specifies the dest AP, HAL should use bssid to connect dest AP.
 */
int HAL_Awss_Connect_Ap(
    _IN_ uint32_t connection_timeout_ms,
    _IN_ char ssid[HAL_MAX_SSID_LEN],
    _IN_ char passwd[HAL_MAX_PASSWD_LEN],
    _IN_OPT_ enum AWSS_AUTH_TYPE auth,
    _IN_OPT_ enum AWSS_ENC_TYPE encry,
    _IN_OPT_ uint8_t bssid[ETH_ALEN],
    _IN_OPT_ uint8_t channel)
{
    LN_UNUSED_ARG(auth);
    LN_UNUSED_ARG(encry);

    wifi_config_t target_config = { 0 };

    if ( (!ssid) || (!passwd) ) {
        return FAIL_RETURN;
    }

    if ( 1== sg_wifi_mgnt.started ) {

    }

    sg_wifi_mgnt.started = 0;

    memset(&target_config, 0, sizeof(wifi_config_t));
    memcpy(&target_config.sta.ssid, ssid, strlen(ssid));
    memcpy(&target_config.sta.password, passwd, strlen(passwd));

    wifi_station_connect(&target_config);
    sg_wifi_mgnt.started = 1;

    return SUCCESS_RETURN;
}

/**
 * @brief   获取在每个信道(`channel`)上扫描的时间长度, 单位是毫秒
 *
 * @return  时间长度, 单位是毫秒
 * @note    推荐时长是200毫秒到400毫秒
 */
int HAL_Awss_Get_Channelscan_Interval_Ms(void)
{
    return 200;
}

/**
 * @brief   获取配网服务(`AWSS`)的超时时间长度, 单位是毫秒
 *
 * @return  超时时长, 单位是毫秒
 * @note    推荐时长是60,0000毫秒
 */
int HAL_Awss_Get_Timeout_Interval_Ms(void)
{
    return 180000;
}

/**
 * @brief   获取`smartconfig`服务的安全等级
 *
 * @param None.
 * @return The security level:
   @verbatim
    0: open (no encrypt)
    1: aes256cfb with default aes-key and aes-iv
    2: aes128cfb with default aes-key and aes-iv
    3: aes128cfb with aes-key per product and aes-iv = 0
    4: aes128cfb with aes-key per device and aes-iv = 0
    5: aes128cfb with aes-key per manufacture and aes-iv = 0
    others: invalid
   @endverbatim
 * @see None.
 */
int HAL_Awss_Get_Encrypt_Type(void)
{
    return 3;
}

/**
 * @brief    Get Security level for wifi configuration with connection.
 *           Used for AP solution of router and App.
 *
 * @param None.
 * @return The security level:
   @verbatim
    3: aes128cfb with aes-key per product and aes-iv = random
    4: aes128cfb with aes-key per device and aes-iv = random
    5: aes128cfb with aes-key per manufacture and aes-iv = random
    others: invalid
   @endverbatim
 * @see None.
 */
int HAL_Awss_Get_Conn_Encrypt_Type(void)
{
    char invalid_ds[DEVICE_SECRET_LEN + 1] = {0};
    char ds[DEVICE_SECRET_LEN + 1] = {0};

    HAL_GetDeviceSecret(ds);

    if (memcmp(invalid_ds, ds, sizeof(ds)) == 0)
        return 3;

    memset(invalid_ds, 0xff, sizeof(invalid_ds));
    if (memcmp(invalid_ds, ds, sizeof(ds)) == 0)
        return 3;

    return 4;
}

/**
  * @brief   开启设备热点（SoftAP模式）
  *
  * @param[in] ssid @n 热点的ssid字符；
  * @param[in] passwd @n 热点的passwd字符；
  * @param[in] beacon_interval @n 热点的Beacon广播周期（广播间隔）；
  * @param[in] hide @n 是否是隐藏热点，hide:0, 非隐藏, 其它值：隐藏；
  * @return，
 @verbatim
    = 0: success
    = -1: unsupported
    = -2: failure with system error
    = -3: failure with no memory
    = -4: failure with invalid parameters
 @endverbatim
  * @Note:
  *       1）ssid和passwd都是以'\0'结尾的字符串，如果passwd字符串的
  *          长度为0，表示该热点采用Open模式（不加密）；
  *       2）beacon_interval表示热点的Beacon广播间隔（或周期），单
  *          位为毫秒，一般会采用默认100ms；
  *       3）hide表示创建的热点是否是隐藏热点，hide=0表示非隐藏热
  *         点，其他值表示隐藏热点；
  */
int HAL_Awss_Open_Ap(const char *ssid, const char *passwd, int beacon_interval, int hide)
{
    uint8_t mac_addr[6] = { 0 }, mac_addr_default[6] = { 0 };
    wifi_config_t ap_config = { 0 };
    wifi_init_type_t init_param = {
        .wifi_mode = WIFI_MODE_AP,
        .sta_ps_mode = WIFI_NO_POWERSAVE,
        .dhcp_mode = WLAN_DHCP_SERVER
    };

    if (NULL == ssid) {
        return -4;
    }

    // set wifi mode
    wifi_set_mode(init_param.wifi_mode);

    // check MAC address
    system_parameter_get_wifi_macaddr_default(SOFT_AP_IF, mac_addr_default);
    wifi_get_macaddr(SOFT_AP_IF, mac_addr);
    if (ln_is_valid_mac((const char *)mac_addr) && (memcmp(mac_addr, mac_addr_default, 6) != 0)) {
        wifi_set_macaddr_current(SOFT_AP_IF, mac_addr);
    } else {
        ln_generate_random_mac(mac_addr);
        wifi_set_macaddr(SOFT_AP_IF, mac_addr);
    }

    // set config
    memcpy(&ap_config.ap.ssid, ssid, strlen(ssid));
    ap_config.ap.ssid_len = strlen(ssid);
    if ( (NULL != passwd) && (strlen(passwd) > 0) ) {
        memcpy(&ap_config.ap.password, passwd, strlen(passwd));
        ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ap_config.ap.ssid_hidden = hide;
    ap_config.ap.beacon_interval = beacon_interval;
    ap_config.ap.channel = 6;
    ap_config.ap.max_connection = 4;

    wifi_set_config(SOFT_AP_IF, &ap_config);

    if (0 != wifi_start(&init_param, true)) {
        return -2;
    }
    sg_wifi_mgnt.started = 1;

    return 0;
}

/**
  * @brief   关闭当前设备热点，并把设备由SoftAP模式切换到Station模式
  *
  * @return，
 @verbatim
    = 0: success
    = -1: unsupported
    = -2: failure
 @endverbatim
  * @Note:
  *       1）如果当前设备已经开启热点，关闭当前热点，如果当前设备正
  *          在开热点，取消开热点的操作；
  *       2）如果当前设备不是以Station模式（包括Station+SoftAP模式和
  *          SoftAP模式）工作，设备必须切换到Station模式；
  *       3）Wi-Fi状态机需要切换到初始化状态，因为接下来很可能进行
  *          连接某一个路由器操作；
  */
int HAL_Awss_Close_Ap(void)
{
    if (sg_wifi_mgnt.started) {
        wifi_stop();
        sg_wifi_mgnt.started = 0;
    }

    return 0;
}

/**
 * @brief   设置Wi-Fi网卡切换到指定的信道(channel)上
 *
 * @param[in] primary_channel @n Primary channel.
 * @param[in] secondary_channel @n Auxiliary channel if 40Mhz channel is supported, currently
 *              this param is always 0.
 * @param[in] bssid @n A pointer to wifi BSSID on which awss lock the channel, most HAL
 *              may ignore it.
 */
void HAL_Awss_Switch_Channel(char primary_channel, char secondary_channel, uint8_t bssid[ETH_ALEN])
{
    wifi_set_channel( (uint8_t)primary_channel );
}