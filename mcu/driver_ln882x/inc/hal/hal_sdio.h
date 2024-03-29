#ifndef __HAL_SDIO_H__
#define __HAL_SDIO_H__

#include <stdint.h>
#include "ln_types.h"
#include "osal/osal.h"
#include "ll/ll_sdio.h"

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

// #define SDIO_BUFF_FOR_RECV_USE_MEMCPY
#define SDIO_SLAVE_MAX_RECV_SIZE (1536U)

typedef int (*receive_from_host_cb_t)(uint8_t *addr, int len);
typedef uint8_t *(*pkt_buf_malloc_cb_t)(uint16_t size);
typedef struct
{
    receive_from_host_cb_t    receive_from_host_cb;
    pkt_buf_malloc_cb_t       pkt_buf_malloc_cb;
}host_ops_t;

typedef struct
{
    OS_Mutex_t          lock;
    OS_Semaphore_t      sdio_sem;
    bool                fn1_en;
    host_ops_t          host_ops;
    uint8_t             sdio_cis_fn0[128];
    uint8_t             sdio_cis_fn1[128];
    uint8_t             rx_buffer[1600];
}hal_sdio_ctrl_t;

void hal_sdio_xfer_to_host(uint8_t *addr, int len);
void hal_sdio_init(sdio_config_t *config, host_ops_t *sdio_ops);
void hal_sdio_reset(void);

void sdio_if_init(void *wlib_recv_from_sdio, void **wlib_send_to_sdio, void *pkt_buf_malloc);
void sdio_if_reset(void);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __HAL_SDIO_H__

