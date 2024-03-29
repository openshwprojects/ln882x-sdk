#include "ln_types.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "utils/cJSON.h"
#include "airkiss_port_qcloud.h"

static airkiss_token_info_t sg_airkiss_token = { 0 };
static xcloud_pltform_devinfo_t g_devinfo = { 0 };

/**
 * @brief
 *
 * @param devinfo
 * @return int return LN_TRUE on success, LN_FALSE on failure.
 */
static int platform_get_devinfo(xcloud_pltform_devinfo_t *devinfo)
{
	if (NULL == devinfo) {
		ak_port_printf("%s | invalid parameter!\r\n", __func__);
		return LN_FALSE;
	}

	DeviceInfo pltDevInfo = { 0 }; // tencent qcloud platform
	if (QCLOUD_RET_SUCCESS != HAL_GetDevInfo((void *)&pltDevInfo)) {
		ak_port_printf("%s | HAL_GetDevInfo() failed!\r\n", __func__);
		return LN_FALSE;
	}

	memset(devinfo, 0, sizeof(xcloud_pltform_devinfo_t));
	memcpy(devinfo->product_id, pltDevInfo.product_id, strlen(pltDevInfo.product_id));
	memcpy(devinfo->device_name, pltDevInfo.device_name, strlen(pltDevInfo.device_name));
	memcpy(devinfo->product_ver, PRODUCT_VERSION_STRING, strlen(PRODUCT_VERSION_STRING));

	return LN_TRUE;
}

/**
 * @brief Parse token from `recvbuf` and save to `tokenbuf` if parse ok.
 *
 * @param recvbuf
 * @param tokenbuf
 * @param tokenbuflen
 * @return int return LN_TRUE on success, LN_FALSE on failure.
 */
static int parse_and_save_token(char *recvbuf, airkiss_token_info_t* token_info)
{
	if ((NULL == recvbuf) || (NULL == token_info)) {
		ak_port_printf("%s() invalid params!\r\n", __func__);
		return LN_FALSE;
	}

	cJSON *root = cJSON_Parse((const char *)recvbuf);
	if( NULL == root ) {
		ak_port_printf("%s() invalid cJSON string buf!!!\r\n", __func__);
		return LN_FALSE;
	}

	cJSON *cmdTypeObj = cJSON_GetObjectItem(root, "cmdType");
	cJSON *tokenObj = cJSON_GetObjectItem(root, "token");

	if ( ( NULL != cmdTypeObj) && (NULL != tokenObj)
		&& (cJSON_Number == cmdTypeObj->type) && (cmdTypeObj->valueint == AIRKISS_TOKEN_CMDTYPE_SET)
		&& (cJSON_String == tokenObj->type) ) {
			ak_port_printf("PARSE OK, token = [%s]\r\n", tokenObj->valuestring);
			memset(token_info, 0, sizeof(airkiss_token_info_t));
			memcpy(token_info->token_string, tokenObj->valuestring, strlen(tokenObj->valuestring));
			token_info->token_is_valid = 1;
	} else {
		ak_port_printf("%s() parse token failed!!!\r\n", __func__);
		cJSON_Delete(root);
		return LN_FALSE;
	}

	cJSON_Delete(root);
	return LN_TRUE;
}

/**
 * @brief 利用设备3元组构造 token 回复信息。
 *
 * @param pDevinfo 设备三元组
 * @return char*
 */
char * construct_token_reply_msg(xcloud_pltform_devinfo_t *pDevinfo)
{
	char *msg = NULL;

	cJSON* rootObj = cJSON_CreateObject();
	cJSON* cmdTypeObj = cJSON_CreateNumber(AIRKISS_TOKEN_CMDTYPE_REPLY);
	cJSON* productIdObj = cJSON_CreateString((const char *)pDevinfo->product_id);
	cJSON* deviceNameObj = cJSON_CreateString((const char *)pDevinfo->device_name);
	cJSON* productVerObj = cJSON_CreateString((const char *)pDevinfo->product_ver);

	if ((NULL != rootObj)
		&& (NULL != cmdTypeObj)
		&& (NULL != productIdObj)
		&& (NULL != deviceNameObj)
		&& (NULL != productVerObj)) {
		cJSON_AddItemToObject(rootObj, (const char *)"cmdType", cmdTypeObj);
		cJSON_AddItemToObject(rootObj, (const char *)"productId", productIdObj);
		cJSON_AddItemToObject(rootObj, (const char *)"deviceName", deviceNameObj);
		cJSON_AddItemToObject(rootObj, (const char *)"productVersion", productVerObj);
	}

	msg = cJSON_Print(rootObj);
	ak_port_printf("token reply msg: \r\n%s\r\n", msg);

	cJSON_Delete(rootObj);
	return msg;
}

void deconstruct_token_reply_msg(void *pMsg)
{
	if (pMsg) {
		OS_Free(pMsg);
	}
}


/**
 * @brief
 *
 * @param port
 * @return int
 */
int airkiss_recv_and_reply_token(void)
{
	int socket_fd = -1;
	struct sockaddr_in local, remote;
	socklen_t socklen = 0;

	struct timeval recv_timeout;
	recv_timeout.tv_sec = 1;
	recv_timeout.tv_usec = 0;

	int retry_cnt = 10;

	char temp_recv_buf[AIRKISS_TOKEN_MAX_LENGTH * 2] = { 0 };

	memset(&local, 0, sizeof(struct sockaddr_in));
	local.sin_len = sizeof(local);
	local.sin_family = AF_INET;
	local.sin_port = htons(AIRKISS_TOKEN_RECV_PORT);
	local.sin_addr.s_addr = htons(INADDR_ANY);

	if ( 0 > (socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) ) {
		ak_port_printf("%s() create socket fail.\r\n", __func__);
		return LN_FALSE;
	}

	if ( 0 != bind(socket_fd, (struct sockaddr *)&local, sizeof(struct sockaddr_in)) ) {
		ak_port_printf("%s() bind socket fail.\r\n", __func__);
		closesocket(socket_fd);
		return LN_FALSE;
	}

	if ( 0 != setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(struct timeval)) ) {
		ak_port_printf("%s() setsockopt SO_RCVTIMEO fail.\r\n", __func__);
		closesocket(socket_fd);
		return LN_FALSE;
	}

	while (retry_cnt-- > 0) {
		socklen = sizeof(struct sockaddr_in);
		int recv_ret = recvfrom(socket_fd, (void *)temp_recv_buf, sizeof(temp_recv_buf), 0, (struct sockaddr *)&remote, &socklen);
		if (recv_ret > 0) {
			// ak_port_printf("token recv string: [%s]\r\n", temp_recv_buf);
			if (LN_TRUE == parse_and_save_token(temp_recv_buf, &sg_airkiss_token)) {
				break;
			}
		}
	}

	if (retry_cnt <= 0) {
		closesocket(socket_fd);
		return LN_FALSE;
	}

	// 获取产品信息
	if ( LN_FALSE == platform_get_devinfo(&g_devinfo) ) {
		closesocket(socket_fd);
		return LN_FALSE;
	}

	// 构造 token 回复信息
	char *token_reply_msg = construct_token_reply_msg(&g_devinfo);
	if (NULL == token_reply_msg) {
		ak_port_printf("%s | token_reply_msg error!\r\n", __func__);
		closesocket(socket_fd);
		return LN_FALSE;
	}

	// reply token message to mobile device.
	for (retry_cnt = 0; retry_cnt < 10; retry_cnt++) {
		if (0 != sendto(socket_fd, token_reply_msg, strlen(token_reply_msg),
						0, (struct sockaddr *)&remote, socklen) ) {
			// no need to send if the mobile device has received this message and closed remote port.
			if (retry_cnt > 2 ) {
				// at least send 3 times.
				break;
			}
		}
	}
	deconstruct_token_reply_msg(token_reply_msg);

	closesocket(socket_fd);
	return LN_TRUE;
}

airkiss_token_info_t* airkiss_get_token(void)
{
    return &sg_airkiss_token;
}
