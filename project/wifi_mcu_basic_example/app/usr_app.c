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
#include "ln_kv_api.h"
#include "ln_nvds.h"

#include "SEGGER_SYSVIEW.h"

static OS_Thread_t g_usr_app_thread;
#define USR_APP_TASK_STACK_SIZE   6*256 //Byte

#define WIFI_TEMP_CALIBRATE             1

#if WIFI_TEMP_CALIBRATE
static OS_Thread_t g_temp_cal_thread;
#define TEMP_APP_TASK_STACK_SIZE   4*256 //Byte
#endif

typedef struct {
    OS_Thread_t sleep_thr;
    //OS_Semaphore_t kill_signal;
    uint32_t  enter_sleep_timeout;//in ms
    uint32_t  exit_sleep_timeout;//in ms seep duration=exit_sleep_timeout-enter_sleep_timeout
} sleep_param_t;

sleep_param_t g_sleep_timeout={
    .enter_sleep_timeout=30*1000,
    .exit_sleep_timeout=50*1000,// will exit sleep after sleep for approximately 20s
    };
#define USR_SLEEP_TASK_STACK_SIZE   3*256 //Byte




#define NORMAL_GPIO_WAKEUP_TEST                 (0)
#define EXTERNAL_INT_WAKEUP_TEST                (0)
#if (NORMAL_GPIO_WAKEUP_TEST == 1)
  #include "hal/hal_gpio.h"
  #define NORMAL_GPIO_WAKEUP_PIN       GPIOA_0//GPIO0_7
  void GPIO_IRQHandler(void)
  {
      uint32_t status = HAL_GPIO_IntStatus();

      if ( status & _BIT(NORMAL_GPIO_WAKEUP_PIN)) {
          HAL_GPIO_IrqClear(NORMAL_GPIO_WAKEUP_PIN);
          LOG(LOG_LVL_ERROR, "[%s, %d]\r\n", __func__, __LINE__);
      }
  }
  void normal_gpio_wakeup_test(void)
  {
      GPIO_InitTypeDef irqConfig;

      irqConfig.dir = GPIO_INPUT;
      irqConfig.debounce = GPIO_DEBOUNCE_YES;
      irqConfig.trig_type = GPIO_TRIG_RISING_EDGE; //GPIO_TRIG_LOW_LEVEL; // GPIO_TRIG_FALLING_EDGE; //GPIO_TRIG_RISING_EDGE; // GPIO_TRIG_LOW_LEVEL; // GPIO_TRIG_HIGH_LEVEL;

      HAL_GPIO_Init(NORMAL_GPIO_WAKEUP_PIN, irqConfig);

      HAL_GPIO_UnmaskIrq(NORMAL_GPIO_WAKEUP_PIN);
      HAL_GPIO_IntEnable(NORMAL_GPIO_WAKEUP_PIN);
      NVIC_EnableIRQ(GPIO_IRQn);
      hal_sleep_register(MOD_GPIO, NULL, NULL, NULL);
  }
#endif

#if (EXTERNAL_INT_WAKEUP_TEST == 1)
#include "hal/hal_gpio.h"
#include "hal/hal_syscon.h"
#define EXTERNAL_INT_WAKEUP_PIN       GPIOA_0
void external_int_wakeup_test(void)
{
    SYSTEM_EXT_INT_Triggle_Type triggle = SYSTEM_EXT_INT_TRIG_RISING_EDGE;
    SYSTEM_EXT_INT_Wakeup_Index ext_int_idx = HAL_GPIO_Mapping_To_Ext_Int(EXTERNAL_INT_WAKEUP_PIN);

    HAL_SYSCON_EXT_INTR_Set_Triggle_Condition(ext_int_idx, triggle);
    HAL_SYSCON_EXT_INTR_Enable(ext_int_idx, true);
    NVIC_EnableIRQ(EXTERNAL_IRQn);
    hal_sleep_register(MOD_EXT_INT, NULL, NULL, NULL);
}
#endif

void wifi_init_sta(void)
{
    uint8_t macaddr[6] = {0}, macaddr_default[6] = {0};
    #if 0
    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = "LN_emp",
            .password = "LIGHT2020^)#",
            0,
        },
    };
    #else
    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = "TP-LINK_479A64",
            .password = "87654321",
            0,
        },
    };
    #endif

    wifi_config_t temp_config = {0};
    wifi_init_type_t init_param = {
        .wifi_mode = WIFI_MODE_STATION,
        .sta_ps_mode = WIFI_NO_POWERSAVE,//WIFI_MAX_POWERSAVE,
        #if 1
        .dhcp_mode = WLAN_DHCP_CLIENT,
        #else
        .dhcp_mode = WLAN_STATIC_IP,
        .local_ip_addr = "192.168.1.110",
        .net_mask = "255.255.255.0",
        .gateway_ip_addr = "192.168.1.1",
        #endif
        //In station mode, define the length of the AP list scanned.
        .scanned_ap_list_size = SCANNED_AP_LIST_SIZE,
    };

#if (NORMAL_GPIO_WAKEUP_TEST == 1)
    normal_gpio_wakeup_test();
#endif
#if (EXTERNAL_INT_WAKEUP_TEST == 1)
    external_int_wakeup_test();
#endif

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
    wifi_get_config(STATION_IF, &temp_config);
    if(strlen((const char *)temp_config.sta.ssid) > 0){
        //If there is a valid config in flash, use it;
        wifi_set_config_current(STATION_IF, &temp_config);
    }else{
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
            .ssid            = "art2000",
            .ssid_len        = strlen("art2000"),
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
    if(ln_is_valid_mac((const char *)macaddr) && memcmp(macaddr, macaddr_default, 6) != 0){
        //If there is a valid MAC in flash, use it
        wifi_set_macaddr_current(SOFT_AP_IF, macaddr);
    }else{
        //generate random macaddr
        ln_generate_random_mac(macaddr);
        wifi_set_macaddr(SOFT_AP_IF, macaddr);
    }

    //Check config
    wifi_get_config(SOFT_AP_IF, &temp_config);
    system_parameter_get_wifi_config_default (SOFT_AP_IF, &config_default);
    if(memcmp(&temp_config, &config_default, sizeof(wifi_config_t)) != 0 && strlen((const char *)temp_config.ap.ssid) > 0){
        //If there is a valid config in flash, use it;
        wifi_set_config_current(SOFT_AP_IF, &temp_config);
    }else{
        //else, use the prev wifi_config and save it to flash.
        wifi_set_config(SOFT_AP_IF, &wifi_config);
    }

    //Startup WiFi.
    if(0 != wifi_start(&init_param, true)){//WIFI_MAX_POWERSAVE
        LOG(LOG_LVL_ERROR, "[%s, %d]wifi_start() fail.\r\n", __func__, __LINE__);
    }
}


void usr_app_task_entry(void *params)
{
    tcpip_ip_info_t ip_info = {0};
    wifi_interface_enum_t if_index;

    //Set sleep mode
    hal_sleep_set_mode(ACTIVE);

    LOG(LOG_LVL_INFO, "wlib version string: %s\r\n",     wifi_lib_version_string_get());
    LOG(LOG_LVL_INFO, "wlib version number: 0x%08x\r\n", wifi_lib_version_number_get());
    LOG(LOG_LVL_INFO, "wlib build time    : %s\r\n",     wifi_lib_build_time_get());

    LOG(LOG_LVL_INFO, "SDK  version string: %s\r\n",     LN_SDK_VERSION_STRING);
    LOG(LOG_LVL_INFO, "SDK  version number: 0x%08x\r\n", LN_SDK_VERSION);
    LOG(LOG_LVL_INFO, "SDK  build time    : %s\r\n",     LN_SDK_BUILD_DATE_TIME);

    if_index = STATION_IF;
    wifi_init_sta();
//    if_index = SOFT_AP_IF;
//    wifi_init_ap();

    //Wait for network link up
    while(LINK_UP != ethernetif_get_link_state()){
        OS_MsDelay(1000);
    }
    ethernetif_get_ip_info(if_index, &ip_info);
    LOG(LOG_LVL_INFO, "ip = %s \r\n", ip4addr_ntoa(&ip_info.ip));
    LOG(LOG_LVL_INFO, "mask = %s \r\n", ip4addr_ntoa(&ip_info.netmask));
    LOG(LOG_LVL_INFO, "gateway = %s \r\n", ip4addr_ntoa(&ip_info.gw));

    uint32_t idle_cnt = 0;
    while(1)
    {
        OS_MsDelay(1000);
        //LOG(LOG_LVL_ERROR, "usr_app_task_entry()\r\n");
        SEGGER_SYSVIEW_ErrorfHost("idle cnt = %d", idle_cnt++);
    }
}

#if WIFI_TEMP_CALIBRATE
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
#endif

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



