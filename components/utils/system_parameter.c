#include "dflt_param_tab.h"
#include "lwip/ip_addr.h"
#include "ln_kv_api.h"
#include "utils/art_string.h"
#include "utils/system_parameter.h"
#include "utils/debug/log.h"
#include "utils/debug/art_assert.h"
#include <string.h>

#if BLE_SUPPORT == ENABLE
#if BLE_MESH_SUPPORT == ENABLE
#include "../../../ble/app/mesh/app_mesh.h"
#include "../../../ble/app/mesh/app_mesh_firmware.h"
#if BLE_MESH_SUPPORT_SERVER == ENABLE
#include "../../../ble/app/mesh/app_mesh_cfg_server.h"
#else
#include "../../../ble/app/mesh/app_mesh_cfg_client.h"
#endif//#if BLE_MESH_SUPPORT_SERVER == ENABLE
#else
#include "../ble/include/ble_common.h"
#endif//#if BLE_MESH_SUPPORT == ENABLE
#endif//#if BLE_SUPPORT == ENABLE


typedef struct {
    uint8_t                     addr[MAC_ADDRESS_LEN];
    uint8_t                     ap_addr[MAC_ADDRESS_LEN];
    uint8_t                     hostname[NETIF_HOSTNAME_LEN_MAX+1];
    uint8_t                     ap_hostname[NETIF_HOSTNAME_LEN_MAX+1];
    wifi_config_t               config;
    wifi_config_t               ap_config;
    tcpip_ip_info_t             ip_info;
    tcpip_ip_info_t             ap_ip_info;
    server_config_t             server_config;
#if BLE_SUPPORT == ENABLE
#if BLE_MESH_SUPPORT == ENABLE
#if BLE_MESH_SUPPORT_SERVER==ENABLE
    Mesh_Node_Cfg_Info      mesh_node_config;
    Node_Element_Flash_Info mesh_element_config;
    Mesh_Key_Info           mesh_key_config;
    Mesh_User_Data          mesh_user_config;
    Firmware_Cfg_Info       mesh_firmware_config;
#else
    Client_Flash_Info       mesh_client_config;
#endif
#else
    white_list_t            ble_white_list_config;
#endif
#endif
} system_parameter_t;

int system_parameter_init(void)
{
    int ret = 0;
	system_parameter_t	system_parameter;

	memset(&system_parameter, 0, sizeof(system_parameter_t));

    if(WIFI_MODE_MAX == system_parameter_get_wifi_mode()){
        //mode
		system_parameter_set_wifi_mode(WIFI_MODE_DEFAULT);
    }
    if(CTRY_CODE_MAX == system_parameter_get_country_code()){
        //country code
		system_parameter_set_country_code(CTRY_CODE_CN);
    }
    ret = system_parameter_get_macaddr(STATION_IF, system_parameter.addr);
    if(ret < 0){
        //sta_macaddr
	    ln_generate_random_mac(system_parameter.addr);//generate random macaddr
		system_parameter_set_macaddr(STATION_IF, system_parameter.addr);
    }
    ret = system_parameter_get_macaddr(SOFT_AP_IF, system_parameter.ap_addr);
    if(ret < 0){
        //ap_macaddr
	    ln_generate_random_mac(system_parameter.ap_addr);//generate random macaddr
		system_parameter_set_macaddr(SOFT_AP_IF, system_parameter.ap_addr);
    }

    ret = system_parameter_get_hostname(STATION_IF, system_parameter.hostname);
    if(ret < 0){
	    system_parameter_get_hostname_default(STATION_IF, (char *)system_parameter.hostname);
		system_parameter_set_hostname(STATION_IF, system_parameter.hostname);
    }
    ret = system_parameter_get_hostname(SOFT_AP_IF, system_parameter.ap_hostname);
    if(ret < 0){
	    system_parameter_get_hostname_default(SOFT_AP_IF, (char *)system_parameter.ap_hostname);
		system_parameter_set_hostname(SOFT_AP_IF, system_parameter.ap_hostname);
    }

    ret = system_parameter_get_config(STATION_IF, &system_parameter.config);
    if(ret < 0){
	    system_parameter_get_wifi_config_default(STATION_IF, &system_parameter.config);
		system_parameter_set_config(STATION_IF, &system_parameter.config);
    }
    ret = system_parameter_get_config(SOFT_AP_IF, &system_parameter.ap_config);
    if(ret < 0){
	    system_parameter_get_wifi_config_default(SOFT_AP_IF, &system_parameter.ap_config);
		system_parameter_set_config(SOFT_AP_IF, &system_parameter.ap_config);
    }

    ret = system_parameter_get_ip_config(STATION_IF, &system_parameter.ip_info);
    if(ret < 0){
	    system_parameter_get_ip_config_default(STATION_IF, &system_parameter.ip_info);
		system_parameter_set_ip_config(STATION_IF, &system_parameter.ip_info);
    }
    ret = system_parameter_get_ip_config(SOFT_AP_IF, &system_parameter.ap_ip_info);
    if(ret < 0){
	    system_parameter_get_ip_config_default(SOFT_AP_IF, &system_parameter.ap_ip_info);
		system_parameter_set_ip_config(SOFT_AP_IF, &system_parameter.ap_ip_info);
    }

    ret = system_parameter_get_dhcpd_config(&system_parameter.server_config);
    if(ret < 0){
	    system_parameter_get_dhcpd_config_default(&system_parameter.server_config);
		system_parameter_set_dhcpd_config(&system_parameter.server_config);
    }
    return ret;
}
int system_parameter_deinit(void)
{
    return 0;
}

//mac config
int system_parameter_set_macaddr(wifi_interface_enum_t if_index, uint8_t *macaddr)
{
    int ret = 0;
    ART_ASSERT(macaddr);
    if(STATION_IF == if_index){
        if (KV_ERR_NONE !=(ret= ln_kv_set((const char *)KV_SYSTEM_PARAMETER_ADDR, (void *)macaddr, MAC_ADDRESS_LEN))) {
            return (-1*ret);
        }
    }else if(SOFT_AP_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_AP_ADDR, (void *)macaddr, MAC_ADDRESS_LEN))) {
            return (-1*ret);
        }
    }
    return 0;
}

int system_parameter_get_macaddr(wifi_interface_enum_t if_index, uint8_t *macaddr)
{
    size_t len = MAC_ADDRESS_LEN;
    int ret = 0;

    ART_ASSERT(macaddr);
    if(STATION_IF == if_index){
        if(KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_ADDR, (void *)macaddr, MAC_ADDRESS_LEN, &len))) {
            return -1*ret;
        }
    }else if(SOFT_AP_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_AP_ADDR, (void *)macaddr, MAC_ADDRESS_LEN, &len))) {
            return -1*ret;
        }
    }
	return 0;
}

//wifi config
int system_parameter_set_config(wifi_interface_enum_t if_index, wifi_config_t *config)
{
    int ret = 0;

    ART_ASSERT(config);
    if(STATION_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_CONFIG, (void *)config, sizeof(wifi_config_t)))) {
            return -1*ret;
        }
    }else if(SOFT_AP_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_AP_CONFIG, (void *)config, sizeof(wifi_config_t)))) {
            return -1*ret;
        }
    }
    return 0;
}

int system_parameter_get_config(wifi_interface_enum_t if_index, wifi_config_t *config)
{
    size_t len = sizeof(wifi_config_t);
    int ret = 0;

    ART_ASSERT(config);
    if(STATION_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_CONFIG, (void *)config, sizeof(wifi_config_t), &len))) {
            return -1*ret;
        }
    }else if(SOFT_AP_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_AP_CONFIG, (void *)config, sizeof(wifi_config_t), &len))) {
            return -1*ret;
        }
    }
    return 0;
}

//ip config
int system_parameter_set_ip_config(wifi_interface_enum_t if_index, tcpip_ip_info_t *ip_config)
{
    int ret = 0;

    ART_ASSERT(ip_config);
    if(STATION_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_IP_INFO, (void *)ip_config, sizeof(tcpip_ip_info_t)))) {
            return -1*ret;
        }
    }else if(SOFT_AP_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_AP_IP_INFO, (void *)ip_config, sizeof(tcpip_ip_info_t)))) {
            return -1*ret;
        }
    }
    return 0;
}
int system_parameter_get_ip_config(wifi_interface_enum_t if_index, tcpip_ip_info_t *ip_config)
{
    size_t len = sizeof(tcpip_ip_info_t);
    int ret = 0;

    ART_ASSERT(ip_config);
    if(STATION_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_IP_INFO, (void *)ip_config, sizeof(tcpip_ip_info_t), &len))) {
            return -1*ret;
        }
    }else if(SOFT_AP_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_AP_IP_INFO, (void *)ip_config, sizeof(tcpip_ip_info_t), &len))) {
            return -1*ret;
        }
    }
    return 0;
}

//wifi mode config
int system_parameter_set_wifi_mode(wifi_mode_enum_t wifi_mode)
{
    int ret = 0;

    if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_MODE, (void *)&wifi_mode, sizeof(wifi_mode_enum_t)))) {
        return -1*ret;
    }
    return 0;
}

wifi_mode_enum_t system_parameter_get_wifi_mode(void)
{
	wifi_mode_enum_t wifi_mode = WIFI_MODE_MAX;
    size_t len = sizeof(wifi_mode_enum_t);

    if (KV_ERR_NONE != ln_kv_get((const char *)KV_SYSTEM_PARAMETER_MODE, (void *)&wifi_mode, sizeof(wifi_mode_enum_t), &len)) {
        return wifi_mode;
    }
    return wifi_mode;
}

int system_parameter_set_country_code(wifi_country_code_enum_t ccode)
{
    int ret = 0;

	if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_CCODE, (void *)&ccode, sizeof(wifi_country_code_enum_t)))) {
        return -1*ret;
    }
    return 0;
}

wifi_country_code_enum_t system_parameter_get_country_code(void)
{
    wifi_country_code_enum_t ccode = CTRY_CODE_MAX;
    size_t len = sizeof(wifi_country_code_enum_t);

    if (KV_ERR_NONE != ln_kv_get((const char *)KV_SYSTEM_PARAMETER_CCODE, (void *)&ccode, sizeof(wifi_country_code_enum_t), &len)) {
        return ccode;
    }
    return ccode;
}

//hostname config
int system_parameter_set_hostname(wifi_interface_enum_t if_index, uint8_t *hostname)
{
    int ret = 0;

    if(STATION_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_HNAME, (void *)hostname, strlen((const char *)hostname)))) {
            return -1*ret;
        }
    }else if(SOFT_AP_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_AP_HNAME, (void *)hostname, strlen((const char *)hostname)))) {
            return -1*ret;
        }
    }
    return 0;
}

int system_parameter_get_hostname(wifi_interface_enum_t if_index, uint8_t *hostname)
{
    size_t len = NETIF_HOSTNAME_LEN_MAX;
    int ret = 0;

    if(STATION_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_HNAME, (void *)hostname, NETIF_HOSTNAME_LEN_MAX, &len))) {
            return -1*ret;
        }
    }else if(SOFT_AP_IF == if_index){
        if (KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_AP_HNAME, (void *)hostname, NETIF_HOSTNAME_LEN_MAX, &len))) {
            return -1*ret;
        }
    }
	return 0;
}

int system_parameter_set_dhcpd_config(server_config_t *server_config)
{
    int ret = 0;

    ART_ASSERT(server_config);
    if (KV_ERR_NONE != (ret=ln_kv_set((const char *)KV_SYSTEM_PARAMETER_SERVER_CONFIG, (void *)server_config, sizeof(server_config_t)))) {
        return -1*ret;
    }
    return 0;
}
int system_parameter_get_dhcpd_config(server_config_t *server_config)
{
    size_t len = sizeof(server_config_t);
    int ret = 0;

    ART_ASSERT(server_config);
    if (KV_ERR_NONE != (ret=ln_kv_get((const char *)KV_SYSTEM_PARAMETER_SERVER_CONFIG, (void *)server_config, sizeof(server_config_t), &len))) {
        return -1*ret;
    }
    return 0;
}

int system_parameter_get_wifi_config_default (wifi_interface_enum_t if_index, wifi_config_t *config)
{
    wifi_ap_config_t  *ap = NULL;
    wifi_sta_config_t *sta = NULL;
    int len = 0;

    ART_ASSERT(config);
    memset(config, 0, sizeof(wifi_config_t));

    //$)Ad;?????h.$e??0d8-?7e?
    if(if_index == STATION_IF){
        sta = &config->sta;
        //memset(sta->ssid, SSID_MAX_LEN, 0);
        //memset(sta->password, PASSWORD_MAX_LEN, 0);
        //memset(sta->bssid, BSSID_LEN, 0);
        sta->channel = 1;
        sta->bssid_set = 0;
    }else{
        ap = &config->ap;
        ap->ssid_len = strlen(WIFI_AP_SSID_DEFAULT);
        memcpy(ap->ssid, WIFI_AP_SSID_DEFAULT, ap->ssid_len);
        len = (strlen(WIFI_AP_PASSWORD_DEFAULT) < PASSWORD_MAX_LEN) ? strlen(WIFI_AP_PASSWORD_DEFAULT) : PASSWORD_MAX_LEN;
        memcpy(ap->password, WIFI_AP_PASSWORD_DEFAULT, len);
        if(strlen((const char *)ap->password) > 0){
            ap->authmode = WIFI_AUTH_WPA2_PSK;
        }else{
            ap->authmode = WIFI_AP_AUTH_MODE_DEFAULT;
        }
        ap->channel = WIFI_AP_CHANNEL_DEFAULT;
        ap->ssid_hidden = WIFI_AP_SSID_HIDDEN_DEFAULT;
        ap->max_connection = WIFI_AP_MAX_CONNECTION_DEFAULT;
        ap->beacon_interval = WIFI_AP_BEACON_INTERVAL_DEFAULT;
    }

    return 0;
}
int system_parameter_get_wifi_macaddr_default (wifi_interface_enum_t if_index, uint8_t *macaddr)
{
    ART_ASSERT(macaddr);
    memset(macaddr, 0, MAC_ADDRESS_LEN);
    if(if_index == STATION_IF){
        macaddr[0] = MAC_ADDR0;
        macaddr[1] = MAC_ADDR1;
        macaddr[2] = MAC_ADDR2;
        macaddr[3] = MAC_ADDR3;
        macaddr[4] = MAC_ADDR4;
        macaddr[5] = MAC_ADDR5;
    }else{
        macaddr[0] = AP_MAC_ADDR0;
        macaddr[1] = AP_MAC_ADDR1;
        macaddr[2] = AP_MAC_ADDR2;
        macaddr[3] = AP_MAC_ADDR3;
        macaddr[4] = AP_MAC_ADDR4;
        macaddr[5] = AP_MAC_ADDR5;
    }
    return 0;
}

int system_parameter_get_ip_config_default (wifi_interface_enum_t if_index, tcpip_ip_info_t *ip_config)
{
    ART_ASSERT(ip_config);
    memset(ip_config, 0, sizeof(tcpip_ip_info_t));
    if(if_index == STATION_IF){
        IP_ADDR4(&ip_config->ip, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
        IP_ADDR4(&ip_config->netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
        IP_ADDR4(&ip_config->gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
    }else{
        IP_ADDR4(&ip_config->ip, AP_IP_ADDR0, AP_IP_ADDR1, AP_IP_ADDR2, AP_IP_ADDR3);
        IP_ADDR4(&ip_config->netmask, AP_NETMASK_ADDR0, AP_NETMASK_ADDR1, AP_NETMASK_ADDR2, AP_NETMASK_ADDR3);
        IP_ADDR4(&ip_config->gw, AP_GW_ADDR0, AP_GW_ADDR1, AP_GW_ADDR2, AP_GW_ADDR3);
    }
    return 0;
}

int system_parameter_get_dhcpd_config_default (server_config_t *server_config)
{
    ART_ASSERT(server_config);
    memset(server_config, 0, sizeof(server_config_t));
    ip4_addr_set_u32(&(server_config->server), AP_IP_ADDR0|(AP_IP_ADDR1 << 8)|(AP_IP_ADDR2 << 16)|(AP_IP_ADDR3 << 24));
    server_config->port = DHCPD_LISTEN_PORT;
    server_config->lease = DHCPD_IP_LEASE_TIME;
    server_config->renew = DHCPD_IP_RENEW_TIME;
    ip4_addr_set_u32(&(server_config->ip_start), DHCPD_IP_START);
    ip4_addr_set_u32(&(server_config->ip_end), DHCPD_IP_END);
    server_config->client_max = DHCPD_CLIENT_MAX;
    return 0;
}

int system_parameter_get_hostname_default (wifi_interface_enum_t if_index, char *hostname)
{
    ART_ASSERT(hostname);
    if(if_index == STATION_IF){
        memcpy(hostname, STA_HOSTNAME, strlen(STA_HOSTNAME));
    }else{
        memcpy(hostname, AP_HOSTNAME, strlen(AP_HOSTNAME));
    }
    return 0;
}

#if BLE_SUPPORT == ENABLE

#if BLE_MESH_SUPPORT == ENABLE
#if BLE_MESH_SUPPORT_SERVER==ENABLE

void system_parameter_set_mesh_node(Mesh_Node_Cfg_Info *mesh_node)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    system_param_item_t *param_item = get_param_item_cache();

    ART_ASSERT(system_parameter && mesh_node);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    param_item->item_id = (uint32_t)BLE_MESH_NODE_CONFIG;
    memcpy(&param_item->item.mesh_node_config, mesh_node, sizeof(Mesh_Node_Cfg_Info));
    param_item->item.mesh_node_config.cfg_flash_magic = mesh_node->cfg_flash_magic;
    system_parameter_sync_to_flash(param_item);
    OS_MutexUnlock(&system_parameter->lock);
}
void system_parameter_get_mesh_node(Mesh_Node_Cfg_Info *mesh_node)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    ART_ASSERT(system_parameter && mesh_node);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    memcpy(mesh_node, &(system_parameter->mesh_node_config), sizeof(Mesh_Node_Cfg_Info));
    OS_MutexUnlock(&system_parameter->lock);
}


void system_parameter_set_mesh_element(Node_Element_Flash_Info *mesh_element)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    system_param_item_t *param_item = get_param_item_cache();

    ART_ASSERT(system_parameter && mesh_element);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    param_item->item_id = (uint32_t)BLE_MESH_ELEMENT_CONFIG;
    memcpy(&param_item->item.mesh_element_config, mesh_element, sizeof(Node_Element_Flash_Info));
    param_item->item.mesh_element_config.element_magic = mesh_element->element_magic;
    system_parameter_sync_to_flash(param_item);
    OS_MutexUnlock(&system_parameter->lock);
}
void system_parameter_get_mesh_element(Node_Element_Flash_Info *mesh_element)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    ART_ASSERT(system_parameter && mesh_element);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    memcpy(mesh_element, &(system_parameter->mesh_element_config), sizeof(Node_Element_Flash_Info));
    OS_MutexUnlock(&system_parameter->lock);
}

void system_parameter_set_mesh_key( Mesh_Key_Info *mesh_key)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    system_param_item_t *param_item = get_param_item_cache();

    ART_ASSERT(system_parameter && mesh_key);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    param_item->item_id = (uint32_t)BLE_MESH_KEY_CONFIG;
    memcpy(&param_item->item.mesh_key_config, mesh_key, sizeof(Mesh_Key_Info));
    param_item->item.mesh_key_config.key_magic = mesh_key->key_magic;
    system_parameter_sync_to_flash(param_item);
    OS_MutexUnlock(&system_parameter->lock);
}
void system_parameter_get_mesh_key(Mesh_Key_Info *mesh_key)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    ART_ASSERT(system_parameter && mesh_key);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    memcpy(mesh_key, &(system_parameter->mesh_key_config), sizeof(Mesh_Key_Info));
    OS_MutexUnlock(&system_parameter->lock);
}


void system_parameter_set_mesh_user(Mesh_User_Data *user)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    system_param_item_t *param_item = get_param_item_cache();

    ART_ASSERT(system_parameter && user);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    param_item->item_id = (uint32_t)BLE_MESH_USER_DATA_CONFIG;
    memcpy(&param_item->item.mesh_user_config, user, sizeof(Mesh_User_Data));
    param_item->item.mesh_user_config.user_data_magic = user->user_data_magic;//MESH_USER_DATA_MAGIC_LABEL;
    system_parameter_sync_to_flash(param_item);
    OS_MutexUnlock(&system_parameter->lock);
}
void system_parameter_get_mesh_user(Mesh_User_Data *user)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    ART_ASSERT(system_parameter && user);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    memcpy(user, &(system_parameter->mesh_user_config), sizeof(Mesh_User_Data));
    OS_MutexUnlock(&system_parameter->lock);
}


void system_parameter_set_mesh_firmware(Firmware_Cfg_Info *firmware)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    system_param_item_t *param_item = get_param_item_cache();

    ART_ASSERT(system_parameter && firmware);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    param_item->item_id = (uint32_t)BLE_MESH_FIRMWARE_CFG_CONFIG;
    memcpy(&param_item->item.mesh_firmware_config, firmware, sizeof(Firmware_Cfg_Info));
    param_item->item.mesh_firmware_config.firmware_cfg_flash_magic = firmware->firmware_cfg_flash_magic;//MESH_FIRMWARE_CFG_MAGIC_LABEL;
    system_parameter_sync_to_flash(param_item);
    OS_MutexUnlock(&system_parameter->lock);
}
void system_parameter_get_mesh_firmware(Firmware_Cfg_Info *firmware)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    ART_ASSERT(system_parameter && firmware);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    memcpy(firmware, &(system_parameter->mesh_firmware_config), sizeof(Firmware_Cfg_Info));
    OS_MutexUnlock(&system_parameter->lock);
}
#else
void system_parameter_set_mesh_client_info(Client_Flash_Info *client_info)
{
    //uint8_t *src = NULL, *dst = NULL;
    system_parameter_t *system_parameter = get_system_parameter_handle();
    system_param_item_t *param_item = get_param_item_cache();

    ART_ASSERT(system_parameter && client_info);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    param_item->item_id = (uint32_t)BLE_MESH_CLIENT_CONFIG;
    //src = (uint8_t *)client_info;
    //dst = (uint8_t *)&param_item->item.mesh_client_config;
    //memcpy(dst, src, sizeof(Client_Flash_Info));
    memcpy(&param_item->item.mesh_client_config, client_info, sizeof(Client_Flash_Info));
    //param_item->item.mesh_client_config.client_magic = client_info->client_magic;
    system_parameter_sync_to_flash(param_item);
    OS_MutexUnlock(&system_parameter->lock);
}
void system_parameter_get_mesh_client_info(Client_Flash_Info *client_info)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    ART_ASSERT(system_parameter && client_info);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    memcpy(client_info, &(system_parameter->mesh_client_config), sizeof(Client_Flash_Info));
    OS_MutexUnlock(&system_parameter->lock);
}

#endif //#if BLE_MESH_SUPPORT_SERVER==ENABLE

#else
void system_parameter_set_ble_white_list_info(white_list_t *white_list_info)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    system_param_item_t *param_item = get_param_item_cache();

    ART_ASSERT(system_parameter && white_list_info);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    param_item->item_id = (uint32_t)BLE_WHITE_LIST_CONFIG;
    memcpy(&param_item->item.ble_white_list_config, white_list_info, sizeof(white_list_t));
    system_parameter_sync_to_flash(param_item);
    OS_MutexUnlock(&system_parameter->lock);

}
void system_parameter_get_ble_white_list_info(white_list_t *white_list_info)
{
    system_parameter_t *system_parameter = get_system_parameter_handle();
    ART_ASSERT(system_parameter && white_list_info);
    OS_MutexLock(&system_parameter->lock, OS_WAIT_FOREVER);
    memcpy(white_list_info, &(system_parameter->ble_white_list_config), sizeof(white_list_t));
    OS_MutexUnlock(&system_parameter->lock);
}

#endif //#if BLE_MESH_SUPPORT == ENABLE

#endif

