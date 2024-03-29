#include "osal/osal.h"
#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"
#include "utils/art_string.h"
#include "wifi/wifi.h"
#include "netif/ethernetif.h"
#include "wifi_manager.h"
#include "lwip/tcpip.h"
#include "hal/hal_sleep.h"
#include "drv_adc_measure.h"
#include "utils/system_parameter.h"
#include "hal/hal_adc.h"
#include "tcp_server_echo.h"
#include "ln_kv_api.h"
#include "ln_nvds.h"

static OS_Thread_t g_usr_app_thread;
#define USR_APP_TASK_STACK_SIZE      (6*256) //Byte

#define WIFI_TEMP_CALIBRATE          (1)
#if WIFI_TEMP_CALIBRATE
  static OS_Thread_t g_temp_cal_thread;
  #define TEMP_APP_TASK_STACK_SIZE   (4*256) //Byte
#endif


void wifi_init_sta(void)
{
    uint8_t macaddr[6] = {0}, macaddr_default[6] = {0};
    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = "TP-LINK_479A64",
            .password = "12345678",
            0,
        },
    };
    wifi_config_t temp_config = {0};
    wifi_init_type_t init_param = {
        .wifi_mode = WIFI_MODE_STATION,
        .sta_ps_mode = WIFI_NO_POWERSAVE,//WIFI_MAX_POWERSAVE,
        .dhcp_mode = WLAN_DHCP_CLIENT,
        //In station mode, define the length of the AP list scanned.
        .scanned_ap_list_size = SCANNED_AP_LIST_SIZE,
    };

    //Set wifi mode
    wifi_set_mode(init_param.wifi_mode);

    //Check mac address
    system_parameter_get_wifi_macaddr_default(STATION_IF, macaddr_default);
    wifi_get_macaddr(STATION_IF, macaddr);
    if (ln_is_valid_mac((const char *)macaddr) && (memcmp(macaddr, macaddr_default, 6) != 0)) {
        //If there is a valid MAC in flash, use it
        wifi_set_macaddr_current(STATION_IF, macaddr);
    } else {
        //generate random macaddr
        ln_generate_random_mac(macaddr);
        wifi_set_macaddr(STATION_IF, macaddr);
    }

    //Check config
    wifi_get_config(STATION_IF, &temp_config);
    if (strlen((const char *)temp_config.sta.ssid) > 0) {
        //If there is a valid config in flash, use it;
        wifi_set_config_current(STATION_IF, &temp_config);
    } else {
        //else, use the prev wifi_config and save it to flash.
        wifi_set_config(STATION_IF, &wifi_config);
    }

    //Startup WiFi.
    if(0 != wifi_start(&init_param, true)){
        LOG(LOG_LVL_ERROR, "[%s, %d]wifi_start() fail.\r\n", __func__, __LINE__);
    }
}

void wifi_init_ap(void)
{
    uint8_t macaddr[6] = {0}, macaddr_default[6] = {0};
    wifi_config_t wifi_config = {
        .ap = {
            .ssid            = "ln_wifi",
            .ssid_len        = strlen("ln_wifi"),
            .password        = "12345678",
            .channel         = 6,
            .authmode        = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden     = 0,
            .max_connection  = 4,
            .beacon_interval = 100,
            .reserved        = 0,
        },
    };
    wifi_config_t temp_config = {0}, config_default = {0};
    wifi_init_type_t init_param = {
        .wifi_mode = WIFI_MODE_AP,
        .sta_ps_mode = WIFI_NO_POWERSAVE,
        .dhcp_mode = WLAN_DHCP_SERVER,
    };

    //Set wifi mode
    wifi_set_mode(init_param.wifi_mode);

    //Check mac address
    system_parameter_get_wifi_macaddr_default(SOFT_AP_IF, macaddr_default);
    wifi_get_macaddr(SOFT_AP_IF, macaddr);
    if (ln_is_valid_mac((const char *)macaddr) && (memcmp(macaddr, macaddr_default, 6) != 0)) {
        //If there is a valid MAC in flash, use it
        wifi_set_macaddr_current(SOFT_AP_IF, macaddr);
    } else {
        //generate random macaddr
        ln_generate_random_mac(macaddr);
        wifi_set_macaddr(SOFT_AP_IF, macaddr);
    }

    //Check config
    wifi_get_config(SOFT_AP_IF, &temp_config);
    system_parameter_get_wifi_config_default (SOFT_AP_IF, &config_default);
    if ((memcmp(&temp_config, &config_default, sizeof(wifi_config_t)) != 0) && (strlen((const char *)temp_config.ap.ssid) > 0)) {
        //If there is a valid config in flash, use it;
        wifi_set_config_current(SOFT_AP_IF, &temp_config);
    } else {
        //else, use the prev wifi_config and save it to flash.
        wifi_set_config(SOFT_AP_IF, &wifi_config);
    }

    //Startup WiFi.
    if(0 != wifi_start(&init_param, true)){//WIFI_MAX_POWERSAVE
        LOG(LOG_LVL_ERROR, "[%s, %d]wifi_start() fail.\r\n", __func__, __LINE__);
    }
}

#include "ln_types.h"
#include "lwip/netdb.h"
#include "lwip/ip_addr.h"

static int get_host_by_name(char *name, uint32_t * addr)
{
	struct hostent * host_entry;

    host_entry = gethostbyname(name);
    if(host_entry) {
        *addr = ntohl(((struct in_addr *)(host_entry->h_addr_list[0]))->s_addr);
        return LN_TRUE;
    } else {
        return LN_FALSE;
    }
}

void usr_app_task_entry(void *params)
{
    ip_addr_t host_ip;
    err_t ret;

    //Set sleep mode
    hal_sleep_set_mode(ACTIVE);

   wifi_init_sta();//wifi_init_ap();

   while(LINK_UP != ethernetif_get_link_state()) {
       OS_MsDelay(200);
   }

   tcp_server_echo_task_creat(8087);

    while(1)
    {
//        struct ip4_addr ip4addr;
//
//        if (LN_TRUE == get_host_by_name("www.baidu.com", &ip4addr.addr)) {
//            LOG(LOG_LVL_ERROR, "www.baidu.com => %s\r\n", ip_ntoa(&ip4addr));
//        } else {
//            LOG(LOG_LVL_ERROR, "get baidu host name failed.\r\n");
//        }

//        if (LN_TRUE == get_host_by_name("www.blog.csdn.net", &ip4addr.addr)) {
//            LOG(LOG_LVL_ERROR, "www.blog.csdn.net => %s\r\n", ip_ntoa(&ip4addr));
//        } else {
//            LOG(LOG_LVL_ERROR, "get csdn host name failed.\r\n");
//        }


        OS_MsDelay(2000);
    }
}

void temp_cal_app_task_entry(void *params)
{
    int8_t cap_comp = 0;

    if (NVDS_ERR_OK == ln_nvds_get_xtal_comp_val((uint8_t *)&cap_comp)) {
        if ((uint8_t)cap_comp == 0xFF) {
            cap_comp = 0;
        }
    }

    drv_adc_init();
    OS_MsDelay(1);
    wifi_temp_cal_init(drv_adc_read(INTL_ADC_CHAN_0), cap_comp);

    while (1)
    {
        OS_MsDelay(1000);
        wifi_do_temp_cal_period(drv_adc_read(INTL_ADC_CHAN_0));
    }
}

void creat_usr_app_task(void)
{
    if(OS_OK != OS_ThreadCreate(&g_usr_app_thread, "UsrAPP", usr_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, USR_APP_TASK_STACK_SIZE)) {
        ART_ASSERT(1);
    }
#if  WIFI_TEMP_CALIBRATE
    if(OS_OK != OS_ThreadCreate(&g_temp_cal_thread, "TempAPP", temp_cal_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, TEMP_APP_TASK_STACK_SIZE)) {
        ART_ASSERT(1);
    }
#endif
}



