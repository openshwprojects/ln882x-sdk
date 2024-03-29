#include "hal/hal_dma.h"
#include "osal/osal.h"
//#include "ll/ll_uart.h"
#include "serial_hw.h"
#include "hal/syscon_types.h"
#include "hal/hal_sleep.h"


static OS_Thread_t g_uart_demo_thread;
#define UART_DEMO_TASK_STACK_SIZE           (6*256) //Byte

//if need rx and tx work at the same time,then use different dma channel.
//Normally,use the same dma channel is ok
#define UART_TX_DMA_CHANNEL     DMA_CHANNEL_0
#define UART_RX_DMA_CHANNEL     DMA_CHANNEL_1//DMA_CHANNEL_0

#define UART_DMA_TEST_WRITE       1
#define UART_DMA_TEST_READ_WRITE  2
#define TEST_CASE                 UART_DMA_TEST_READ_WRITE


uint8_t uart1_dma_write_buffer[256]={0};
uint8_t uart1_dma_read_buffer[256]={0};

static void uart_io_pin_Init(SerialPortID port_id)
{
    int en = 1;

    if(port_id == SER_PORT_UART0){
        HAL_SYSCON_FuncIOSet(GPIO_AF_UART0_RX, GPIO_AF_IO_18, en); //LN882x: GPIO_A[8], FULL_MUX_18, PAD24 [rom_uart0 RX]
        HAL_SYSCON_FuncIOSet(GPIO_AF_UART0_TX, GPIO_AF_IO_19, en); //LN882x: GPIO_A[9], FULL_MUX_19, PAD25 [rom_uart0 TX]
    }else if(port_id == SER_PORT_UART1){
        HAL_SYSCON_FuncIOSet(GPIO_AF_UART1_RX, GPIO_AF_IO_16, en);//LN8820 (LN8825) &BLE EVB: GPIOB6, FULL_MUX_16, Pin31
        HAL_SYSCON_FuncIOSet(GPIO_AF_UART1_TX, GPIO_AF_IO_17, en);//LN8820 (LN8825) &BLE EVB: GPIOB7, FULL_MUX_17, Pin32
    }
}

/***********************************************************
*  Function: uart_init
*  Input: uart
*  Output: none
*  Return: 0-succ,other-fail
***********************************************************/
int uart_init(void)
{
    UART_DevTypeDef l_huart1;
    SerialPortID port_id = SER_PORT_UART1;

    l_huart1.Instance           = UART1;
    l_huart1.Config.BaudRate    = 115200;//115200, 921600
    l_huart1.Config.DataLength  = UART_DATALEN_8BIT;
    l_huart1.Config.Parity      = UART_PARITY_NONE;
    l_huart1.Config.StopBits    = UART_STOP_BIT_1;
    l_huart1.Config.FlowControl = UART_FLOW_CONTROL_SOFTWARE;

    //request pin for uart
    uart_io_pin_Init(port_id);

    //init uart hardware
    HAL_UART_Init(&l_huart1);

    //config tx/rx trigger
    HAL_UART_FIFOControl(&l_huart1, UART_TX_EMPTY_TRIGGER_FIFO_EMPTY, UART_RCVR_TRIGGER_FIFO_HAS_ONE_CHARACTER, UART_DMA_MODE0);
    //HAL_UART_FIFOControl(&l_huart1, UART_TX_EMPTY_TRIGGER_FIFO_EMPTY, UART_RCVR_TRIGGER_FIFO_HALF_FULL, UART_DMA_MODE0);

    hal_sleep_register(MOD_UART1, NULL, NULL, NULL);


}

void uart_dma_read_byte(uint8_t *rd_ptr, uint32_t rd_len_in_word)
{
    DMA_InitTypeDef config;

    config.device = DMA_DEVICE_UART1_RX;
    config.type = DMA_TRANS_TYPE_PERIPHERAL_TO_MEMORY_DMAC_FLOW_CONTROLLER;
    config.msize = DMA_BURST_TRANSACTION_LENGTH_8;
    config.src_inc = DMA_ADDRESS_UNCHANGE;
    config.dst_inc = DMA_ADDRESS_INCREMENT;
    config.width = DMA_TRANSFER_WIDTH_8_BITS;//DMA_TRANSFER_WIDTH_32_BITS;
    config.int_en = DMA_INTERRUPT_ENABLE;

    HAL_DMA_Config(UART_RX_DMA_CHANNEL, &config);

    HAL_DMA_StartTransfer(UART_RX_DMA_CHANNEL, (void *)REG_UART1_BASE, rd_ptr, rd_len_in_word);
    HAL_DMA_WaitDone(UART_RX_DMA_CHANNEL);


}
/**
 * @brief Write data via Standard SPI format.
 *
 * @param bufptr
 * @param length
 */
void uart_dma_write(uint8_t *bufptr, uint32_t length)
{
    DMA_InitTypeDef config;
    config.device = DMA_DEVICE_UART1_TX;
    config.type = DMA_TRANS_TYPE_MEMORY_TO_PERIPHERAL_DMAC_FLOW_CONTROLLER;
    config.msize = DMA_BURST_TRANSACTION_LENGTH_8;
    config.src_inc = DMA_ADDRESS_INCREMENT;
    config.dst_inc = DMA_ADDRESS_UNCHANGE;
    config.width = DMA_TRANSFER_WIDTH_8_BITS;
    config.int_en = DMA_INTERRUPT_DISABLE;

    HAL_DMA_Config(UART_TX_DMA_CHANNEL, &config);

    HAL_DMA_StartTransfer(UART_TX_DMA_CHANNEL, bufptr, (void *)REG_UART1_BASE, length);
    HAL_DMA_WaitDone(UART_TX_DMA_CHANNEL);

}

void UART_DMA_transfer_init(void)
{
    uart_init();
    HAL_DMA_Init(UART_TX_DMA_CHANNEL, NULL);
    HAL_DMA_Init(UART_RX_DMA_CHANNEL, NULL);

}
void uart_dma_task_entry(void *params)
{
    uint16_t i=0;
    uint8_t  rx_ch;

    for(i=0;i<256;i++){
        uart1_dma_write_buffer[i]=i;
    }

    UART_DMA_transfer_init();//enable dma
    switch(TEST_CASE){
        case  UART_DMA_TEST_WRITE://uart dma write test
            while(1){
                for(i=1;i<256;i++){
                    uart_dma_write(uart1_dma_write_buffer,i);  //write to uart1
                    OS_MsDelay(500);
                }
            }
            break;

        case UART_DMA_TEST_READ_WRITE://uart dma read and write test
            memset(uart1_dma_read_buffer,0,256);
            i=0;
            while(1)
            {
                uart_dma_read_byte(&rx_ch,1);
                if(i<256){
                    uart1_dma_read_buffer[i++]=rx_ch;
                }else{
                    i=0;
                }
                if(rx_ch=='\n'){
                    #if 0// write to uart0 through cpu uart mode. verify ok
                        LOG(LOG_LVL_ERROR, "receiv:%s \r\n",uart1_dma_read_buffer);
                    #else//write to uart1 through DMA uart mode.
                        uart_dma_write(uart1_dma_read_buffer,i);
                    #endif
                    memset(uart1_dma_read_buffer,0,256);
                    i=0;
                }
            }
            break;
    }
}

void uart_dma_demo_task(void)
{
    if(OS_OK != OS_ThreadCreate(&g_uart_demo_thread, "UartDemo", uart_dma_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, UART_DEMO_TASK_STACK_SIZE)) {
        LOG(LOG_LVL_ERROR, "creat uart_dma_task error\r\n");
    }

}



