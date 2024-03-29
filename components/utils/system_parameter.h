#ifndef __SYSTEM_PARAMETER_H__
#define __SYSTEM_PARAMETER_H__

#include "wifi/wifi.h"
#include "dhcpd_api.h"


#define NETIF_HOSTNAME_LEN_MAX                  (8)

typedef struct {
    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gw;
} tcpip_ip_info_t;

int system_parameter_init(void);
int system_parameter_deinit(void);

//mac config
int system_parameter_set_macaddr(wifi_interface_enum_t if_index, uint8_t *macaddr);
int system_parameter_get_macaddr(wifi_interface_enum_t if_index, uint8_t *macaddr);
//wifi config
int system_parameter_set_config(wifi_interface_enum_t if_index, wifi_config_t *config);
int system_parameter_get_config(wifi_interface_enum_t if_index, wifi_config_t *config);
//ip config
int system_parameter_set_ip_config(wifi_interface_enum_t if_index, tcpip_ip_info_t *ap_ip_config);
int system_parameter_get_ip_config(wifi_interface_enum_t if_index, tcpip_ip_info_t *ap_ip_config);

//ap_hostname config
int system_parameter_set_hostname(wifi_interface_enum_t if_index, uint8_t *hostname);
int system_parameter_get_hostname(wifi_interface_enum_t if_index, uint8_t *hostname);

int system_parameter_set_wifi_mode(wifi_mode_enum_t wifi_mode);
wifi_mode_enum_t system_parameter_get_wifi_mode(void);

int system_parameter_set_country_code(wifi_country_code_enum_t ccode);
wifi_country_code_enum_t system_parameter_get_country_code(void);

int system_parameter_set_dhcpd_config(server_config_t *server_config);
int system_parameter_get_dhcpd_config(server_config_t *server_config);

int system_parameter_get_dhcpd_config_default (server_config_t *server_config);
int system_parameter_get_wifi_macaddr_default (wifi_interface_enum_t if_index, uint8_t *macaddr);
int system_parameter_get_wifi_config_default (wifi_interface_enum_t if_index, wifi_config_t *config);
int system_parameter_get_ip_config_default (wifi_interface_enum_t if_index, tcpip_ip_info_t *ip_config);
int system_parameter_get_hostname_default (wifi_interface_enum_t if_index, char *hostname);

#endif /* __SYSTEM_PARAMETER_H__ */

