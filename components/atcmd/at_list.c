#include "proj_config.h"
#include "at_list.h"
#include "at_cmd_basic.h"
#include "at_cmd_wifi.h"
#include "at_parser.h"

#if BLE_SUPPORT == ENABLE
#if BLE_MESH_SUPPORT==ENABLE
extern void app_mesh_cfg_at_cmd_set_handle(char *pcmd);
#if BLE_MESH_SUPPORT_SERVER==DISABLE
extern uint8_t app_ble_at_cmd_Model_Subscription_add(char *pcmd);
extern uint8_t app_ble_at_cmd_group_onoff_set_add(char *pcmd);
extern uint8_t app_mesh_light_onoff_at_cmd_handle(char *pcmd);
#endif
#else
extern uint8_t app_ble_at_cmd_set_handle(char *pcmd);
extern uint8_t app_ble_at_cmd_execute_handle(char *pcmd);
#endif
#endif//#if BLE_SUPPORT == ENABLE

void at_cmd_init(void)
{
    at_register_short_command("AT",NULL,NULL,NULL,(at_callback)at_at_excute);
    at_register_short_command("ATE0",NULL,NULL,NULL,(at_callback)at_ate0_excute);
    at_register_short_command("ATE1",NULL,NULL,NULL,(at_callback)at_ate1_excute);

    at_register_command("RST",NULL,NULL,NULL,(at_callback)at_rst_excute);
    at_register_command("GMR",NULL,NULL,NULL,(at_callback)at_gmr_excute);
    at_register_command("GSLP",NULL,(at_callback)at_gslp_excute,NULL,NULL);
    at_register_command("RESTORE",NULL,NULL,NULL,(at_callback)at_restore_excute);
    at_register_command("SLEEP",(at_callback)at_sleep_get,(at_callback)at_sleep_set,NULL,NULL);

    //wifi mode
    at_register_command("CWMODE", (at_callback)at_get_wifi_mode, (at_callback)at_set_wifi_mode, (at_callback)at_help_wifi_mode, NULL);
    at_register_command("CWMODE_CUR", (at_callback)at_get_wifi_mode_current, (at_callback)at_set_wifi_mode_current, (at_callback)at_help_wifi_mode_current, NULL);
    at_register_command("CWMODE_DEF", (at_callback)at_get_wifi_mode_def, (at_callback)at_set_wifi_mode_def, (at_callback)at_help_wifi_mode_def, NULL);

    //config station(connect to ap)
    at_register_command("CWJAP", (at_callback)at_station_get_connected_info, (at_callback)at_station_connect, NULL, NULL);
    at_register_command("CWJAP_CUR", (at_callback)at_station_get_connected_info_current, (at_callback)at_station_connect_current, NULL, NULL);
    at_register_command("CWJAP_DEF", (at_callback)at_station_get_connected_info_def, (at_callback)at_station_connect, NULL, NULL);
    //config softAP
    at_register_command("CWSAP", (at_callback)at_softap_get_config, (at_callback)at_softap_set_config, NULL, NULL);
    at_register_command("CWSAP_CUR", (at_callback)at_softap_get_config_current, (at_callback)at_softap_set_config_current, NULL, NULL);
    at_register_command("CWSAP_DEF", (at_callback)at_softap_get_config_def, (at_callback)at_softap_set_config_def, NULL, NULL);

    //sta mac
    at_register_command("CIPSTAMAC", (at_callback)at_station_getmac, (at_callback)at_station_setmac, NULL, NULL);
    at_register_command("CIPSTAMAC_CUR", (at_callback)at_station_getmac_current, (at_callback)at_station_setmac_current, NULL, NULL);
    at_register_command("CIPSTAMAC_DEF", (at_callback)at_station_getmac_def, (at_callback)at_station_setmac_def,NULL,NULL);
    //softAP mac
    at_register_command("CIPAPMAC", (at_callback)at_softap_getmac, (at_callback)at_softap_setmac, NULL, NULL);
    at_register_command("CIPAPMAC_CUR", (at_callback)at_softap_getmac_current, (at_callback)at_softap_setmac_current, NULL, NULL);
    at_register_command("CIPAPMAC_DEF", (at_callback)at_softap_getmac_def, (at_callback)at_softap_setmac_def,NULL,NULL);

    //sta ip
    at_register_command("CIPSTA",    (at_callback)at_get_sta_ip,    (at_callback)at_set_sta_ip_cur,NULL,NULL);
    at_register_command("CIPSTA_CUR",(at_callback)at_get_sta_ip_cur,(at_callback)at_set_sta_ip_cur,NULL,NULL);
    at_register_command("CIPSTA_DEF",(at_callback)at_get_sta_ip_def,(at_callback)at_set_sta_ip_def,NULL,NULL);
    //ap ip
    at_register_command("CIPAP",     (at_callback)at_get_ap_ip,     (at_callback)at_set_ap_ip_def,NULL,NULL);
    at_register_command("CIPAP_CUR", (at_callback)at_get_ap_ip_cur, (at_callback)at_set_ap_ip_cur,NULL,NULL);
    at_register_command("CIPAP_DEF", (at_callback)at_get_ap_ip_def, (at_callback)at_set_ap_ip_def,NULL,NULL);


#if WIFI_TRACK
    at_register_command("CWLAPLN",NULL,(at_callback)at_station_scan,NULL,(at_callback)at_station_scan_no_filter);
	at_register_command("CWLAP",NULL,NULL,NULL,(at_callback)at_station_aplx);
    at_register_command("CWLAPLST",NULL,NULL,NULL,(at_callback)at_station_aplist);
#else
    at_register_command("CWLAP",NULL,(at_callback)at_station_scan,NULL,(at_callback)at_station_scan_no_filter);
#endif
    at_register_command("CWQAP",NULL,NULL,NULL,(at_callback)at_station_disconnect);
    at_register_command("CWLAPOPT",NULL,(at_callback)at_station_set_scan_list_display_option,NULL,NULL);
    at_register_command("CWLIF", NULL, NULL, NULL, (at_callback)at_softap_get_station_list);

    at_register_command("CWAUTOCONN",(at_callback)at_get_station_auto_connect,(at_callback)at_set_station_auto_connect,NULL,NULL);
    at_register_command("CWHOSTNAME",(at_callback)at_get_sta_host_name,(at_callback)at_set_sta_host_name,NULL,NULL);

    //dhcp
    at_register_command("CWDHCP",    (at_callback)at_get_dhcp_def,  (at_callback)at_set_dhcp_def,NULL,NULL);
    at_register_command("CWDHCP_CUR",(at_callback)at_get_dhcp_cur,  (at_callback)at_set_dhcp_cur,NULL,NULL);
    at_register_command("CWDHCP_DEF",(at_callback)at_get_dhcp_def,  (at_callback)at_set_dhcp_def,NULL,NULL);
    //dhcps
    at_register_command("CWDHCPS_CUR",(at_callback)at_get_dhcps_cur,(at_callback)at_set_dhcps_cur,NULL,NULL);
    at_register_command("CWDHCPS_DEF",(at_callback)at_get_dhcps_def,(at_callback)at_set_dhcps_def,NULL,NULL);

    //ping
    at_register_command("PING",NULL,(at_callback)at_ping,NULL,NULL);

    //misc cmd
    at_register_command("PVTCMD",NULL,(at_callback)at_pvtcmd_set,NULL,NULL);                             //wifi private cmd
    at_register_command("CPUUSG",(at_callback)at_get_cpu_usage,NULL,NULL,(at_callback)at_get_cpu_usage); //cpu usage
    at_register_command("GETHEAPSIZE",NULL,NULL,NULL,(at_callback)at_get_heap_size);                     //heap size
    at_register_command("RF_TEST_DESTORY", NULL, (at_callback)at_rf_test_destory, NULL, NULL);           //rf_test_bin distory
    at_register_command("TXPOWER_COMP", (at_callback)at_get_tx_power_comp, (at_callback)at_set_tx_power_comp, NULL,NULL);//wifi tx power comp
    at_register_command("XTAL_COMP", (at_callback)at_get_xtal_cap_comp, (at_callback)at_set_xtal_cap_comp, NULL,NULL);
    at_register_command("IPERF",NULL,(at_callback)at_iperf,NULL,NULL);                                                   //iperf test

    at_register_command("CSN", (at_callback)at_get_chip_sn,   (at_callback)at_set_chip_sn,NULL,NULL);    //chip SN
    at_register_command("CEFUSE", (at_callback)at_get_efuse, (at_callback)at_set_efuse, NULL,NULL);      //CEFUSE


    at_register_command("SW_VERSION",NULL,NULL,NULL,(at_callback)at_software_version);
#if WIFI_SWITCH
    at_register_command("WF_SWITCH",NULL,(at_callback)at_wifi_switch,NULL,NULL);
#endif

#if BLE_SUPPORT == ENABLE
  #if BLE_MESH_SUPPORT==ENABLE
    at_register_command("BLEPTS",NULL,(at_callback)app_mesh_cfg_at_cmd_set_handle,NULL,NULL);
    #if BLE_MESH_SUPPORT_SERVER==DISABLE
    at_register_command("BLESUBADD",NULL,(at_callback)app_ble_at_cmd_Model_Subscription_add,NULL,NULL);
    at_register_command("GROUPONOFF",NULL,(at_callback)app_ble_at_cmd_group_onoff_set_add,NULL,NULL);
    at_register_command("ONOFF",NULL,(at_callback)app_mesh_light_onoff_at_cmd_handle,NULL,NULL);
    #endif
  #else
    at_register_command("BLECMD",NULL,(at_callback)app_ble_at_cmd_set_handle,NULL,NULL);
    at_register_command("BLERST",NULL,NULL,NULL,(at_callback)app_ble_at_cmd_execute_handle);
  #endif
#endif //#if BLE_SUPPORT == ENABLE
}



