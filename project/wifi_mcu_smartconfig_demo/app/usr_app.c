#include <string.h>
#include "usr_app.h"
#include "osal/osal.h"
#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"
#include "utils/art_string.h"
#include "wifi/wifi.h"
#include "ping.h"
#include "netif/ethernetif.h"
#include "wifi_manager.h"
#include "lwip/tcpip.h"
#include "hal/hal_sleep.h"
#include "drv_adc_measure.h"
#include "utils/system_parameter.h"
#include "hal/hal_adc.h"
#include "ln_smartcfg_api.h"
#include "ln_kv_api.h"
#include "ln_nvds.h"


static OS_Thread_t g_usr_app_thread;
#define USR_APP_TASK_STACK_SIZE    (6*256) //Byte

#define WIFI_TEMP_CALIBRATE        (1)
#if WIFI_TEMP_CALIBRATE
  static OS_Thread_t g_temp_cal_thread;
  #define TEMP_APP_TASK_STACK_SIZE   (4*256) //Byte
#endif


void wifi_init_sta(void)
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
    if(0 != wifi_start(&init_param, true)){
        LOG(LOG_LVL_ERROR, "[%s, %d]wifi_start() fail.\r\n", __func__, __LINE__);
    }
}

void connect_to_ap(uint8_t * ssid, uint8_t * pwd)
{
    wifi_config_t wifi_config = {0,};

    memset(&wifi_config, 0, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid,ssid, strlen((const char *)ssid));
    memcpy(wifi_config.sta.password,pwd, strlen((const char *)pwd));

    wifi_station_connect(&wifi_config);
}


void usr_app_task_entry(void *params)
{
    uint16_t channel_map = 0;
    uint8_t * ssid = NULL;
    uint8_t * pwd = NULL;

    wifi_init_sta();

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
    connect_to_ap(ssid, pwd);

    //Wait for network link up
    while(LINK_UP != ethernetif_get_link_state()){
        OS_MsDelay(300);
    }

    ln_smartconfig_send_ack();

    while(1)
    {
        OS_MsDelay(200);
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



