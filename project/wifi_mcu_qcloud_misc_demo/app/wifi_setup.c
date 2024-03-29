#include "ln_types.h"
#include "hal/hal_sleep.h"
#include "utils/art_string.h"
#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"
#include "wifi/wifi.h"
#include "netif/ethernetif.h"
#include "airkiss_entry.h"
#include "ln_softapcfg_api.h"
#include "ln_smartcfg_api.h"

/**
 * @brief
 *
 */
static void _wifi_init_sta(void)
{
    uint8_t macaddr[6] = {0}, macaddr_default[6] = {0};
    wifi_config_t wifi_config = {0};

    wifi_init_type_t init_param = {
        .wifi_mode = WIFI_MODE_STATION,
        .sta_ps_mode = WIFI_NO_POWERSAVE, // WIFI_NO_POWERSAVE, WIFI_MIN_POWERSAVE
        .dhcp_mode = WLAN_DHCP_CLIENT,
        .scanned_ap_list_size = SCANNED_AP_LIST_SIZE,
    };

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
    if(0 != wifi_start(&init_param, true)){//WIFI_MAX_POWERSAVE
        LOG(LOG_LVL_ERROR, "[%s, %d]wifi_start() fail.\r\n", __func__, __LINE__);
    }
}

static void _connect_to_ap(uint8_t * ssid, uint8_t * pwd)
{
    wifi_config_t wifi_config = {0,};
    wifi_scan_config_t scan_config = {0,};

    memset(&wifi_config, 0, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, ssid, strlen((const char *)ssid));
    memcpy(wifi_config.sta.password, pwd, strlen((const char *)pwd));
    // wifi_set_config_current(STATION_IF, &wifi_config); // NOTE: 这里不能调用这个API
    wifi_set_config(STATION_IF, &wifi_config); // NOTE: 调用这个才是OK的。

    memset(&scan_config, 0, sizeof(wifi_scan_config_t));
    if (strlen((const char *)wifi_config.sta.ssid) > 0) {
        memcpy(scan_config.ssid, wifi_config.sta.ssid, strlen((const char *)wifi_config.sta.ssid));
    }
    scan_config.channel = 0;
    scan_config.show_hidden = false;
    scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    scan_config.scan_time.active = 20;
    scan_config.scan_time.passive = 40;
    wifi_station_scan(&scan_config);
}

/**
 * @brief
 * @note The AP which the phone is connected to must be in 2.4GHz.
 */
void setup_wifi_via_airkiss(void)
{
    uint16_t channel_mask = 0;
    uint8_t * ssid = NULL;
    uint8_t * pwd = NULL;

    //Set sleep mode
    hal_sleep_set_mode(ACTIVE);

    _wifi_init_sta();

    channel_mask = CH1_MASK | CH2_MASK | CH3_MASK | CH4_MASK | CH5_MASK | CH6_MASK | \
                   CH7_MASK | CH8_MASK | CH9_MASK | CH10_MASK | CH11_MASK | CH12_MASK | CH13_MASK;

    if (!airkiss_start(channel_mask)) {
        LOG(LOG_LVL_INFO, "failed to start airkiss...\r\n");
    }

    while (!airkiss_is_complete()) {
        OS_MsDelay(300);
    }

    airkiss_stop();

    ssid = airkiss_get_ssid();
    pwd  = airkiss_get_pwd();
    _connect_to_ap(ssid, pwd);

    while(LINK_UP != ethernetif_get_link_state()) {
        OS_MsDelay(200);
    }

    airkiss_send_ack();
    airkiss_recv_and_reply_token();
}

/**
 * @brief The mobile phone connects to this softAP and pass ssid, password to this module.
 *
 */
void setup_wifi_via_softap(void)
{
    uint8_t *ssid = NULL;
    uint8_t *pwd = NULL;

    hal_sleep_set_mode(ACTIVE);

    ln_softapcfg_set_product_param("DFK2GJBO5I", "light003", "2.0");

    if (LN_TRUE != ln_softapcfg_start("softap_cfg", "12345678", "192.168.4.1")) {
        LOG(LOG_LVL_ERROR, "start softapcfg failed!!!\r\n");
    }

    while(LN_TRUE != ln_softapcfg_is_complete()) {
        OS_MsDelay(300);
    }

    ln_softapcfg_stop();

    ssid = ln_softapcfg_get_ssid();
    pwd = ln_softapcfg_get_pwd();

    _wifi_init_sta();
    _connect_to_ap(ssid, pwd);

    LOG(LOG_LVL_ERROR, "token: %s\r\n", (const char *)ln_softapcfg_get_token());

    while (LINK_UP != ethernetif_get_link_state()) {
        OS_MsDelay(300);
    }
}

/**
 * @brief
 * @note The AP which the mobile phone is connected to must be in 2.4GHz.
 *
 */
void setup_wifi_via_smartconfig(void)
{
    uint16_t channel_map = 0;
    uint8_t * ssid = NULL;
    uint8_t * pwd = NULL;

    _wifi_init_sta();

    channel_map = CHANNEL1_MASK | CHANNEL2_MASK | CHANNEL3_MASK | CHANNEL4_MASK | \
                  CHANNEL5_MASK | CHANNEL6_MASK | CHANNEL7_MASK | CHANNEL8_MASK | \
                  CHANNEL9_MASK | CHANNEL10_MASK | CHANNEL11_MASK | CHANNEL12_MASK | CHANNEL13_MASK;

    if (LN_TRUE != ln_smartconfig_start(channel_map)) {
        LOG(LOG_LVL_INFO, "failed to start smartconfig...\r\n");
    }

    while (LN_TRUE != ln_smartconfig_is_complete()) {
        OS_MsDelay(300);
    }

    ln_smartconfig_stop();

    ssid = ln_smartconfig_get_ssid();
    pwd  = ln_smartconfig_get_pwd();
    _connect_to_ap(ssid, pwd);

    //Wait for network link up
    while(LINK_UP != ethernetif_get_link_state()){
        OS_MsDelay(300);
    }

    ln_smartconfig_send_ack();
}

/**
 * @brief
 * @note The AP which the phone is connected to must be in 2.4GHz.
 */
void setup_wifi_direct(void)
{
    uint16_t channel_mask = 0;
    uint8_t ssid[] = "TP-LINK_479A64";
    uint8_t pwd[] = "";

    //Set sleep mode
    hal_sleep_set_mode(ACTIVE);

    _wifi_init_sta();

    _connect_to_ap(ssid, pwd);

    while(LINK_UP != ethernetif_get_link_state()) {
        OS_MsDelay(200);
    }

    airkiss_send_ack();
}
