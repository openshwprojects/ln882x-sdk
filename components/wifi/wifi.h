#ifndef __WIFI_H__
#define __WIFI_H__
#include "ln_types.h"
#include "utils/ln_list.h"
#include <stdbool.h>

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#define SSID_MAX_LEN            33
#define PASSWORD_MAX_LEN        64
#define MAC_ADDRESS_LEN         6
#define BSSID_LEN               MAC_ADDRESS_LEN

/*
* This macro defines the maximum size of the scanned AP list, which can be modified by the customer, Maximum value is 50.
*/
#define SCANNED_AP_LIST_SIZE          (32)

typedef enum {
    STATION_IF = 0,         /**<  station interface */
    SOFT_AP_IF,             /**<  soft-AP interface */
    MONITOR_IF,             /**<  Monitor interface */
    WIFI_IF_NUM,
} wifi_interface_enum_t;

typedef enum {
    AP_LIST_BY_SIGNAL = 0,        /**<  Sort match AP in scan list by RSSI */
    AP_LIST_BY_SECURITY,          /**<  Sort match AP in scan list by security mode */
} ap_list_sort_method_t;


typedef enum
{
    WIFI_SCAN_TYPE_ACTIVE,   //active scan
	WIFI_SCAN_TYPE_PASSIVE,  //passive scan
}wifi_scan_type_t;

typedef enum {
	CIPHER_NONE,      /**<  cipher type is none */
	CIPHER_WEP40,     /**<  cipher type is WEP40 */
	CIPHER_WEP104,    /**<  cipher type is WEP104 */
	CIPHER_TKIP,      /**<  cipher type is TKIP */
	CIPHER_CCMP,      /**<  cipher type is CCMP */
	CIPHER_TKIP_CCMP, /**<  cipher type is TKIP and CCMP */
	CIPHER_UNKNOWN    /**<  cipher type is unknown */
} cipher_type_enum_t;

typedef enum {
    WIFI_AUTH_OPEN = 0,         /**<  authenticate mode : open */
    WIFI_AUTH_WEP,              /**<  authenticate mode : WEP */
    WIFI_AUTH_WPA_PSK,          /**<  authenticate mode : WPA_PSK */
    WIFI_AUTH_WPA2_PSK,         /**<  authenticate mode : WPA2_PSK */
    WIFI_AUTH_WPA_WPA2_PSK,     /**<  authenticate mode : WPA_WPA2_PSK */
    WIFI_AUTH_WPA2_ENTERPRISE,  /**<  authenticate mode : WPA2_ENTERPRISE */
    WIFI_AUTH_MAX
} wifi_auth_mode_enum_t;

typedef enum {
	CTRY_CODE_CN = 0,
	CTRY_CODE_US,
	CTRY_CODE_JP,
	CTRY_CODE_ISR,
	CTRY_CODE_MAX,
} wifi_country_code_enum_t;

typedef enum
{
    WIFI_MODE_STATION,     /**<  station mode */
    WIFI_MODE_AP,     /**<  SoftAP mode */
    WIFI_MODE_MONITOR,
    WIFI_MODE_AP_STATION,   /**<  Station + SoftAP mode */
    WIFI_MODE_MAX
} wifi_mode_enum_t;
typedef enum
{
    WIFI_NO_POWERSAVE = 0,
    WIFI_MIN_POWERSAVE = 1,
    WIFI_MAX_POWERSAVE = 2,
} sta_powersave_mode_enum_t;

typedef enum{
    WIFI_STA_STATUS_READY = 0,
    WIFI_STA_STATUS_SCANING,
    WIFI_STA_STATUS_CONNECTING,
    WIFI_STA_STATUS_CONNECTED,
    WIFI_STA_STATUS_DISCONNECTING,
    WIFI_STA_STATUS_DISCONNECTED,
}wifi_station_status_enum_t;

typedef enum{
    WIFI_AP_STATUS_READY = 0,
    WIFI_AP_STATUS_STACONNECTED,
    WIFI_AP_STATUS_STADISCONNECTED,
}wifi_softap_status_enum_t;

typedef enum {
    WLAN_DHCP_DISABLE = 0,
    WLAN_DHCP_CLIENT,
    WLAN_DHCP_SERVER,
    WLAN_STATIC_IP = WLAN_DHCP_DISABLE,
}dhcp_mode_enum_t;

typedef struct {
    wifi_mode_enum_t            wifi_mode;
    sta_powersave_mode_enum_t   sta_ps_mode;
    dhcp_mode_enum_t            dhcp_mode;              /* DHCP mode, @ref DHCP_Disable, @ref DHCP_Client and @ref DHCP_Server. */
    char                        local_ip_addr[16];      /* Static IP configuration, Local IP address. */
    char                        net_mask[16];           /* Static IP configuration, Netmask. */
    char                        gateway_ip_addr[16];    /* Static IP configuration, Router IP address. */
    char                        dns_server_ip_addr[16]; /* Static IP configuration, DNS server IP address. */
    int                         scanned_ap_list_size;   /* In station mode, this parameter defines the list size of APS list*/
}wifi_init_type_t;

/** @brief Aggregate of active & passive scan time per channel */
typedef struct {
    uint16_t active;  /**< active scan time per channel, units: millisecond. */
    uint16_t passive;  /**< passive scan time per channel, units: millisecond, values above 1500ms may
                            cause station to disconnect from AP and are not recommended. */
    uint16_t scan_timeout; /* Total timeout of scan, units: second. */
} wifi_scan_time_t;

/** @brief STA configuration settings for the LN882x */
typedef struct {
    uint8_t                    ssid[SSID_MAX_LEN];     /**<  SSID of target AP*/
    uint8_t                    password[PASSWORD_MAX_LEN]; /**<  password of target AP*/
    uint8_t                    bssid[BSSID_LEN];     /**<  MAC address of target AP*/
    uint8_t                    channel;      /**<  channel of target AP. Set to 1~13 to scan starting from the specified channel before connecting to AP. If the channel of AP is unknown, set it to 0.*/
    bool                       bssid_set;    /**<  whether set MAC address of target AP or not. Generally, station_config.bssid_set needs to be 0; and it needs to be 1 only when users need to check the MAC address of the AP.*/
} wifi_sta_config_t;

/** @brief Soft-AP configuration settings for the LN882x */
typedef struct {
    uint8_t               ssid[SSID_MAX_LEN];        /**<  SSID of LN882x soft-AP */
    uint8_t               password[PASSWORD_MAX_LEN];    /**<  Password of LN882x soft-AP */
    uint8_t               ssid_len;        /**<  Length of SSID. If softap_config.ssid_len==0, check the SSID until there is a termination character; otherwise, set the SSID length according to softap_config.ssid_len. */
    uint8_t               channel;         /**<  Channel of LN882x soft-AP */
    wifi_auth_mode_enum_t authmode;        /**<  Auth mode of LN882x soft-AP. Do not support AUTH_WEP in soft-AP mode */
    uint8_t               ssid_hidden;     /**<  Broadcast SSID or not, default 0, broadcast the SSID */
    uint8_t               max_connection;  /**<  Max number of stations allowed to connect in, default 4, max 4 */
    uint16_t              beacon_interval; /**<  Beacon interval, 100 ~ 60000 ms, default 100 ms */
    uint16_t              reserved;
} wifi_ap_config_t;

typedef union {
    wifi_ap_config_t  ap;  /**< configuration of AP */
    wifi_sta_config_t sta; /**< configuration of STA */
} wifi_config_t;

/** @brief Parameters for an SSID scan. */
typedef struct wifi_scan_config{
    uint8_t ssid[SSID_MAX_LEN];  /**< SSID of AP */
    uint8_t bssid[BSSID_LEN];    /**< MAC address of AP */
    uint8_t channel;             /**< channel, scan the specific channel */
    bool show_hidden;            /**< enable to scan AP whose SSID is hidden */
    wifi_scan_type_t scan_type;  /**< scan type, active or passive */
    wifi_scan_time_t scan_time;  /**< scan time per channel */
} wifi_scan_config_t;

typedef struct {
    bool     valid;
	uint8_t  id;       /* MAX = Maximum number of connections */
    uint8_t  bssid[6];
    uint32_t ip;
} station_info_t;

typedef struct {
    ln_list_t             list;
    uint8_t               bssid[BSSID_LEN];
    uint8_t               ssid[SSID_MAX_LEN];
    uint8_t               ssid_len;
    uint8_t               channel;
    wifi_auth_mode_enum_t authmode;
    uint8_t               imode;
    int8_t                rssi;
    uint8_t               is_hidden;
    int16_t               freq_offset;
    int16_t               freqcal_val;
    uint8_t               wps;
    int                   rest_lifetime;
} ap_info_t;

typedef struct {
    uint8_t password[PASSWORD_MAX_LEN];
    uint8_t password_len;
    uint8_t ssid[SSID_MAX_LEN];
    uint8_t ssid_len;
    uint8_t psk[40];
} wifi_psk_info_t;

void wifi_rf_preprocess(void);
void wifi_rf_image_cal(void);
void wifi_temp_cal_init(uint16_t adc_ch0_val,int8_t cap_comp);
void wifi_do_temp_cal_period(uint16_t adc_ch0_val);

//STA
int wifi_station_scan(wifi_scan_config_t *config);
int wifi_station_get_scan_list_size(void);
int wifi_station_get_scan_list(ln_list_t *out_list, int out_list_size, bool sort);
int wifi_station_connect(wifi_config_t *sta_config);
int wifi_station_disconnect(void);
int8_t  wifi_station_get_rssi(void);
int wifi_station_set_dtim_period(int period);

//AP
int wifi_softap_disconnect(void);

//COMMON
int wifi_init(void);
int wifi_deinit(void);
int wifi_start(wifi_init_type_t *init_param, bool auto_connect);
void wifi_stop(void);

/**
 * Set wifi mode and write it to flash args area.
*/
int wifi_set_mode(wifi_mode_enum_t mode);

/**
 * Set wifi mode, don't write it to flash args area.
*/
int wifi_set_mode_current(wifi_mode_enum_t mode);

/**
 * Read wifi mode from flash args area.
*/
wifi_mode_enum_t wifi_get_mode(void);

/**
 * Read wifi mode from RAM.
*/
wifi_mode_enum_t wifi_get_mode_current(void);

int wifi_get_config(wifi_interface_enum_t if_index, wifi_config_t *config);
int wifi_get_config_default (wifi_interface_enum_t if_index, wifi_config_t *config);
int wifi_set_config (wifi_interface_enum_t if_index, wifi_config_t *config);
int wifi_set_config_current (wifi_interface_enum_t if_index, wifi_config_t *config);
int wifi_get_config_current(wifi_interface_enum_t if_index, wifi_config_t *config);

int wifi_get_macaddr (wifi_interface_enum_t if_index, uint8_t *macaddr);//read from flash
int wifi_set_macaddr (wifi_interface_enum_t if_index, uint8_t *macaddr);
int wifi_set_macaddr_current (wifi_interface_enum_t if_index, uint8_t *macaddr);
int wifi_get_macaddr_current (wifi_interface_enum_t if_index, uint8_t *macaddr);//read from RAM
int wifi_set_11n_enable(uint8_t ht_enable);

uint8_t wifi_get_channel(void);
int wifi_set_channel (uint8_t channel);
/********************************************************************
function description: this function is to set dhcp status for wifi firmware
input parameter:      status : 0--dhcp not complete  1--dhcp complete(got ip)
output parameter:     none
return :           the status indicate whether config to firmware success or not
                    0--success     1--failed
********************************************************************/
int wifi_set_dhcp_status (uint8_t status);
int wifi_get_link_status (wifi_interface_enum_t if_index, int *status);
int wifi_set_country_code(wifi_country_code_enum_t country_code);
int wifi_get_country_code(wifi_country_code_enum_t *country_code);
int wifi_station_set_powersave(sta_powersave_mode_enum_t ps_mode);
sta_powersave_mode_enum_t wifi_station_get_powersave(void);
int wifi_set_psk_info(wifi_interface_enum_t if_index, wifi_psk_info_t *psk_info);
int wifi_get_psk_info(wifi_interface_enum_t if_index, wifi_psk_info_t *psk_info);

int wifi_private_command(char *pvtcmd);


#define WIFI_PKT_MGMT                       (0)             /**< sniffer packet is management frame */
#define WIFI_PKT_CTRL                       (1)             /**< sniffer packet is control frame */
#define WIFI_PKT_DATA                       (2)             /**< sniffer packet is data frame */

#define WIFI_PROMIS_FILTER_MASK_MGMT        (1UL << 0U)     /**< WIFI_PKT_MGMT packet filter mask */
#define WIFI_PROMIS_FILTER_MASK_CTRL        (1UL << 1U)     /**< WIFI_PKT_CTRL packet filter mask */
#define WIFI_PROMIS_FILTER_MASK_DATA        (1UL << 2U)     /**< WIFI_PKT_DATA packet filter mask */
#define WIFI_PROMIS_FILTER_MASK_ALL         (0xFFFFFFFF)    /**< all packets filter mask */

typedef void (* wifi_promiscuous_cb_t)(void *buf, uint16_t len, uint8_t pkt_type);

void wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb);
int  wifi_set_promiscuous(bool en);
int  wifi_get_promiscuous(bool *en);
int  wifi_set_promiscuous_filter(uint32_t filter_mask);
int  wifi_get_promiscuous_filter(uint32_t *filter_mask);



//wifi advanced API
int wifi_send_80211_mgmt_raw_frame(uint8_t *data, int len);

int wifi_if_send_ethernet_pkt(uint8_t *data, int len, uint16_t retry_max, uint8_t retry_timeout);


// wifi lib version
uint32_t    wifi_lib_version_number_get(void);
const char *wifi_lib_version_string_get(void);
const char *wifi_lib_build_time_get(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __WIFI_H__ */
