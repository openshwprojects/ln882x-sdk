#include "ln_smartcfg_port.h"
#include "ln_types.h"
#include "osal/osal.h"
#include "wifi/wifi.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#define    SC_ACK_UDP_IP           ("239.5.5.5")
#define    SC_ACK_UDP_PORT         (8765)
#define    CHANNEL_HOP_INTERVAL    (150)
static     OS_Timer_t    channel_hop_timer;

void ln_sc_wifi_set_channel(uint8_t ch)
{
    wifi_set_channel(ch);
}

void ln_sc_wifi_channel_hop_start(void * timer_cb, uint16_t * channel_map)
{
    OS_TimerCreate(&channel_hop_timer, OS_TIMER_PERIODIC, (OS_TimerCallback_t)timer_cb, channel_map, CHANNEL_HOP_INTERVAL);
    OS_TimerStart(&channel_hop_timer);
}

void ln_sc_wifi_channel_hop_stop(void)
{
    OS_TimerStop(&channel_hop_timer);
    OS_TimerDelete(&channel_hop_timer);
}

void ln_sc_wifi_sniffer_enable(void * callback)
{
    wifi_set_promiscuous_filter(WIFI_PROMIS_FILTER_MASK_DATA);

    wifi_set_promiscuous_rx_cb((wifi_promiscuous_cb_t)callback);

    wifi_set_promiscuous(true);
}

void ln_sc_wifi_sniffer_disable(void)
{
    wifi_set_promiscuous(false);
    wifi_set_promiscuous_rx_cb(NULL);
}

int ln_sc_aes_decrypt(char * key, uint8_t * enc_data,uint16_t enc_data_len, uint8_t * dec_buf)
{
    // FIXME: hardware aes decrypt

    return LN_TRUE;
}

int ln_sc_udp_send_ack(uint8_t random)
{
	uint8_t send_data = 0;
	int optval = 0;
	int ret = 0;

	struct sockaddr_in addr;
	int socketfd;

	send_data = random;

	if (0 > (socketfd = socket(AF_INET, SOCK_DGRAM, 0))) {
		sc_log("%s() create socket fail\n", __func__);
		return LN_FALSE;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_port   = htons(SC_ACK_UDP_PORT);
	addr.sin_family = AF_INET;
	if (0 > inet_aton(SC_ACK_UDP_IP, &addr.sin_addr)) {
		sc_log("%s() inet_aton error\n", __func__);
		goto err;
	}

	if (0 > setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int))) {
		sc_log("%s() setsockopt error\n", __func__);
		goto err;
	}

	for (uint16_t i = 0; i < 20; i ++)
    {
		ret = sendto(socketfd, &send_data, sizeof(send_data), 0, (struct sockaddr *)&addr, sizeof(addr));
		if (ret == -1) {
			sc_log("%s() udp send error, %d\r\n", __func__);
			goto err;
		}
		OS_MsDelay(60);
	}

	closesocket(socketfd);
	return LN_TRUE;

err:
	closesocket(socketfd);
	return LN_FALSE;
}

void *ln_sc_malloc(size_t size)
{
    return OS_Malloc(size);
}

void  ln_sc_free(void *ptr)
{
    OS_Free(ptr);
}


