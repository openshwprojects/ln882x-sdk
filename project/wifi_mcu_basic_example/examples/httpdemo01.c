#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "proj_config.h"
#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"
#include "osal/osal.h"

#include "httpsclient.h"
#include "httpsclient_wrapper.h"
#include "httpsclient_debug.h"
#include "demo_def.h"

/**
 * @brief An HTTP GET callback demo.
 *
 * @param recvbuf Buffer for HTTP GET store received data.
 * @param recvbuf_len Length of the buffer.
 * @param total_bytes How many bytes will HTTP GET to fetch.
 * @param is_recv_finished Indicates whether HTTP GET fetch all data.
 * @return int Return how many bytes this callback has done with.
 */
int my_http_get_cb(const char *recvbuf, int32_t recvbuf_len, int32_t total_bytes, int8_t is_recv_finished)
{
    if (!is_recv_finished) {
        LOG_I("cb data len: %d\r\n", recvbuf_len);
        LOG_I("cb data: \r\n[\r\n%s\r\n]\r\n", recvbuf);
    } else {
        // no data to deal with.
        LOG_I("cb info: recv %d finished, no more data to deal with.\r\n", total_bytes);
    }
    return recvbuf_len;
}

int httpdemo01_entry(void *params)
{
    int retCode = 0;
    HTTP_INFO *http_info = NULL;
    char *urlStr = NULL;
    int recvTotal = 0;

    OS_SecDelay(15);

    while(1) {

        http_info = httpinfo_new();
        if (http_info == NULL) {
            LOG_EMSG("malloc for @http_info failed.\r\n");
            retCode = -1;
            goto flag_exit;
        }

        if (http_init(&http_info, FALSE) != 0) {
            retCode = -1;
            goto flag_err;
        }

        urlStr = DEMO_GET_URL_PDF;

        recvTotal = http_get_with_callback(http_info, urlStr, my_http_get_cb);
        if (recvTotal) {
            LOG_I("total received %d bytes.\r\n", recvTotal);
        } else {
            retCode = -1;
            LOG_EMSG("http_get error!!!\r\n");
        }

        http_close(http_info);

    flag_err:
        httpinfo_delete(http_info);

    flag_exit:

        OS_SecDelay(5);
        LOG_EMSG("http client is restart...\r\n");
    } // while(1)

    return retCode;
}

static OS_Thread_t g_httpdemo01_thread;
#define HTTPDEMO01_STACK_SIZE   (4 * 256 * 15)
void create_httpdemo01_task(void)
{
    if (OS_OK != OS_ThreadCreate(&g_httpdemo01_thread, "httpdemo01", httpdemo01_entry, NULL, OS_PRIORITY_NORMAL, HTTPDEMO01_STACK_SIZE)) {
        ART_ASSERT(1);
    }
}
