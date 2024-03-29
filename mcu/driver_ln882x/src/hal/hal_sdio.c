#include "string.h"
#include "ln88xx.h"
#include "hal/hal_sdio.h"
#include "hal/hal_syscon.h"
#include "utils/debug/log.h"

static hal_sdio_ctrl_t g_hal_sdio_ctrl;
static hal_sdio_ctrl_t *hal_sdio_ctrl_get_handle(void)
{
    return &g_hal_sdio_ctrl;
}

/*
 * ruturn the value of powers 2:
 * eg:  1 --> 4
 *      3 --> 4
 *      33 --> 64
 *      511 --> 512
 *      513 --> 1024
 *      1025 --> 1536
 * max value is 1536
 */
static int power_of_2(int v)
{
    int res = 1;
    if(v < SDIO_FUNC_BLOCK_SIZE){
	    while (res < v)
	        res <<= 1;
	    if(res < 4)
	        res = 4;
    }else if(v <= 3*SDIO_FUNC_BLOCK_SIZE){
    	if(v % SDIO_FUNC_BLOCK_SIZE)
            res = (v / SDIO_FUNC_BLOCK_SIZE + 1) * SDIO_FUNC_BLOCK_SIZE;
        else
    		res = v;
    }else{
        LOG(LOG_LVL_ERROR, "[%s, %d]SDIO XFER data length is overfllow(len=%d), and truncated.\r\n", __func__, __LINE__, v);
        res = 3*SDIO_FUNC_BLOCK_SIZE;
    }
    
    return res;
}

static void hal_sdio_receive_from_host(void)
{
    hal_sdio_ctrl_t *hal_sdio_ctrl = NULL;
    host_ops_t      *host_ops = NULL;
    uint8_t *buff = NULL;
    uint16_t data_len = 0;
    uint8_t *src = (uint8_t *)ll_sdio_receive_from_host_buffer_get();
    uint16_t count = ll_sdio_receive_from_host_buffer_size_get();

    hal_sdio_ctrl = hal_sdio_ctrl_get_handle();
    if (hal_sdio_ctrl == NULL) {
        return;
    }

    host_ops = &(hal_sdio_ctrl->host_ops);
    if (host_ops == NULL) {
        return;
    }
    if (host_ops->pkt_buf_malloc_cb == NULL) {
        return;
    }

    if (src == NULL) {
        LOG(LOG_LVL_ERROR, "Error src, should not be null\r\n");
        ll_sdio_receive_from_host_buffer_set(hal_sdio_ctrl->rx_buffer);
        return;
    }

    data_len = src[0] | (src[1]&0x0F)<<8;
    if(!(data_len <= count && count <= 3*SDIO_FUNC_BLOCK_SIZE)) {
        LOG(LOG_LVL_ERROR, "[%s, %d]SDIO receive fail. count=%d, real_count=%d(free addr=%p)\r\n", 
                __func__, __LINE__, count, data_len, src);
        //Continue to use this buffer
        ll_sdio_receive_from_host_buffer_set(src);
        return;
    }

#ifdef SDIO_BUFF_FOR_RECV_USE_MEMCPY
    buff = host_ops->pkt_buf_malloc_cb(count);
    if (buff == NULL) {
        // LOG(LOG_LVL_ERROR, "-");
        ll_sdio_receive_from_host_buffer_set(hal_sdio_ctrl->rx_buffer);
        return;
    }
    memcpy(buff, src, count);
    ll_sdio_receive_from_host_buffer_set(hal_sdio_ctrl->rx_buffer);
    // LOG(LOG_LVL_ERROR, "%d\r\n", count);
    host_ops->receive_from_host_cb(buff, count);
#else
    if (src != hal_sdio_ctrl->rx_buffer) {
        // LOG(LOG_LVL_ERROR, "%d\r\n", count);
        host_ops->receive_from_host_cb(src, count);
    }

    buff = host_ops->pkt_buf_malloc_cb(SDIO_SLAVE_MAX_RECV_SIZE);
    if (buff != NULL) {
        ll_sdio_receive_from_host_buffer_set(buff);
    } else {
        // LOG(LOG_LVL_ERROR, "-");
        ll_sdio_receive_from_host_buffer_set(hal_sdio_ctrl->rx_buffer);
    }
#endif /* SDIO_BUFF_FOR_RECV_USE_MEMCPY */
}

void hal_sdio_xfer_to_host(uint8_t *addr, int len)
{
    uint32_t size = 0;
    hal_sdio_ctrl_t *hal_sdio_ctrl = hal_sdio_ctrl_get_handle();

    OS_MutexLock(&hal_sdio_ctrl->lock, OS_WAIT_FOREVER);
    if(hal_sdio_ctrl->fn1_en){
        //hexdump(LOG_LVL_ERROR, "To HOST", addr, len);
        size = power_of_2(len);
        if (size > SDIO_SLAVE_MAX_RECV_SIZE) {
            LOG(LOG_LVL_ERROR, "send to host size:%d is too long\r\n", size);
        }
        ll_sdio_xfer_to_host_buffer_set(addr);
        ll_sdio_xfer_to_host_buffer_size_set(size);
        ll_sdio_triggle_data1_interrupt_to_host();
        OS_SemaphoreWait(&hal_sdio_ctrl->sdio_sem, OS_WAIT_FOREVER);
    }
    OS_MutexUnlock(&hal_sdio_ctrl->lock);
}

void hal_sdio_init(sdio_config_t *config, host_ops_t *sdio_ops)
{
    hal_sdio_ctrl_t *hal_sdio_ctrl = hal_sdio_ctrl_get_handle();
    if (hal_sdio_ctrl == NULL || sdio_ops == NULL) {
        return;
    }

    if (OS_OK != OS_SemaphoreCreateBinary(&hal_sdio_ctrl->sdio_sem)){
		LOG(LOG_LVL_ERROR, "[%s, %d]OS_SemaphoreCreateBinary hal_sdio_ctrl->lock fail.\r\n", __func__, __LINE__);
        return;
    }
    OS_MutexCreate(&hal_sdio_ctrl->lock);

    hal_sdio_ctrl->host_ops.receive_from_host_cb = sdio_ops->receive_from_host_cb;
    hal_sdio_ctrl->host_ops.pkt_buf_malloc_cb = sdio_ops->pkt_buf_malloc_cb;

    config->cis_fn0_base = hal_sdio_ctrl->sdio_cis_fn0;
    config->cis_fn1_base = hal_sdio_ctrl->sdio_cis_fn1;

#ifdef SDIO_BUFF_FOR_RECV_USE_MEMCPY
    config->from_host_buffer = hal_sdio_ctrl->rx_buffer;
#else
    uint8_t *buffer = NULL;
    if(sdio_ops->pkt_buf_malloc_cb){
        buffer = sdio_ops->pkt_buf_malloc_cb(SDIO_SLAVE_MAX_RECV_SIZE);
    }
    if (buffer != NULL) {
        config->from_host_buffer = buffer;
    } else {
        config->from_host_buffer = hal_sdio_ctrl->rx_buffer;
    }
#endif
    ll_sdio_init(config);
}

void hal_sdio_reset(void){
    hal_sdio_ctrl_t *hal_sdio_ctrl = hal_sdio_ctrl_get_handle();
    host_ops_t      *host_ops = &(hal_sdio_ctrl->host_ops);
    uint8_t *buffer = NULL;

#ifdef SDIO_BUFF_FOR_RECV_USE_MEMCPY
    ll_sdio_receive_from_host_buffer_set(hal_sdio_ctrl->rx_buffer);
#else
    if(host_ops->pkt_buf_malloc_cb){
        buffer = host_ops->pkt_buf_malloc_cb(SDIO_SLAVE_MAX_RECV_SIZE);
    }
    if (buffer != NULL) {
        ll_sdio_receive_from_host_buffer_set(buffer);
    } else {
        ll_sdio_receive_from_host_buffer_set(hal_sdio_ctrl->rx_buffer);
    }
#endif
}

void SDIO_FUN1_IRQHandler(void)
{
    hal_sdio_ctrl_t *hal_sdio_ctrl = hal_sdio_ctrl_get_handle();
    uint32_t int_status = ll_sdio_get_interrupt_status() & 0xfff;
    
    //LOG(LOG_LVL_ERROR, "[%s, %d]int_status=0x%08X\r\n", __func__, __LINE__, int_status);
    if(int_status & FN1_WRITE_OVER_INTERRPT) { //HOST write over interrupt
        hal_sdio_receive_from_host();
        //clear this IRQ bit
        ll_sdio_set_interrupt_status(int_status&(~FN1_WRITE_OVER_INTERRPT));
        //clear busy SD
        ll_sdio_clear_busy_sd();
    }
    if(int_status & FN1_READ_OVER_INTERRPT) {
        //clear this IRQ bit
        ll_sdio_set_interrupt_status(int_status&(~FN1_READ_OVER_INTERRPT));
        if(hal_sdio_ctrl->fn1_en){
            OS_SemaphoreRelease(&hal_sdio_ctrl->sdio_sem);
        }
    }
    if(int_status & READ_ERROR_FN1_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~READ_ERROR_FN1_INTERRPT));
    }
    if(int_status & WRITE_ERROR_FN1_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~WRITE_ERROR_FN1_INTERRPT));
    }
    if(int_status & WRITE_ABORT_FN1_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~WRITE_ABORT_FN1_INTERRPT));
    }
    if(int_status & RESET_FN1_INTERRPT) {
    	hal_sdio_ctrl->fn1_en = false;
		ll_sdio_set_interrupt_status(int_status&(~RESET_FN1_INTERRPT));
    }
    if(int_status & FN1_ENABLE_INTERRPT) {
        hal_sdio_ctrl->fn1_en = true;
        ll_sdio_set_interrupt_status(int_status&(~FN1_ENABLE_INTERRPT));
    }
    if(int_status & FN1_STATUS_PCRRT_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~FN1_STATUS_PCRRT_INTERRPT));
    }
    if(int_status & FN1_STATUS_PCWRT_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~FN1_STATUS_PCWRT_INTERRPT));
    }
    if(int_status & FN1_RTC_SET_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~FN1_RTC_SET_INTERRPT));
    }
    if(int_status & FN1_CLINTRD_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~FN1_CLINTRD_INTERRPT));
    }
    if(int_status & FN1_INT_EN_UP_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~FN1_INT_EN_UP_INTERRPT));
    }
    if(int_status & FN1_M2S_INT_INTERRPT) {
        ll_sdio_set_interrupt_status(int_status&(~FN1_M2S_INT_INTERRPT));
    }
}


void sdio_if_init(void *wlib_recv_from_sdio, void **wlib_send_to_sdio, void *pkt_buf_malloc)
{
    sdio_config_t config = {0,};
    host_ops_t    sdio_host_ops;
    
    sdio_host_ops.receive_from_host_cb = (receive_from_host_cb_t)wlib_recv_from_sdio;
    sdio_host_ops.pkt_buf_malloc_cb    = (pkt_buf_malloc_cb_t)pkt_buf_malloc;
    
    //Enable SDIO IO
    HAL_SYSCON_SDIOEnable();
    
    //Enable SDIO IO
    HAL_SYSCON_SDIOEnable();
    HAL_SYSCON_GPIO_PullUp(GPIOA_6); /* D2 */
    HAL_SYSCON_GPIO_PullUp(GPIOA_7); /* D3 */
    HAL_SYSCON_GPIO_PullUp(GPIOA_8); /* CMD */
    //HAL_SYSCON_GPIO_NoPull(GPIOA_8);
    //HAL_SYSCON_GPIO_PullUp(GPIOA_9); /* CLK PULL UP */
    HAL_SYSCON_GPIO_NoPull(GPIOA_9); /* CLK */
    HAL_SYSCON_GPIO_PullUp(GPIOA_10); /* D0 */
    HAL_SYSCON_GPIO_PullUp(GPIOA_11); /* D1 */

    //SDIO Config setting
    config.supp_func_num = 1;
    config.clr_busy_sd = CLEAR_BUSY_SD;
    config.csa_support = FN1_CSA_SUPPORT
						| FN2_CSA_SUPPORT
						| FN3_CSA_SUPPORT
						| FN4_CSA_SUPPORT
						| FN5_CSA_SUPPORT
						| FN6_CSA_SUPPORT
						| FN7_CSA_SUPPORT;
    config.supp_high_speed = Supp_High_Speed_Enabled;
    config.card_cap_sd = SDIO_CCCR_CAP_SCSI
                        | SDIO_CCCR_CAP_SMB
						| SDIO_CCCR_CAP_SDC
						| SDIO_CCCR_CAP_SRW
						| SDIO_CCCR_CAP_SBS
						| SDIO_CCCR_CAP_4BLS;
    config.int_en_ctrl.bit_field.fn1_m2s_int = Fn1_M2s_Int_Interrupt_Disabled;
    config.int_en_ctrl.bit_field.fn1_int_en_up = Fn1_Int_En_Up_Interrupt_Disabled;
    config.int_en_ctrl.bit_field.fn1_clintrd = Fn1_Clintrd_Interrupt_Disabled;
    config.int_en_ctrl.bit_field.fn1_rtc_set = Fn1_Rtc_Set_Interrupt_Disabled;
    config.int_en_ctrl.bit_field.fn1_status_pcwrt = Fn1_Status_Pcwrt_Interrupt_Disabled;
    config.int_en_ctrl.bit_field.fn1_status_pcrrt = Fn1_Status_Pcrrt_Interrupt_Disabled;
    config.int_en_ctrl.bit_field.fn1_en = Fn1_Enable_Interrupt_Enabled;
    config.int_en_ctrl.bit_field.rst_fn1 = Reset_Fn1_Interrupt_Enabled;
    config.int_en_ctrl.bit_field.wr_abort_fn1 = Write_Abort_Fn1_Interrupt_Disabled;
    config.int_en_ctrl.bit_field.wr_err_fn1 = Write_Error_Fn1_Interrupt_Enabled;
    config.int_en_ctrl.bit_field.rd_err_fn1 = Read_Error_Fn1_Interrupt_Enabled;
    config.int_en_ctrl.bit_field.fn1_rd_ovr = Fn1_Read_Over_Interrupt_Enabled;
    config.int_en_ctrl.bit_field.fn1_wr_ovr = Fn1_Write_Over_Interrupt_Enabled;

    *wlib_send_to_sdio = hal_sdio_xfer_to_host;
    
    hal_sdio_init(&config, &sdio_host_ops);

    NVIC_EnableIRQ(SDIO_FUN1_IRQn);
}

void sdio_if_reset(void)
{
    hal_sdio_reset();
}


