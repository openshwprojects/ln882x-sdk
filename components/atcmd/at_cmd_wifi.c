#include "proj_config.h"
#include "at_cmd_wifi.h"
#include "at_parser.h"
#include "at_string.h"
#include "ln_utils.h"
#include "wifi/wifi.h"
#include "wifi_manager.h"
#include "netif/ethernetif.h"
#include "lwip/tcpip.h"
#include "ping.h"
#include "iperf.h"
#include "dhcpd.h"
#include "ln_nvds.h"
#include "ln_kv_api.h"
#include "hal/hal_efuse.h"
#include "hal/flash.h"
#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"
#include "utils/art_string.h"
#include "utils/system_parameter.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define RET_OK_STR   ("\r\nOK\r\n")
#define RET_ERR_STR  ("\r\nERROR\r\n")
#define RET_FAIL_STR ("\r\nFAIL\r\n")

typedef struct {
    unsigned char sort_enable;
    unsigned int  mask;
} scan_list_dsiplay_option_t;
static scan_list_dsiplay_option_t g_scan_list_dsiplay_option = {0,};

static  OS_Timer_t g_con_timeout_handle={0} ;
void connect_timeout_alarm_fn(void *arg)
{
    wifi_config_t config = {0,};
    if(LINK_DOWN == ethernetif_get_link_state()){
        at_printf("\r\nCONNECT FAILED\r\n");
        wifi_set_config(STATION_IF, &config);
        wifi_station_disconnect();
    }else{
        ;//do nothing if connect success
    }
}

void at_connect_timeout(uint32_t timeout_ms)
{
    uint8_t ret;

    if(g_con_timeout_handle.handle){
        OS_TimerStop(&g_con_timeout_handle);
        OS_TimerDelete(&g_con_timeout_handle);
    }

    ret = OS_TimerCreate(&g_con_timeout_handle, OS_TIMER_ONCE, connect_timeout_alarm_fn, NULL, timeout_ms);
    if(ret == OS_OK){
        OS_TimerStart(&g_con_timeout_handle);
    }
}

char at_station_setmac(char *str)
{
    uint8_t addr[6] = {0,};

    char mac_str[3*MAC_ADDRESS_LEN] = {0,};
    char *p = NULL;
    int start = 0, end = 0;

    ART_ASSERT(str);
    //mac string
    p = str + end;
    if(p < str + strlen(str)){
        start = (strchr(p, '\"') + 1) - str;
        p = str +start;
        end = strchr(p, '\"') - str;
        if(!substing(str, mac_str, start, end)){
            goto err;
        }
    }

    ln_mac_str2hex((const uint8_t *)mac_str, addr);
    if(0 != wifi_set_macaddr(STATION_IF, addr)){
        goto err;
    }
    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_station_getmac(char *str)
{
    uint8_t addr[6] = {0x00,};

    if(0 != wifi_get_macaddr(STATION_IF, addr)){
        goto err;
    }
    at_printf("\r\n+CIPSTAMAC: "MACSTR"\r\n", MAC2STR(addr));
    at_printf(RET_OK_STR);

    return AT_OK;
err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_station_setmac_current(char *str)
{
    uint8_t addr[6] = {0,};

    char mac_str[3*MAC_ADDRESS_LEN] = {0,};
    char *p = NULL;
    int start = 0, end = 0;

    ART_ASSERT(str);
    //mac string
    p = str + end;
    if(p < str + strlen(str)){
        start = (strchr(p, '\"') + 1) - str;
        p = str +start;
        end = strchr(p, '\"') - str;
        if(!substing(str, mac_str, start, end)){
            goto err;
        }
    }

    ln_mac_str2hex((const uint8_t *)mac_str, addr);
    if(0 != wifi_set_macaddr_current(STATION_IF, addr)){
        goto err;
    }
    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_station_getmac_current(char *str)
{
    uint8_t addr[6] = {0x00,};

    if(0 != wifi_get_macaddr_current(STATION_IF, addr)){
        goto err;
    }
    at_printf("\r\n+CIPSTAMAC_CUR: "MACSTR"\r\n", MAC2STR(addr));
    at_printf(RET_OK_STR);

    return AT_OK;
err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_station_setmac_def(char *str)
{
    uint8_t addr[6] = {0,};

    char mac_str[3*MAC_ADDRESS_LEN] = {0,};
    char *p = NULL;
    int start = 0, end = 0;

    ART_ASSERT(str);
    //mac string
    p = str + end;
    if(p < str + strlen(str)){
        start = (strchr(p, '\"') + 1) - str;
        p = str +start;
        end = strchr(p, '\"') - str;
        if(!substing(str, mac_str, start, end)){
            goto err;
        }
    }

    ln_mac_str2hex((const uint8_t *)mac_str, addr);
    if(0 != wifi_set_macaddr(STATION_IF, addr)){
        goto err;
    }
    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_station_getmac_def(char *str)
{
    uint8_t addr[6] = {0x00,};

    if(0 != wifi_get_macaddr(STATION_IF, addr)){
        goto err;
    }
    at_printf("\r\n+CIPSTAMAC_DEF: "MACSTR"\r\n", MAC2STR(addr));
    at_printf(RET_OK_STR);

    return AT_OK;
err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}

char at_softap_setmac(char *str)
{
    uint8_t addr[6] = {0,};

    char mac_str[3*MAC_ADDRESS_LEN] = {0,};
    char *p = NULL;
    int start = 0, end = 0;

    ART_ASSERT(str);
    //mac string
    p = str + end;
    if(p < str + strlen(str)){
        start = (strchr(p, '\"') + 1) - str;
        p = str +start;
        end = strchr(p, '\"') - str;
        if(!substing(str, mac_str, start, end)){
            goto err;
        }
    }

    ln_mac_str2hex((const uint8_t *)mac_str, addr);
    if(0 != wifi_set_macaddr(SOFT_AP_IF, addr)){
        goto err;
    }
    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_softap_getmac(char *str)
{
    uint8_t addr[6] = {0x00,};

    if(0 != wifi_get_macaddr(SOFT_AP_IF, addr)){
        goto err;
    }
    at_printf("\r\n+CIPAPMAC: "MACSTR"\r\n", MAC2STR(addr));
    at_printf(RET_OK_STR);

    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_softap_setmac_current(char *str)
{
    uint8_t addr[6] = {0,};

    char mac_str[3*MAC_ADDRESS_LEN] = {0,};
    char *p = NULL;
    int start = 0, end = 0;

    ART_ASSERT(str);
    //mac string
    p = str + end;
    if(p < str + strlen(str)){
        start = (strchr(p, '\"') + 1) - str;
        p = str +start;
        end = strchr(p, '\"') - str;
        if(!substing(str, mac_str, start, end)){
            goto err;
        }
    }

    ln_mac_str2hex((const uint8_t *)mac_str, addr);
    if(0 != wifi_set_macaddr_current(SOFT_AP_IF, addr)){
        goto err;
    }
    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_softap_getmac_current(char *str)
{
    uint8_t addr[6] = {0x00,};

    if(0 != wifi_get_macaddr_current(SOFT_AP_IF, addr)){
        goto err;
    }
    at_printf("\r\n+CIPAPMAC_CUR: "MACSTR"\r\n", MAC2STR(addr));
    at_printf(RET_OK_STR);

    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_softap_setmac_def(char *str)
{
    uint8_t addr[6] = {0,};

    char mac_str[3*MAC_ADDRESS_LEN] = {0,};
    char *p = NULL;
    int start = 0, end = 0;

    ART_ASSERT(str);
    //mac string
    p = str + end;
    if(p < str + strlen(str)){
        start = (strchr(p, '\"') + 1) - str;
        p = str +start;
        end = strchr(p, '\"') - str;
        if(!substing(str, mac_str, start, end)){
            goto err;
        }
    }

    ln_mac_str2hex((const uint8_t *)mac_str, addr);
    if(0 != wifi_set_macaddr(SOFT_AP_IF, addr)){
        goto err;
    }
    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}
char at_softap_getmac_def(char *str)
{
    uint8_t addr[6] = {0x00,};

    if(0 != wifi_get_macaddr(SOFT_AP_IF, addr)){
        goto err;
    }
    at_printf("\r\n+CIPAPMAC_DEF: "MACSTR"\r\n", MAC2STR(addr));
    at_printf(RET_OK_STR);

    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}

//private command
char at_pvtcmd_set(char *str)
{
    ART_ASSERT(str);
    if(0 != wifi_private_command(str)){
        goto err;
    }
    at_printf(RET_OK_STR);
    return AT_OK;
err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}

static wifi_init_type_t init_param = {0};

char at_set_wifi_mode(char *str)
{
    wifi_mode_enum_t mode = WIFI_MODE_STATION;

    ART_ASSERT(str && strlen(str) == 1);

    mode = (wifi_mode_enum_t)atoi(str);

    if (mode == 1) {
        mode  = WIFI_MODE_STATION;
    } else if (mode == 2) {
        mode  = WIFI_MODE_AP;
    } else if (mode == 3) {
        mode  = WIFI_MODE_AP_STATION;
    }

    if(mode >= WIFI_MODE_MAX){
        goto err;
    }

    wifi_stop();

    if (0 != wifi_set_mode(mode)) {
        goto err;
    }

    if(mode == WIFI_MODE_STATION){
        init_param.wifi_mode = WIFI_MODE_STATION;
        init_param.sta_ps_mode = WIFI_MAX_POWERSAVE;
        init_param.dhcp_mode = WLAN_DHCP_CLIENT;
        init_param.scanned_ap_list_size = SCANNED_AP_LIST_SIZE;
        if (0 != wifi_start(&init_param, true)) {
            goto err;
        }
    }

    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;
}

char at_get_wifi_mode(char *str)
{
    wifi_mode_enum_t mode;

    mode = wifi_get_mode();
    if (mode == WIFI_MODE_STATION) {
        mode  = (wifi_mode_enum_t)1;
    } else if (mode == WIFI_MODE_AP) {
        mode  = (wifi_mode_enum_t)2;
    } else if (mode == WIFI_MODE_AP_STATION) {
        mode  = (wifi_mode_enum_t)3;
    }

    at_printf("\r\n+CWMODE: %d\r\n", mode);
    at_printf(RET_OK_STR);

    return AT_OK;
}

char at_help_wifi_mode(char *str)
{

    at_printf("\r\n+CWMODE: 1, 2, 3\r\n");
    at_printf("\t1 -- Station Mode\r\n");
    at_printf("\t2 -- SoftAP Mode\r\n");
    at_printf("\t3 -- SoftAP+Station Mode\r\n");
    at_printf(RET_OK_STR);

    return AT_OK;
}

char at_set_wifi_mode_current(char *str)
{
    wifi_mode_enum_t mode;

    ART_ASSERT(str && strlen(str) == 1);

    mode = (wifi_mode_enum_t)atoi(str);

    if (mode == 1) {
        mode  = WIFI_MODE_STATION;
    } else if (mode == 2) {
        mode  = WIFI_MODE_AP;
    } else if (mode == 3) {
        mode  = WIFI_MODE_AP_STATION;
    }

    wifi_stop();

    if(0 != wifi_set_mode_current(mode)){
        goto err;
    }

    init_param.wifi_mode = WIFI_MODE_STATION;
    init_param.sta_ps_mode = WIFI_MAX_POWERSAVE;
    init_param.dhcp_mode = WLAN_DHCP_CLIENT;
    init_param.scanned_ap_list_size = SCANNED_AP_LIST_SIZE;
    if (0 != wifi_start(&init_param, true)) {
        goto err;
    }

    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}

char at_get_wifi_mode_current(char *str)
{
    wifi_mode_enum_t mode;

    mode = wifi_get_mode_current();
    if (mode == WIFI_MODE_STATION) {
        mode  = (wifi_mode_enum_t)1;
    } else if (mode == WIFI_MODE_AP) {
        mode  = (wifi_mode_enum_t)2;
    } else if (mode == WIFI_MODE_AP_STATION) {
        mode  = (wifi_mode_enum_t)3;
    }

    at_printf("\r\n+CWMODE_CUR: %d\r\n", mode);
    at_printf(RET_OK_STR);

    return AT_OK;

}

char at_help_wifi_mode_current(char *str)
{
    at_printf("\r\n+CWMODE_CUR: 1, 2, 3\r\n");
    at_printf("\t1 -- Station Mode\r\n");
    at_printf("\t2 -- SoftAP Mode\r\n");
    at_printf("\t3 -- SoftAP+Station Mode\r\n");
    at_printf(RET_OK_STR);

    return AT_OK;

}
char at_set_wifi_mode_def(char *str)
{
    wifi_mode_enum_t mode;

    ART_ASSERT(str && strlen(str) == 1);

    mode = (wifi_mode_enum_t)atoi(str);

    if (mode == 1) {
        mode  = WIFI_MODE_STATION;
    } else if (mode == 2) {
        mode  = WIFI_MODE_AP;
    } else if (mode == 3) {
        mode  = WIFI_MODE_AP_STATION;
    }

    wifi_stop();

    if(0 != wifi_set_mode(mode)){
        goto err;
    }

    init_param.wifi_mode = WIFI_MODE_STATION;
    init_param.sta_ps_mode = WIFI_MAX_POWERSAVE;
    init_param.dhcp_mode = WLAN_DHCP_CLIENT;
    init_param.scanned_ap_list_size = SCANNED_AP_LIST_SIZE;
    if (0 != wifi_start(&init_param, true)) {
        goto err;
    }

    at_printf(RET_OK_STR);
    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;

}

char at_get_wifi_mode_def(char *str)
{
    wifi_mode_enum_t mode;

    mode = wifi_get_mode();
    if (mode == WIFI_MODE_STATION) {
        mode  = (wifi_mode_enum_t)1;
    } else if (mode == WIFI_MODE_AP) {
        mode  = (wifi_mode_enum_t)2;
    } else if (mode == WIFI_MODE_AP_STATION) {
        mode  = (wifi_mode_enum_t)3;
    }

    at_printf("\r\n+CWMODE_DEF: %d\r\n", mode);
    at_printf(RET_OK_STR);

    return AT_OK;
}

char at_help_wifi_mode_def(char *str)
{
    at_printf("\r\n+CWMODE_DEF: 1, 2, 3\r\n");
    at_printf("\t*1 -- Station Mode\r\n");
    at_printf("\t*2 -- SoftAP Mode\r\n");
    at_printf("\t*3 -- SoftAP+Station Mode\r\n");
    at_printf(RET_OK_STR);

    return AT_OK;

}



static bool at_station_connect_general(char *str, bool save_to_flash)
{
    char temp_str[SSID_MAX_LEN] = {0,};
    char *domain_begin = NULL, *domain_end = NULL, *domain_separator_addr = NULL, *limit = str + strlen(str);
    char domain_separator = ',', content_flag = '\"';
    wifi_config_t wifi_config = {0,};
    unsigned int timeout_ms=0;

    ART_ASSERT(str);
    /*
     * string format: <ssid>,<pwd>[,<bssid>]
     *
     */
    //ssid
    domain_begin = str;
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, &content_flag, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        memcpy(wifi_config.sta.ssid, temp_str, strlen(temp_str));
    }

    //password
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto partial_config;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, sizeof(SSID_MAX_LEN));
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, &content_flag, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        memcpy(wifi_config.sta.password, temp_str, strlen(temp_str));
    }

    //connect timeout
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto partial_config;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, sizeof(SSID_MAX_LEN));
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, &content_flag, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        timeout_ms=atoi(temp_str);
    }

    //bssid
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto partial_config;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, sizeof(SSID_MAX_LEN));
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, &content_flag, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0 && ln_is_valid_mac_str(temp_str)){
        ln_mac_str2hex((const uint8_t *)temp_str, wifi_config.sta.bssid);
    }


partial_config:
    if(save_to_flash){
        if(0 != wifi_set_config(STATION_IF, &wifi_config)){
            goto err;
        }
    }else{
        if(0 != wifi_set_config_current(STATION_IF, &wifi_config)){
            goto err;
        }
    }

    wifi_station_connect(&wifi_config);

    if(timeout_ms){
        at_connect_timeout(timeout_ms);
    }

    return true;

err:
    return false;
}
//set get connect(softAP mode)
char at_station_connect(char *str)
{
    if(at_station_connect_general(str, true)){
        if(LINK_UP == ethernetif_get_link_state()){
            at_printf("disconnect\r\n");
        }
        at_printf(RET_OK_STR);
        return AT_OK;
    }else{
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
}
char at_station_connect_current(char *str)
{
    if(at_station_connect_general(str, false)){
        if(LINK_UP == ethernetif_get_link_state()){
            at_printf("disconnect\r\n");
        }
        at_printf(RET_OK_STR);
        return AT_OK;
    }else{
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
}

char at_station_get_connected_info(char *str)
{

    int rssi = 0;
    wifi_config_t wifi_config = {0,};
    wifi_sta_config_t *sta = &wifi_config.sta;
    if(LINK_UP != ethernetif_get_link_state()){
        at_printf(RET_OK_STR);
        return AT_OK;
    }else{
        wifi_get_config(STATION_IF, &wifi_config);
        rssi = wifi_station_get_rssi();
        at_printf("\r\n+CWJAP: %s, "MACSTR", %d, %d\r\n", sta->ssid, MAC2STR(sta->bssid), sta->channel, rssi);
        at_printf(RET_OK_STR);
        return AT_OK;
    }

}
char at_station_get_connected_info_current(char *str)
{

    int rssi = 0;
    wifi_config_t wifi_config = {0,};
    wifi_sta_config_t *sta = &wifi_config.sta;
    if(LINK_UP != ethernetif_get_link_state()){
        at_printf(RET_OK_STR);
        return AT_OK;
    }else{
        wifi_get_config_current(STATION_IF, &wifi_config);
        rssi = wifi_station_get_rssi();
        at_printf("\r\n+CWJAP_CUR: %s, "MACSTR", %d, %d\r\n", sta->ssid, MAC2STR(sta->bssid), sta->channel, rssi);
        at_printf(RET_OK_STR);
        return AT_OK;
    }

}
char at_station_get_connected_info_def(char *str)
{

    int rssi = 0;
    wifi_config_t wifi_config = {0,};
    wifi_sta_config_t *sta = &wifi_config.sta;
    if(LINK_UP != ethernetif_get_link_state()){
        at_printf(RET_OK_STR);
        return AT_OK;
    }else{
        wifi_get_config(STATION_IF, &wifi_config);
        rssi = wifi_station_get_rssi();
        at_printf("\r\n+CWJAP_DEF: %s, "MACSTR", %d, %d\r\n", sta->ssid, MAC2STR(sta->bssid), sta->channel, rssi);
        at_printf(RET_OK_STR);
        return AT_OK;
    }

}

//set scan list option
char at_station_set_scan_list_display_option(char *str)
{
    scan_list_dsiplay_option_t *dsiplay_option = &g_scan_list_dsiplay_option;
    char temp_str[3*BSSID_LEN] = {0};
    char *p = NULL;
    int start = 0, end = 0;

    ART_ASSERT(str && dsiplay_option);
    //sort_enable
    p = str + end;
    if(p < str + strlen(str)){
        start = p - str;
        p = str +start;
        end = strchr(p, ',') - str;
        substing(str, (char *)temp_str, start, end);
    }
    dsiplay_option->sort_enable = atoi(temp_str);

    //mask
    p = str + end + 1;
    if(p < str + strlen(str)){
        start = p - str;
        end = strlen(str);
        memset(temp_str, 0, 3*BSSID_LEN);
        substing(str, (char *)temp_str, start, end);
    }
    dsiplay_option->mask = atoi(temp_str);
    at_printf(RET_OK_STR);
    return AT_OK;

}
static OS_Semaphore_t              g_scan_sem;
static void at_station_scan_done_msg_cb(wifi_msg_t *msg)
{
    if(WIFI_MSG_ID_STA_SCAN_DONE == msg->msg_id){
        OS_SemaphoreRelease(&g_scan_sem);
    }
}

char at_station_scan_no_filter(char *str)
{
    int i;
    wifi_mode_enum_t mode;
    scan_list_dsiplay_option_t *display_option = &g_scan_list_dsiplay_option;
    int ap_count = 0;
    ap_info_t *item_iterator = NULL;
    ln_list_t scan_list, *iterator, *prev;
    wifi_scan_config_t scan_cfg = {0,};

    /* WiFi mode check, only STA mode support this command. */
    if(WIFI_MODE_STATION != wifi_get_mode()){
        goto err;
    }
#if WIFI_TRACK
    at_printf("\r\n+CWLAPLN\r\n");
#else
    at_printf("\r\n+CWLAP\r\n");
#endif

    mode = wifi_get_mode();
    if (LINK_UP == ethernetif_get_link_state())
    {
        scan_cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
        scan_cfg.channel = 0;
        scan_cfg.show_hidden = false;
        scan_cfg.scan_time.active = 30;
        scan_cfg.scan_time.passive = 120;
        reg_wifi_msg_callbcak(WIFI_MSG_ID_STA_SCAN_DONE, at_station_scan_done_msg_cb);
        if (OS_OK != OS_SemaphoreCreateBinary(&g_scan_sem)) {
            goto err;
        }
        wifi_station_scan(&scan_cfg);
        OS_SemaphoreWait(&g_scan_sem, OS_WAIT_FOREVER);
        reg_wifi_msg_callbcak(WIFI_MSG_ID_STA_SCAN_DONE, NULL);
        if (OS_OK != OS_SemaphoreDelete(&g_scan_sem)) {
            goto err;
        }
    }

    ap_count = wifi_station_get_scan_list_size();
    if(ap_count > 0){
        //prepare scan_list space
        ln_list_init(&scan_list);
        for(i = 0; i < ap_count; i++){
            item_iterator = OS_Malloc(sizeof(ap_info_t));
            if(item_iterator){
                ln_list_add(&scan_list, &(item_iterator->list));
            }
        }

        //get ap list
        wifi_station_get_scan_list(&scan_list, ap_count, (display_option->sort_enable == 1)? true : false);
        at_printf("Auth{ 0: Open, 1: WEP, 2: WPA_PSK, 3:WPA2_PSK, 4:WPA_WPA2_PSK, 5: WPA2_ENTERPRISE}\r\n");
        at_printf ("\r\nAP_num SSID                            RSSI Channel Auth_mode  BSSID              IMODE WPS Freq_offset FreqCal\r\n");
        for (iterator = scan_list.next, i = 0; iterator != &scan_list; iterator = iterator->next, i++) {
            item_iterator = ln_list_entry(iterator, ap_info_t, list);
            at_printf ("%6d %-31s %4d %7d %9d  "MACSTR"  %2X    %3d %11d %7d\r\n",
                        (i + 1), item_iterator->ssid, item_iterator->rssi, item_iterator->channel, item_iterator->authmode,
                        MAC2STR(item_iterator->bssid),
                        item_iterator->imode, item_iterator->wps, item_iterator->freq_offset, item_iterator->freqcal_val
                        );
        }

        //free scan_list space
        for (iterator = scan_list.next; iterator != &scan_list; iterator = iterator->next) {
            prev = iterator->prev;
            item_iterator = ln_list_entry(iterator, ap_info_t, list);
            ln_list_rm(iterator);
            if(item_iterator){
                OS_Free(item_iterator);
            }
            iterator = prev;
        }
    }

    at_printf(RET_OK_STR);

    return AT_OK;

err:
    at_printf(RET_FAIL_STR);
    return AT_ERROR;
}
#if WIFI_TRACK
#define APLIST_USED_FLAG (1)
char at_notify_aplist(void)
{
    int i=0;
    int ap_count = 0;
    ap_info_t *item_iterator = NULL;
    list_t scan_list, *iterator, *prev;

    ap_count = wifi_station_get_scan_list_size();
    if(ap_count > 0){
        //prepare scan_list space
        list_init(&scan_list);
        for(i = 0; i < ap_count; i++){
            item_iterator = OS_Malloc(sizeof(ap_info_t));
            if(item_iterator){
                list_add(&scan_list, &(item_iterator->list));
            }
        }

        //get ap list
        wifi_station_get_scan_list(&scan_list, ap_count, true);
        at_printf("\r\nAT+ScanAp\r\n",ap_count);
        for (iterator = scan_list.next, i = 0; (i < 10) && iterator != &scan_list; iterator = iterator->next, i++) {
            item_iterator = list_entry(iterator, ap_info_t, list);
            at_printf(MACSTR";%d;%s\r\n", MAC2STR(item_iterator->bssid), item_iterator->rssi, item_iterator->ssid);
        }

        //free scan_list space
        for (iterator = scan_list.next; iterator != &scan_list; iterator = iterator->next) {
            prev = iterator->prev;
            item_iterator = list_entry(iterator, ap_info_t, list);
            list_rm(iterator);
            if(item_iterator){
                OS_Free(item_iterator);
            }
            iterator = prev;
        }
    }
    return AT_OK;
}

char at_station_aplx(char *str)
{
    int ap_count = 0, i;
    ap_info_t *item_iterator = NULL;
    list_t scan_list, *iterator, *prev;

    ap_count = wifi_station_get_scan_list_size();
    if(ap_count > 0){
        //prepare scan_list space
        list_init(&scan_list);
        for(i = 0; i < ap_count; i++){
            item_iterator = OS_Malloc(sizeof(ap_info_t));
            if(item_iterator){
                list_add(&scan_list, &(item_iterator->list));
            }
        }

        //get ap list
        wifi_station_get_scan_list(&scan_list, ap_count, true);
        for (iterator = scan_list.next; iterator != &scan_list; iterator = iterator->next) {
            item_iterator = list_entry(iterator, ap_info_t, list);
            at_printf("\r\n+CWLAP:%d,\"%s\",%d,\""MACSTR"\",%d",
                                            item_iterator->authmode,
                                            item_iterator->ssid,
                                            item_iterator->rssi,
                                            MAC2STR(item_iterator->bssid),
                                            item_iterator->channel);
        }

        //free scan_list space
        for (iterator = scan_list.next; iterator != &scan_list; iterator = iterator->next) {
            prev = iterator->prev;
            item_iterator = list_entry(iterator, ap_info_t, list);
            list_rm(iterator);
            if(item_iterator){
                OS_Free(item_iterator);
            }
            iterator = prev;
        }
    }
    at_printf("\r\n\r\nOK\r\n",ap_count);
    return AT_OK;
}
char at_station_aplist(char *str)
{
    int ap_count = 0, i;
    ap_info_t *item_iterator = NULL;
    list_t scan_list, *iterator, *prev;

    ap_count = wifi_station_get_scan_list_size();
    if(ap_count > 0){
        //prepare scan_list space
        list_init(&scan_list);
        for(i = 0; i < ap_count; i++){
            item_iterator = OS_Malloc(sizeof(ap_info_t));
            if(item_iterator){
                list_add(&scan_list, &(item_iterator->list));
            }
        }

        //get ap list
        wifi_station_get_scan_list(&scan_list, ap_count, true);
        at_printf("\r\n+CWLAPLST: %d, ",ap_count);
        for (iterator = scan_list.next; iterator != &scan_list; iterator = iterator->next) {
            item_iterator = list_entry(iterator, ap_info_t, list);
            at_printf(MACSTR" @ %d, ", MAC2STR(item_iterator->bssid), item_iterator->rssi);
        }

        //free scan_list space
        for (iterator = scan_list.next; iterator != &scan_list; iterator = iterator->next) {
            prev = iterator->prev;
            item_iterator = list_entry(iterator, ap_info_t, list);
            list_rm(iterator);
            if(item_iterator){
                OS_Free(item_iterator);
            }
            iterator = prev;
        }
        at_printf("\r\n\r\nOK\r\n");
    }
    else
    {
        at_printf("\r\n+CWLAPLST: 0\r\n\r\nOK\r\n",ap_count);
    }
    return AT_OK;
}
#endif
static bool at_station_parse_scan_string(char *scan_str, wifi_scan_config_t *scan_cfg)
{
    char temp_str[SSID_MAX_LEN] = {0,};
    char *domain_begin = NULL, *domain_end = NULL, *domain_separator_addr = NULL, *limit = scan_str + strlen(scan_str);
    char domain_separator = ',', content_flag = '\"';

    ART_ASSERT(scan_str && scan_cfg);

    /*
     * string format: <ssid>[,<mac>,<channel>,<scan_type>,<scan_time_min>,<scan_time_max>]
     *
     */
    //ssid
    domain_begin = scan_str;
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, &content_flag, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        memcpy(scan_cfg->ssid, temp_str, strlen(temp_str));
    }


    //mac address
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto partial_config;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, sizeof(SSID_MAX_LEN));
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, &content_flag, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0 && ln_is_valid_mac_str(temp_str)){
        ln_mac_str2hex((const uint8_t *)temp_str, scan_cfg->bssid);
    }


    //channel
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto partial_config;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, sizeof(SSID_MAX_LEN));
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, NULL, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        scan_cfg->channel = atoi(temp_str);
    }

    //scan_type
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto partial_config;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, sizeof(SSID_MAX_LEN));
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, NULL, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        scan_cfg->scan_type = (wifi_scan_type_t)atoi(temp_str);
    }

    //scan_time_min
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto partial_config;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, sizeof(SSID_MAX_LEN));
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, NULL, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        scan_cfg->scan_time.active = atoi(temp_str);
    }

partial_config:
    //show_hidden
    scan_cfg->show_hidden = false;

    return true;

err:
    return false;
}

char at_station_scan(char *str)
{
    wifi_scan_config_t scan_cfg = {0,};

    ART_ASSERT(str);
    scan_cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    scan_cfg.channel = 0;
    scan_cfg.show_hidden = false;
    scan_cfg.scan_time.active = 30;
    scan_cfg.scan_time.passive = 120;

    if(at_station_parse_scan_string(str, &scan_cfg)){
        wifi_station_scan(&scan_cfg);
        at_printf(RET_OK_STR);
        return AT_OK;
    }

    at_printf(RET_FAIL_STR);
    return AT_ERROR;
}

char at_station_disconnect(void)
{
    wifi_config_t config = {0,};

    if(LINK_UP == ethernetif_get_link_state()){
        at_printf("wifi disconnect\r\n");
    }

    if(0 != wifi_set_config(STATION_IF, &config))
    {
        at_printf(RET_FAIL_STR);
    }
    else
    {
        at_printf(RET_OK_STR);
    }
    return AT_OK;

}

static bool at_softap_config_string_parse(char *str, wifi_ap_config_t *ap)
{
    char temp_str[3*BSSID_LEN] = {0,};
    char *p = NULL;
    int start = 0, end = 0;

    ART_ASSERT(str && ap);
    //ssid
    p = str + end;
    if(p < str + strlen(str)){
        start = (strchr(p, '\"') + 1) - str;
        p = str +start;
        end = strchr(p, '\"') - str;
        if(!substing(str, (char *)ap->ssid, start, end)){
            goto err;
        }
        ap->ssid_len = ((strlen(str) - start) > (end - start)) ? (end - start) : (strlen(str) - start);
    }

    //password
    p = str + end + 1;
    if(p < str + strlen(str)){
        start = (strchr(p, '\"') + 1) - str;
        p = str +start;
        end = strchr(p, '\"') - str;
        if(!substing(str, (char *)ap->password, start, end)){
            goto err;
        }
    }

    //channel
    p = str + end + 2;
    if(p < str + strlen(str)){
        start = p - str;
        p = str +start;
        if(strchr(p, ',')){
            end = strchr(p, ',') - str;
        }else{
            end = strlen(str);
        }
        if(!substing(str, temp_str, start, end)){
            goto err;
        }
        ap->channel = atoi(temp_str);
    }
    //ecn
    p = str + end + 1;
    if(p < str + strlen(str)){
        start = p - str;
        p = str +start;
        if(strchr(p, ',')){
            end = strchr(p, ',') - str;
        }else{
            end = strlen(str);
        }
        memset(temp_str, 0, 3*BSSID_LEN);
        if(!substing(str, temp_str, start, end)){
            goto err;
        }
        ap->authmode = (wifi_auth_mode_enum_t)atoi(temp_str);
    }
    //max conn
    p = str + end + 1;
    if(p < str + strlen(str)){
        start = p - str;
        p = str +start;
        if(strchr(p, ',')){
            end = strchr(p, ',') - str;
        }else{
            end = strlen(str);
        }
        memset(temp_str, 0, 3*BSSID_LEN);
        if(!substing(str, temp_str, start, end)){
            goto err;
        }
        ap->max_connection = atoi(temp_str);
    }
    //ssid hidden
    p = str + end + 1;
    if(p < str + strlen(str)){
        start = p - str;
        p = str +start;
        if(strchr(p, ',')){
            end = strchr(p, ',') - str;
        }else{
            end = strlen(str);
        }
        memset(temp_str, 0, 3*BSSID_LEN);
        if(!substing(str, temp_str, start, end)){
            goto err;
        }
        ap->ssid_hidden = atoi(temp_str);
    }
    return true;

err:
    return false;
}

char at_softap_set_config(char *str)
{
    init_param.wifi_mode = WIFI_MODE_AP;
    init_param.sta_ps_mode = WIFI_NO_POWERSAVE;
    init_param.dhcp_mode = WLAN_DHCP_SERVER;

    wifi_config_t wifi_config = {0,};

    ART_ASSERT(str);
    if(!at_softap_config_string_parse(str, &wifi_config.ap)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    if(0 != wifi_start(&init_param, true)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    if(0 != wifi_set_config(SOFT_AP_IF, &wifi_config)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    at_printf(RET_OK_STR);
    return AT_OK;
}

char at_softap_get_config(char *str)
{
    wifi_config_t wifi_config = {0,};
    wifi_ap_config_t *ap = &wifi_config.ap;

    if(0 != wifi_get_config(SOFT_AP_IF, &wifi_config)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    at_printf("\r\n+CWSAP: %s, %s, %d, %d, %d, %d\r\n", ap->ssid, ap->password, ap->channel, ap->authmode, ap->max_connection, ap->ssid_hidden);
    at_printf(RET_OK_STR);
    return AT_OK;
}
char at_softap_set_config_current(char *str)
{
    wifi_config_t wifi_config = {0,};

    ART_ASSERT(str);

    init_param.wifi_mode = WIFI_MODE_AP;
    init_param.sta_ps_mode = WIFI_NO_POWERSAVE;
    init_param.dhcp_mode = WLAN_DHCP_SERVER;

    if(!at_softap_config_string_parse(str, &wifi_config.ap)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    if(0 != wifi_start(&init_param, true)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    if(0 != wifi_set_config_current(SOFT_AP_IF, &wifi_config)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    at_printf(RET_OK_STR);
    return AT_OK;
}

char at_softap_get_config_current(char *str)
{
    wifi_config_t wifi_config = {0,};
    wifi_ap_config_t *ap = &wifi_config.ap;

    if(0 != wifi_get_config_current(SOFT_AP_IF, &wifi_config)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    at_printf("\r\n+CWSAP_CUR: %s, %s, %d, %d, %d, %d\r\n", ap->ssid, ap->password, ap->channel, ap->authmode, ap->max_connection, ap->ssid_hidden);
    at_printf(RET_OK_STR);
    return AT_OK;
}
char at_softap_set_config_def(char *str)
{
    return at_softap_set_config(str);
}

char at_softap_get_config_def(char *str)
{
    wifi_config_t wifi_config = {0,};
    wifi_ap_config_t *ap = &wifi_config.ap;

    wifi_get_config(SOFT_AP_IF, &wifi_config);
    at_printf("\r\n+CWSAP_DEF: %s, %s, %d, %d, %d, %d\r\n", ap->ssid, ap->password, ap->channel, ap->authmode, ap->max_connection, ap->ssid_hidden);
    at_printf(RET_OK_STR);
    return AT_OK;
}

char at_softap_get_station_list(char *str)
{

    at_printf("\r\nSorry, current not support!\r\n");
    at_printf(RET_OK_STR);
    return AT_OK;
}

//get sta ip
char at_get_sta_ip(char *str)//(esp8266)[AT+CIPSTA?]
{
    tcpip_ip_info_t ip_info = {0};

    ethernetif_get_ip_info(STATION_IF, &ip_info);

    at_printf("+CIPSTA: ip = %s \r\n", ip4addr_ntoa(&ip_info.ip));
    at_printf("+CIPSTA: netmask = %s \r\n", ip4addr_ntoa(&ip_info.netmask));
    at_printf("+CIPSTA: gateway = %s \r\n", ip4addr_ntoa(&ip_info.gw));
    at_printf(RET_OK_STR);
    return AT_OK;
}


char at_get_sta_ip_cur(char *str)//(esp8266)[AT+CIPSTA_CUR?]
{
    tcpip_ip_info_t ip_info = {0};

    ethernetif_get_ip_info(STATION_IF, &ip_info);

    at_printf("+CIPSTA_CUR: ip = %s \r\n", ip4addr_ntoa(&ip_info.ip));
    at_printf("+CIPSTA_CUR: netmask = %s \r\n", ip4addr_ntoa(&ip_info.netmask));
    at_printf("+CIPSTA_CUR: gateway = %s \r\n", ip4addr_ntoa(&ip_info.gw));

    at_printf(RET_OK_STR);
    return AT_OK;
}
char at_get_sta_ip_def(char *str)//!!!(esp8266)[AT+CIPSTA_DEF?]Why not take parameters from flash?
{
    tcpip_ip_info_t ip_info = {0};

    system_parameter_get_ip_config(STATION_IF, &ip_info);

    at_printf("+CIPSTA_DEF: ip = %s \r\n", ip4addr_ntoa(&ip_info.ip));
    at_printf("+CIPSTA_DEF: netmask = %s \r\n", ip4addr_ntoa(&ip_info.netmask));
    at_printf("+CIPSTA_DEF: gateway = %s \r\n", ip4addr_ntoa(&ip_info.gw));
    at_printf(RET_OK_STR);
    return AT_OK;
}

//set sta ip
char at_set_sta_ip_cur(char *str)
{
    int16_t len = 0;
    uint8_t i   = 0;
    char * pos_start = str;
    char value_ch[4] = {0,};

    uint32_t ip_addr = 0;
    uint32_t netmask = 0;
    uint32_t gateway = 0;
    uint32_t gw_min  = 0;
    uint32_t gw_max  = 0;
    uint32_t netmask_temp = 0;

    tcpip_ip_info_t ip_info;
    uint8_t addr0, addr1, addr2, addr3;

    //ip_addr[0]
    pos_start = strchr(pos_start,'\"');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr0 = (uint8_t)atoi(value_ch);
        if(addr0 > 255){
            goto error_end;
        }
    } else {
        goto error_end;
    }

    //ip_addr[1]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr1 = (uint8_t)atoi(value_ch);
        if(addr1 > 255){
            goto error_end;
        }
    } else {
        goto error_end;
    }

    //ip_addr[2]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr2 = (uint8_t)atoi(value_ch);
        if(addr2 > 255){
            goto error_end;
        }
    } else {
        goto error_end;
    }

    //ip_addr[3]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }
        addr3 = (uint8_t)atoi(value_ch);
        if(addr3 > 255){
            goto error_end;
        }
    } else {
        goto error_end;
    }
    IP_ADDR4(&ip_info.ip, addr0, addr1, addr2, addr3);

    //gateway[0]
    pos_start = strstr(pos_start,"\",\"");
    if(pos_start > 0) {
        pos_start += strlen("\",\"");
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto no_gateway_netmask;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr0 = (uint8_t)atoi(value_ch);
        if(addr0 > 255){
            goto no_gateway_netmask;
        }
    } else {
        goto no_gateway_netmask;
    }

    //gateway[1]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr1 = (uint8_t)atoi(value_ch);
        if(addr1 > 255){
            goto error_end;
        }
    } else {
        goto error_end;
    }

    //gateway[2]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr2 = (uint8_t)atoi(value_ch);
        if(addr2 > 255){
            goto error_end;
        }
    } else {
        goto error_end;
    }

    //gateway[3]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr3 = (uint8_t)atoi(value_ch);
        if(addr3 > 255){
            goto error_end;
        }
    } else {
        goto error_end;
    }
    IP_ADDR4(&ip_info.gw, addr0, addr1, addr2, addr3);

    //netmask[0]
    pos_start = strstr(pos_start,"\",\"");
    if(pos_start > 0) {
        pos_start += strlen("\",\"");
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto no_gateway_netmask;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr0 = (uint8_t)atoi(value_ch);
        if(addr0 > 255){
            goto no_gateway_netmask;
        }
    } else {
        goto no_gateway_netmask;
    }

    //netmask[1]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto no_gateway_netmask;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr1 = (uint8_t)atoi(value_ch);
        if(addr1 > 255){
            goto no_gateway_netmask;
        }
    } else {
        goto no_gateway_netmask;
    }

    //netmask[2]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto no_gateway_netmask;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        addr2 = (uint8_t)atoi(value_ch);
        if(addr2 > 255){
            goto no_gateway_netmask;
        }
    } else {
        goto no_gateway_netmask;
    }

    //netmask[3]
    pos_start = strchr(pos_start,'.');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789");
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len > 3)){
            goto no_gateway_netmask;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }
        addr3 = (uint8_t)atoi(value_ch);
        if(addr3 > 255){
            goto no_gateway_netmask;
        }
    } else {
        goto no_gateway_netmask;
    }
    IP_ADDR4(&ip_info.netmask, addr0, addr1, addr2, addr3);
    netmask = ip_info.netmask.addr;

    //1.Verify the netmask is valid
    netmask = ip_info.netmask.addr;

    netmask_temp = ~netmask + 1;
    if((netmask_temp & (netmask_temp - 1)) != 0) {
        goto no_gateway_netmask;//netmask invalid
    }

    //2.Calculate the gateway address range
    ip_addr = ip_info.ip.addr;

    gw_min = (ip_addr & netmask) + 1;
    gw_max = (ip_addr & netmask) + (~netmask - 1);

    //3.Verify the gateway is in gateway address range
    gateway = ip_info.gw.addr;

    if((gateway < gw_min) || (gateway > gw_max)){
        at_printf(RET_ERR_STR);
        at_printf("\r\nGateway netmask mismatch!\r\n");
        return AT_OK;
    }

    if(0 != ethernetif_set_ip_info(STATION_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;

no_gateway_netmask:
    //1.Set default netmask
    IP_ADDR4(&ip_info.netmask, 255, 255, 255, 0);
    //2.Set gateway based on the ip and netmask
    ip_addr = ip_info.ip.addr;

    gateway = (ip_addr & 0xFFFFFF00) + 1;

    ip_info.gw.addr = gateway;
    if(0 != ethernetif_set_ip_info(STATION_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    at_printf(RET_OK_STR);
    return AT_OK;
}

char at_set_sta_ip_def(char *str)
{
    int16_t len = 0;
    char * pos_start = str;
    char ip_str[18]  = {0,};

    uint32_t ip_addr = 0;
    uint32_t netmask = 0;
    uint32_t gateway = 0;
    uint32_t gw_min  = 0;
    uint32_t gw_max  = 0;
    uint32_t netmask_temp = 0;

    tcpip_ip_info_t ip_info;

    //ip_addr
    pos_start = strchr(pos_start,'\"');
    if(pos_start > 0) {
        pos_start += sizeof(char);

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto error_end;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.ip)) {
            goto error_end;//converted to addr failed!
        }

    } else {
        goto error_end;
    }

    //gw_addr
    pos_start = strstr(pos_start,"\",\"");
    if(pos_start > 0) {
        pos_start += strlen("\",\"");

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto no_gateway_netmask;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.gw)) {
            goto no_gateway_netmask;//converted to addr failed!
        }

    } else {
        goto no_gateway_netmask;
    }

    //netmask_addr
    pos_start = strstr(pos_start,"\",\"");
    if(pos_start > 0) {
        pos_start += strlen("\",\"");

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto no_gateway_netmask;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.netmask)) {
            goto no_gateway_netmask;//converted to addr failed!
        }

    } else {
        goto no_gateway_netmask;
    }

    //1.Verify the netmask is valid
    netmask  = ip_info.netmask.addr;

    netmask_temp = ~netmask + 1;
    if((netmask_temp & (netmask_temp - 1)) != 0) {
        goto no_gateway_netmask;//netmask invalid
    }

    //2.Calculate the gateway address range
    ip_addr  = ip_info.ip.addr;

    gw_min = (ip_addr & netmask) + 1;
    gw_max = (ip_addr & netmask) + (~netmask - 1);

    //3.Verify the gateway is in gateway address range
    gateway  = ip_info.gw.addr;

    if((gateway < gw_min) || (gateway > gw_max)){
        at_printf(RET_ERR_STR);
        at_printf("\r\nGateway netmask mismatch!\r\n");
        return AT_OK;
    }

    if(0 != ethernetif_set_ip_info(STATION_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    if(0 != system_parameter_set_ip_config(STATION_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;

no_gateway_netmask:
    //1.Set default netmask
    IP_ADDR4(&ip_info.netmask, 255, 255, 255, 0);

    //2.Set gateway based on the ip and netmask
    ip_addr  = ip_info.ip.addr;

    gateway = (ip_addr & 0xFFFFFF00) + 1;
    ip_info.gw.addr = gateway;

    if(0 != ethernetif_set_ip_info(STATION_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    if(0 != system_parameter_set_ip_config(STATION_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    at_printf(RET_OK_STR);
    return AT_OK;
}

//get ap ip
char at_get_ap_ip(char *str)//(esp8266)[AT+CIPAPA?]
{
    tcpip_ip_info_t ip_info = {0};

    ethernetif_get_ip_info(SOFT_AP_IF, &ip_info);

    at_printf("+CIPAP: ip = %s \r\n", ip4addr_ntoa(&ip_info.ip));
    at_printf("+CIPAP: netmask = %s \r\n", ip4addr_ntoa(&ip_info.netmask));
    at_printf("+CIPAP: gateway = %s \r\n", ip4addr_ntoa(&ip_info.gw));
    at_printf(RET_OK_STR);
    return AT_OK;
}
char at_get_ap_ip_cur(char *str)//(esp8266)[AT+CIPAP_CUR?]
{
    tcpip_ip_info_t ip_info = {0};

    ethernetif_get_ip_info(SOFT_AP_IF, &ip_info);

    at_printf("+CIPAP_CUR: ip = %s \r\n", ip4addr_ntoa(&ip_info.ip));
    at_printf("+CIPAP_CUR: netmask = %s \r\n", ip4addr_ntoa(&ip_info.netmask));
    at_printf("+CIPAP_CUR: gateway = %s \r\n", ip4addr_ntoa(&ip_info.gw));
    at_printf(RET_OK_STR);
    return AT_OK;
}
char at_get_ap_ip_def(char *str)//!!!(esp8266)[AT+CIPAP_DEF?]Why not take parameters from flash?
{
    tcpip_ip_info_t ip_info = {0};

    system_parameter_get_ip_config(SOFT_AP_IF, &ip_info);

    at_printf("+CIPAP_DEF: ip = %s \r\n", ip4addr_ntoa(&ip_info.ip));
    at_printf("+CIPAP_DEF: netmask = %s \r\n", ip4addr_ntoa(&ip_info.netmask));
    at_printf("+CIPAP_DEF: gateway = %s \r\n", ip4addr_ntoa(&ip_info.gw));
    at_printf(RET_OK_STR);
    return AT_OK;
}

//set ap ip
char at_set_ap_ip_cur(char *str)
{
    int16_t len = 0;
    char * pos_start = str;
    char ip_str[18]  = {0,};

    uint32_t ip_addr = 0;
    uint32_t netmask = 0;
    uint32_t gateway = 0;
    uint32_t gw_min  = 0;
    uint32_t gw_max  = 0;

    tcpip_ip_info_t ip_info;
    server_config_t dhcpd_config = {0,};

    //ip_addr
    pos_start = strchr(pos_start,'\"');
    if(pos_start > 0) {
        pos_start += sizeof(char);

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto error_end;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.ip)) {
            goto error_end;//converted to addr failed!
        }

    } else {
        goto error_end;
    }

    //gw_addr
    pos_start = strstr(pos_start,"\",\"");
    if(pos_start > 0) {
        pos_start += strlen("\",\"");

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto no_gateway_netmask;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.gw)) {
            goto no_gateway_netmask;//converted to addr failed!
        }

    } else {
        goto no_gateway_netmask;
    }

    //netmask_addr
    pos_start = strstr(pos_start,"\",\"");
    if(pos_start > 0) {
        pos_start += strlen("\",\"");

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto no_gateway_netmask;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.netmask)) {
            goto no_gateway_netmask;//converted to addr failed!
        }

    } else {
        goto no_gateway_netmask;
    }

    //1.Verify the netmask is valid
    netmask = ip_info.netmask.addr;

    if(((netmask + 1) & netmask) != 0) {
        goto no_gateway_netmask;//netmask invalid
    }

    //2.Calculate the gateway address range
    ip_addr = ip_info.ip.addr;

    gw_min = (ip_addr & netmask) + 1;
    gw_max = (ip_addr & netmask) + (~netmask - 1);

    //3.Verify the gateway is in gateway address range
    gateway = ip_info.gw.addr;

    if((gateway < gw_min) || (gateway > gw_max)){
        at_printf(RET_ERR_STR);
        at_printf("\r\nGateway netmask mismatch!\r\n");
        return AT_OK;
    }

    if(0 != ethernetif_set_ip_info(SOFT_AP_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    if(0 != system_parameter_set_ip_config(SOFT_AP_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    system_parameter_get_dhcpd_config(&dhcpd_config);
    memcpy(&(dhcpd_config.server), &(ip_info.ip), sizeof(ip_info.ip));
    system_parameter_set_dhcpd_config(&dhcpd_config);

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;

no_gateway_netmask:
    //1.Set default netmask
    IP_ADDR4(&ip_info.netmask, 255, 255, 255, 0);

    //2.Set gateway based on the ip and netmask
    ip_addr  = ip_info.ip.addr;

    gateway = (ip_addr & 0xFFFFFF00) + 1;
    ip_info.gw.addr = gateway;

    if(0 != ethernetif_set_ip_info(SOFT_AP_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    if(0 != system_parameter_set_ip_config(SOFT_AP_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    system_parameter_get_dhcpd_config(&dhcpd_config);
    memcpy(&(dhcpd_config.server), &(ip_info.ip), sizeof(ip_info.ip));
    system_parameter_set_dhcpd_config(&dhcpd_config);

    at_printf(RET_OK_STR);
    return AT_OK;
}

char at_set_ap_ip_def(char *str)
{
    int16_t len = 0;
    char * pos_start = str;
    char ip_str[18]  = {0,};

    uint32_t ip_addr = 0;
    uint32_t netmask = 0;
    uint32_t gateway = 0;
    uint32_t gw_min  = 0;
    uint32_t gw_max  = 0;

    tcpip_ip_info_t ip_info;
    server_config_t dhcpd_config = {0,};

    //ip_addr
    pos_start = strchr(pos_start,'\"');
    if(pos_start > 0) {
        pos_start += sizeof(char);

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto error_end;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.ip)) {
            goto error_end;//converted to addr failed!
        }

    } else {
        goto error_end;
    }

    //gw_addr
    pos_start = strstr(pos_start,"\",\"");
    if(pos_start > 0) {
        pos_start += strlen("\",\"");

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto no_gateway_netmask;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.gw)) {
            goto no_gateway_netmask;//converted to addr failed!
        }

    } else {
        goto no_gateway_netmask;
    }

    //netmask_addr
    pos_start = strstr(pos_start,"\",\"");
    if(pos_start > 0) {
        pos_start += strlen("\",\"");

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (strlen("255.255.255.255")))){
            goto no_gateway_netmask;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        if(! ip4addr_aton(ip_str, &ip_info.netmask)) {
            goto no_gateway_netmask;//converted to addr failed!
        }

    } else {
        goto no_gateway_netmask;
    }

    //1.Verify the netmask is valid
    netmask = ip4_addr_get_u32(&ip_info.netmask);

    if(((netmask + 1) & netmask) != 0) {
        goto no_gateway_netmask;//netmask invalid
    }

    //2.Calculate the gateway address range
    ip_addr = ip4_addr_get_u32(&ip_info.ip);

    gw_min = (ip_addr & netmask) + 1;
    gw_max = (ip_addr & netmask) + (~netmask - 1);

    //3.Verify the gateway is in gateway address range
    gateway = ip4_addr_get_u32(&ip_info.gw);

    if((gateway < gw_min) || (gateway > gw_max)){
        at_printf(RET_ERR_STR);
        at_printf("\r\nGateway netmask mismatch!\r\n");
        return AT_OK;
    }

    if(0 != ethernetif_set_ip_info(SOFT_AP_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    if(0 != system_parameter_set_ip_config(SOFT_AP_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    system_parameter_get_dhcpd_config(&dhcpd_config);
    memcpy(&(dhcpd_config.server), &(ip_info.ip), sizeof(ip_info.ip));
    system_parameter_set_dhcpd_config(&dhcpd_config);

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;

no_gateway_netmask:
    //1.Set default netmask
    IP_ADDR4(&ip_info.netmask, 255, 255, 255, 0);

    //2.Set gateway based on the ip and netmask
    ip_addr = ip4_addr_get_u32(&ip_info.ip);

    gateway = (ip_addr & 0xFFFFFF00) + 1;

    ip4_addr_set_u32(&ip_info.gw, gateway);

    if(0 != ethernetif_set_ip_info(SOFT_AP_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }
    if(0 != system_parameter_set_ip_config(SOFT_AP_IF, &ip_info)){
        at_printf(RET_FAIL_STR);
        return AT_ERROR;
    }

    system_parameter_get_dhcpd_config(&dhcpd_config);
    memcpy(&(dhcpd_config.server), &(ip_info.ip), sizeof(ip_info.ip));
    system_parameter_set_dhcpd_config(&dhcpd_config);

    at_printf(RET_OK_STR);
    return AT_OK;
}

//set dhcp
//0,1
char at_set_dhcp_cur(char * str)
{
    uint8_t mode = 0, en = 0;
    server_config_t dhcpd_config = {0};

    if ((!isdigit(str[0])) && ('.' != str[1]) && (!isdigit(str[2]))) {
        goto error_end;
    }

    mode = str[0] - '0';
    en   = str[2] - '0';

    if(en > 1){
        goto error_end;
    }

    if(mode == 0){//SoftAP
        if(en == 1){
            ethernetif_set_dhcp_mode(SOFT_AP_IF, WLAN_DHCP_SERVER);
            dhcpd_curr_config_get(&dhcpd_config);
            dhcpd_curr_config_set(&dhcpd_config);
        }else{
            ethernetif_set_dhcp_mode(SOFT_AP_IF, WLAN_STATIC_IP);
        }
    } else if(mode == 1){//Station
        if(en == 1){
            ethernetif_set_dhcp_mode(STATION_IF, WLAN_DHCP_CLIENT);
        }else{
            ethernetif_set_dhcp_mode(STATION_IF, WLAN_STATIC_IP);
        }
    } else {
        goto error_end;//error mode(SoftAP + Station not support)
    }

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;
}

char at_set_dhcp_def(char * str)
{
    uint8_t mode = 0, en = 0;
    server_config_t dhcpd_config = {0};

    if ((!isdigit(str[0])) && ('.' != str[1]) && (!isdigit(str[2]))) {
        goto error_end;
    }

    mode = str[0] - '0';
    en   = str[2] - '0';

    if(en > 1){
        goto error_end;//softAP mode(We don't support)
    }

    if(mode == 0){//SoftAP
        if(en == 1){//In SoftAP, must enable DHCP_SERVER
            ethernetif_set_dhcp_mode(SOFT_AP_IF, WLAN_DHCP_SERVER);

            system_parameter_get_dhcpd_config(&dhcpd_config);
            dhcpd_curr_config_set(&dhcpd_config);
        }else{
            ethernetif_set_dhcp_mode(SOFT_AP_IF, WLAN_STATIC_IP);
            if(dhcpd_is_running()){
                dhcpd_stop();
            }
        }
    } else if(mode == 1){//Station
        if(en == 1){
            ethernetif_set_dhcp_mode(STATION_IF, WLAN_DHCP_CLIENT);
        }else{
            ethernetif_set_dhcp_mode(STATION_IF, WLAN_STATIC_IP);
        }
    } else {
        goto error_end;//error mode(SoftAP + Station not support)
    }

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;
}

//get dhcp
char at_get_dhcp_cur(char * str)
{
    uint8_t en = 0;
    dhcp_mode_enum_t dhcp_mode;

    //STA
    dhcp_mode = ethernetif_get_dhcp_mode(STATION_IF);
    if(dhcp_mode == WLAN_DHCP_CLIENT){
        LN_SET_BIT(en, 0);
    }

    //AP
    dhcp_mode = ethernetif_get_dhcp_mode(SOFT_AP_IF);
    if(dhcp_mode == WLAN_DHCP_SERVER){
        LN_SET_BIT(en, 1);
    }

    at_printf("+CWDHCP_CUR:%d\r\n", en);
    at_printf(RET_OK_STR);
    return AT_OK;
}
char at_get_dhcp_def(char * str)
{
    uint8_t en = 0;
    dhcp_mode_enum_t dhcp_mode;

    //STA
    dhcp_mode = ethernetif_get_dhcp_mode(STATION_IF);
    if(dhcp_mode == WLAN_DHCP_CLIENT){
        LN_SET_BIT(en, 0);
    }

    //AP
    dhcp_mode = ethernetif_get_dhcp_mode(SOFT_AP_IF);
    if(dhcp_mode == WLAN_DHCP_SERVER){
        LN_SET_BIT(en, 1);
    }
    at_printf("+CWDHCP_DEF:%d\r\n", en);
    at_printf(RET_OK_STR);
    return AT_OK;
}

static bool at_softap_parse_dhcps_string(char *dhcps_str, int *enable, int *lease, unsigned int *ip_start, unsigned int *ip_end)
{
    char temp_str[16] = {0,};
    char *domain_begin = NULL, *domain_end = NULL, *domain_separator_addr = NULL, *limit = dhcps_str + strlen(dhcps_str);
    char domain_separator = ',', content_flag = '\"';

    ART_ASSERT(dhcps_str);

    /*
     * string format: <enable>[,<lease time>,<start IP>,<end IP>]
     *
     */
    //enable
    domain_begin = dhcps_str;
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, NULL, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        *enable = atoi(temp_str);
    }

    //lease time
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto partial_config;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, 16);
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, NULL, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        *lease = atoi(temp_str);
    }

    //start IP
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto err;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, 16);
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, &content_flag, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        *ip_start = inet_addr(temp_str);
    }

    //end IP
    domain_begin = domain_separator_addr + 1;
    if(domain_begin > limit){
        goto err;
    }
    domain_separator_addr = strchr(domain_begin, domain_separator);
    if(!domain_separator_addr){
        domain_separator_addr = limit;
    }else if(domain_separator_addr && domain_separator_addr > limit){
        goto err;
    }
    domain_end = domain_separator_addr;
    memset(temp_str, 0, 16);
    if(!art_string_extract_domain_content(domain_begin, domain_end, domain_separator, &content_flag, temp_str)){
        goto err;
    }
    if(strlen(temp_str) > 0){
        *ip_end = inet_addr(temp_str);
    }

partial_config:
    return true;

err:
    return false;
}

char at_set_dhcps_cur(char * str)
{
    server_config_t dhcpd_config = {0,};
    int enable, lease_time, client_max;
    unsigned int ip_start, ip_end;

    if(!at_softap_parse_dhcps_string(str, &enable, &lease_time, &ip_start, &ip_end)){
        at_printf(RET_ERR_STR);
        return AT_ERROR;
    }

    if(enable == 0){
        system_parameter_get_dhcpd_config_default(&dhcpd_config);
        dhcpd_curr_config_set(&dhcpd_config);
        at_printf(RET_OK_STR);
        return AT_OK;
    }else if(enable == 1){
        if(ip_start&0xFFFFFF != ip_end&0xFFFFFF){
            at_printf(RET_ERR_STR);
            return AT_ERROR;
        }
        dhcpd_curr_config_get(&dhcpd_config);
        dhcpd_config.lease = lease_time;
        client_max = MIN(dhcpd_config.client_max, (ip_end>>24) - (ip_start>>24) + 1);
        dhcpd_config.client_max = client_max;
        ip4_addr_set_u32(&(dhcpd_config.ip_start), ip_start);
        ip4_addr_set_u32(&(dhcpd_config.ip_end), ip_end);
        ip4_addr_set_u32(&(dhcpd_config.server), (ip_start&0xFFFFFF)|(1<<24));
        dhcpd_curr_config_set(&dhcpd_config);
        at_printf(RET_OK_STR);
        return AT_OK;
    }

    at_printf(RET_ERR_STR);
    return AT_ERROR;
}

char at_set_dhcps_def(char * str)
{
    server_config_t dhcpd_config = {0,};
    int enable, lease_time, client_max;
    unsigned int ip_start, ip_end;

    if(!at_softap_parse_dhcps_string(str, &enable, &lease_time, &ip_start, &ip_end)){
        at_printf(RET_ERR_STR);
        return AT_ERROR;
    }

    if(enable == 0){
        system_parameter_get_dhcpd_config_default(&dhcpd_config);
        system_parameter_set_dhcpd_config(&dhcpd_config);
        dhcpd_curr_config_set(&dhcpd_config);
        at_printf(RET_OK_STR);
        return AT_OK;
    }else if(enable == 1){
        system_parameter_get_dhcpd_config(&dhcpd_config);
        dhcpd_config.lease = lease_time;
        client_max = MIN(dhcpd_config.client_max, (ip_end - ip_start + 1));
        dhcpd_config.client_max = client_max;
        ip4_addr_set_u32(&(dhcpd_config.ip_start), ip_start);
        ip4_addr_set_u32(&(dhcpd_config.ip_end), ip_end);
        system_parameter_set_dhcpd_config(&dhcpd_config);
        at_printf(RET_OK_STR);
        return AT_OK;
    }

    at_printf(RET_ERR_STR);
    return AT_ERROR;
}

//get dhcps
//+CWDHCPS_CUR:120,"192.168.4.2","192.168.4.101"
char at_get_dhcps_cur(char * str)
{
    server_config_t dhcpd_config = {0,};

    dhcpd_curr_config_get(&dhcpd_config);
    at_printf("+CWDHCPS_CUR:%d,\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"\r\n", dhcpd_config.lease,
                                                            ip4_addr1(&dhcpd_config.ip_start), ip4_addr2(&dhcpd_config.ip_start), ip4_addr3(&dhcpd_config.ip_start), ip4_addr4(&dhcpd_config.ip_start),
                                                            ip4_addr1(&dhcpd_config.ip_end), ip4_addr2(&dhcpd_config.ip_end), ip4_addr3(&dhcpd_config.ip_end), ip4_addr4(&dhcpd_config.ip_end));

    at_printf(RET_OK_STR);
    return AT_OK;
}

char at_get_dhcps_def(char * str)
{
    server_config_t dhcpd_config = {0,};

    system_parameter_get_dhcpd_config(&dhcpd_config);
    at_printf("+CWDHCPS_DEF:%d,\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"\r\n", dhcpd_config.lease,
                                                            ip4_addr1(&dhcpd_config.ip_start), ip4_addr2(&dhcpd_config.ip_start), ip4_addr3(&dhcpd_config.ip_start), ip4_addr4(&dhcpd_config.ip_start),
                                                            ip4_addr1(&dhcpd_config.ip_end), ip4_addr2(&dhcpd_config.ip_end), ip4_addr3(&dhcpd_config.ip_end), ip4_addr4(&dhcpd_config.ip_end));

    at_printf(RET_OK_STR);
    return AT_OK;
}

//ping test
#if (TCP_TEST_DATA_FROM_CONSOLE==1)
ping_param_t peer_ip_addr;
#endif
char at_ping(char * str)
{
    int16_t len = 0;
    char * pos_start = str;
    char ip_str[64]  = {0,};

    ART_ASSERT(str);
    //ip_addr
    pos_start = strchr(pos_start,'\"');
    if(pos_start > 0) {
        pos_start += sizeof(char);

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (sizeof(ip_str)-1))){
            goto error_end;
        }

        memset(ip_str, '\0', sizeof(ip_str));
        memcpy(ip_str, pos_start, len);

        ping_param_t ping_param;
        ping_param.host = ip_str;
#if (TCP_TEST_DATA_FROM_CONSOLE==1)
        peer_ip_addr.host = ip_str;
#endif
        ping_param.cnt  = 1;
        ping_param.interval_ms = 1000;

        if(! create_ping_task(&ping_param)){
            at_printf("busy p...\r\n");
            return AT_OK;
        };
        return AT_OK;

    } else {
        goto error_end;
    }


error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;
}

char at_get_cpu_usage(char * str)
{
    at_printf("\r\n+CPUUSG: %d%\r\n", osGetCPUUsage());
    at_printf(RET_OK_STR);

    return AT_OK;
}

//chip sn
char at_get_chip_sn(char * str)//AT+CSN?  +CSN:"01234567890123456789"
{
    char sn_str[NV4_CHIP_SN_LEN+1]  = {0,};

    memset(sn_str, '\0', sizeof(sn_str));
    ln_nvds_get_chip_sn((uint8_t *)sn_str, NV4_CHIP_SN_LEN);

    at_printf("\r\n+CSN:\"%s\"\r\n", sn_str);
    at_printf(RET_OK_STR);
    return AT_OK;
}

char at_set_chip_sn(char * str)//AT+CSN="01234567890123456789"
{
    int16_t len = 0;
    char * pos_start = str;
    char sn_str[NV4_CHIP_SN_LEN+1]  = {0,};

    ART_ASSERT(str);

    pos_start = strchr(pos_start,'\"');
    if(pos_start > 0) {
        pos_start += sizeof(char);

        len = strcspn(pos_start, "\"");
        if ((len < 1) || (len > (sizeof(sn_str)-1))){
            goto error_end;
        }

        memset(sn_str, '\0', sizeof(sn_str));
        memcpy(sn_str, pos_start, len);

        ln_nvds_set_chip_sn((uint8_t *)sn_str);
    } else {
        goto error_end;
    }

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;
}

//CEFUSE
char at_get_efuse(char * str) //AT+CEFUSE? +CEFUSE:value0,value1,value2,value3,value4,value5,value6,value7
{
    uint32_t efuse_val[8] = {0,};

    for(int i = 0; i < 8; i++){
        efuse_val[i] = LL_EFUSE_ReadCorrectReg(EFUSE, i);
    }

    at_printf("\r\n+CEFUSE:0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x\r\n", \
                efuse_val[0],efuse_val[1],efuse_val[2],efuse_val[3], \
                efuse_val[4],efuse_val[5],efuse_val[6],efuse_val[7]);

    at_printf(RET_OK_STR);
    return AT_OK;
};

char at_set_efuse(char * str) //AT+CEFUSE=0,0x12345678
{
    int16_t len = 0, i = 0;
    char * pos_start = str;
    char value_ch[9];
    uint32_t index = 0,data = 0;

    ART_ASSERT(str);

    //index
    pos_start = strpbrk(pos_start, "0123456789");
    if(pos_start > 0) {
        len = strspn(pos_start, "0123456789");

        if ((len < 1) || (len >= sizeof(value_ch))){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        index = (uint8_t)atoi(value_ch);
        if(index > 7){
            goto error_end;
        }
    } else {
        goto error_end;
    }

    //val
    pos_start = strchr(pos_start,'x');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789abcdefABCDEF");
        len = strspn(pos_start, "0123456789abcdefABCDEF");

        if ((len < 1) || (len >= sizeof(value_ch))){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ; i++){
            value_ch[i]   = *(pos_start + i) ;
            if(!isxdigit(value_ch[i])){
                goto error_end;
            }
            data += ln_char2hex(value_ch[i]) << (((len - 1) - i)*4);
        }

    } else {
        goto error_end;
    }

    HAL_EFUSE_WriteEnable(EFUSE);
    HAL_EFUSE_WriteShadowReg(EFUSE, (EFUSE_Reg_Index_t)index, data);

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;
};

//XTAL_COMP
char at_get_xtal_cap_comp(char * str)//AT+XTAL_COMP?; +XTAL_COMP:0
{
    int8_t cmop = 0;

    if (NVDS_ERR_OK == ln_nvds_get_xtal_comp_val((uint8_t *)&cmop)) {
        if ((uint8_t)cmop == 0xFF) {
            cmop = 0;
        }
    }

    at_printf("\r\n+XTAL_COMP:%d\r\n", cmop);
    at_printf(RET_OK_STR);
    return AT_OK;
};

char at_set_xtal_cap_comp(char * str)//AT+XTAL_COMP=0 //set val rang:-20 ~ +10
{
    int16_t len = 0, i = 0;
    char * pos_start = str;
    char value_ch[4];
    int8_t val = 0;

    ART_ASSERT(str);

    //val
    pos_start = strpbrk(pos_start, "-0123456789");
    if(pos_start > 0) {
        len = strspn(pos_start, "-0123456789");

        if ((len < 1) || (len >= 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        val = (int8_t)atoi(value_ch);
        if((val > 20) || (val < -8)){
            goto error_end;
        }
    } else {
        goto error_end;
    }

    ln_nvds_set_xtal_comp_val((uint8_t)val);

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;
};

//TXPOWER_COMP
char at_get_tx_power_comp(char * str)//AT+TXPOWER_COMP?; +TXPOWER_COMP:0
{
    int8_t cmop = 0;
    
    if (NVDS_ERR_OK == ln_nvds_get_tx_power_comp((uint8_t *)&cmop)) {
        if ((uint8_t)cmop == 0xFF) {
            cmop = 0;
        }
    }

    at_printf("\r\n+TXPOWER_COMP:%d\r\n", cmop);
    at_printf(RET_OK_STR);
    return AT_OK;
};

char at_set_tx_power_comp(char * str)//AT+TXPOWER_COMP=0 //set val rang:-16 ~ +8
{
    int16_t len = 0, i = 0;
    char * pos_start = str;
    char value_ch[4];
    int8_t val = 0;

    ART_ASSERT(str);

    //val
    pos_start = strpbrk(pos_start, "-0123456789");
    if(pos_start > 0) {
        len = strspn(pos_start, "-0123456789");

        if ((len < 1) || (len >= 3)){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;){
            value_ch[i]   = *(pos_start + i) ;
            value_ch[++i] = '\0';
        }

        val = (int8_t)atoi(value_ch);
        if((val > 8) || (val < -16)){
            goto error_end;
        }
    } else {
        goto error_end;
    }

    ln_nvds_set_tx_power_comp((uint8_t)val);

    at_printf(RET_OK_STR);
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;
};



char at_set_station_auto_connect(char *str)
{
    at_printf("[%s, %d]\r\n", __func__, __LINE__);
    return AT_OK;
}

char at_get_station_auto_connect(char *str)
{
    at_printf("[%s, %d]\r\n", __func__, __LINE__);
    return AT_OK;
}


char at_set_sta_host_name(char *str)
{
    at_printf("[%s, %d]\r\n", __func__, __LINE__);
    return AT_OK;
}


char at_get_sta_host_name(char *str)
{
    at_printf("[%s, %d]\r\n", __func__, __LINE__);
    return AT_OK;
}

char at_iperf(char *str)
{
    if((str != NULL) && (0 == iperf(str))){
        at_printf(RET_OK_STR);
        return AT_OK;
    }
    at_printf(RET_ERR_STR);
    return AT_ERROR;
}

char at_get_heap_size(char *str)
{
    int32_t heap_size=0;
    heap_size=OS_GetFreeHeapSize();
    at_printf("left heap size=%d\r\n",heap_size);
    at_printf("OK\r\n");
    return AT_OK;
}


char at_rf_test_destory(char *str)//AT+RF_TEST_DESTORY=0x00104000
{
    int16_t len = 0, i = 0;
    char * pos_start = str;
    char value_ch[9];
    uint32_t offset = 0;

    ART_ASSERT(str);

    //offset
    pos_start = strchr(pos_start,'x');
    if(pos_start > 0) {
        pos_start += sizeof(char);
        pos_start = strpbrk(pos_start, "0123456789abcdefABCDEF");
        len = strspn(pos_start, "0123456789abcdefABCDEF");

        if ((len < 1) || (len >= sizeof(value_ch))){
            goto error_end;
        }

        memset(value_ch, '\0', sizeof(value_ch));
        for (i = 0; i < len ;i++){
            value_ch[i]   = *(pos_start + i) ;
            if(!isxdigit(value_ch[i])){
                goto error_end;
            }

            offset += ln_char2hex(value_ch[i]) << (((len - 1) - i)*4);
        }

    } else {
        goto error_end;
    }

    at_printf(RET_OK_STR);
    OS_MsDelay(60); //make sure the "ret ok" goes out before it self-destory!

    FLASH_Erase(offset, 4*1024);//rf test self-destory!
    return AT_OK;

error_end:
    at_printf(RET_ERR_STR);
    return AT_OK;
}
/******************************************************
function description: to get SDK version of wifi lib
*******************************************************/
char at_software_version(void)
{
    at_printf("sw version:%s\r\n", wifi_lib_version_string_get());
    at_printf(RET_OK_STR);
    return AT_OK;
}

#if WIFI_SWITCH
uint8_t wifi_en=1;
extern void wifi_track_init(void);
extern void wifi_track_reinit(void);
extern void wifi_track_deinit(void);
extern bool wifi_manager_init(void);
extern bool wifi_manager_deinit(void);
char at_wifi_switch(char *str)
{
    uint8_t  en;
    en=atoi(str);
    if(en&&wifi_en==0){
        wifi_en=1;
        //wifi_init();
        //wifi_manager_init();
        wifi_track_reinit();
    }else if(0==en&&wifi_en){
        wifi_en=0;
        wifi_track_deinit();//call wifi_stop()
        //wifi_manager_deinit();
        //wifi_deinit();
    }

    at_printf("OK\r\n");

return AT_OK;
}
#endif

