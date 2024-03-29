#ifndef AIRKISS_PORT_QCLOUD_H
#define AIRKISS_PORT_QCLOUD_H

#include "qcloud_iot_import.h"
#include "qcloud_iot_export.h"

#define PRODUCT_VERSION_STRING 			"v0.1.1"

// receive `token` from mobile device via UDP server port 8266.
#define AIRKISS_TOKEN_RECV_PORT 		(8266)

#define AIRKISS_TOKEN_CMDTYPE_SET		(0) // mobile device -->   WiFi module
#define AIRKISS_TOKEN_CMDTYPE_REPLY 	(2)	// WiFi module   --> mobile device

#define AIRKISS_TOKEN_MAX_LENGTH 		(100)

#define XCLOUD_PLATFORM_PRODUCTID_LENGTH 	(MAX_SIZE_OF_PRODUCT_ID + 1) 	// 10+1
#define XCLOUD_PLATFORM_DEVNAME_LENGTH 		(MAX_SIZE_OF_DEVICE_NAME + 1) 	// 48+1
#define XCLOUD_PLATFORM_PRODUCTVER_LENGTH 	(10)

#define XCLOUD_PLATFORM_BIND_TOKEN_REPLY_METHOD   	"app_bind_token_reply"
#define XCLOUD_PLATFORM_BIND_TOKEN_REPLY_CODE     	0
#define XCLOUD_PLATFORM_METHOD_BIND_DEVICE     		"bind_device"
#define XCLOUD_PLATFORM_TOKEN_REPORT_MAX 			(20)

typedef struct airkiss_token {
	char token_string[AIRKISS_TOKEN_MAX_LENGTH];
	int  token_is_valid;
} airkiss_token_info_t;

typedef struct xcloud_pltform_devinfo {
	char product_id[XCLOUD_PLATFORM_PRODUCTID_LENGTH];
	char device_name[XCLOUD_PLATFORM_DEVNAME_LENGTH];
	char product_ver[XCLOUD_PLATFORM_PRODUCTVER_LENGTH];
} xcloud_pltform_devinfo_t;

typedef struct _bind_status
{
    int8_t bind_token_reply_cnt;
    int8_t bind_device_cnt;
    int8_t bind_is_ok;
} bind_status_t;

int     airkiss_recv_and_reply_token(void);
airkiss_token_info_t* airkiss_get_token(void);

#endif // !AIRKISS_PORT_QCLOUD_H
