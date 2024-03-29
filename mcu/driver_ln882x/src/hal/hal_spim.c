#include "hal/hal_spim.h"
#include "ll/ll_spim.h"
#include "hal/hal_common.h"

void HAL_SPIM_Init(uint32_t src_clk, SPIM_InitTypeDef config)
{
    LL_SPIM_Enable(SSI_Disabled);
    LL_SPIM_CLK_Div(src_clk / config.speed);

    LL_SPIM_CtrlR0(config.dfs, config.cfs, 0, 0, 0, config.scpol, config.scph, config.format);
}


/*----------------------------------------------------------------------------*/
/* work mode: Transmit_and_Receive, EEPROM_Read, Transmit_Only, Receive_Only  */
/*----------------------------------------------------------------------------*/


void HAL_SPIM_SetMode_TXRX(SPIM_Slave_Id id)
{
    LL_SPIM_TransferMode_Set(Transmit_and_Receive);
    LL_SPIM_Enable(SSI_Enabled);
    LL_SPIM_Slave_Select(0);
    LL_SPIM_Slave_Select(id);
}

void HAL_SPIM_SetMode_EEPROMRead(SPIM_Slave_Id id, uint16_t rd_len)
{
    LL_SPIM_TransferMode_Set(EEPROM_Read);
    LL_SPIM_CtrlR1(rd_len - 1);
    LL_SPIM_Enable(SSI_Enabled);
    LL_SPIM_Slave_Select(0);
    LL_SPIM_Slave_Select(id);
}

void HAL_SPIM_SetMode_WriteOnly(SPIM_Slave_Id id)
{
    LL_SPIM_TransferMode_Set(Transmit_Only);
    LL_SPIM_Enable(SSI_Enabled);
    LL_SPIM_Slave_Select(0);
    LL_SPIM_Slave_Select(id);
}

void HAL_SPIM_SetMode_ReadOnly(SPIM_Slave_Id id, uint16_t rd_len, uint32_t dummy_data)
{
    LL_SPIM_TransferMode_Set(Receive_Only);
    LL_SPIM_CtrlR1(rd_len - 1);
    LL_SPIM_Enable(SSI_Enabled);

    LL_SPIM_Slave_Select(0);
    LL_SPIM_Slave_Select(id);

    LL_SPIM_DataReg_Set(dummy_data);
}


/*----------------------------------------------------------------------------*/
/*-------------------------  do TX/RX in polling mode  -----------------------*/
/*----------------------------------------------------------------------------*/


void HAL_SPIM_Read_Register_Polling(SPIM_Slave_Id id, uint32_t *wr_ptr, uint16_t wr_len, uint32_t *rd_ptr, uint16_t rd_len)
{
    uint16_t i;

    LL_SPIM_TransferMode_Set(EEPROM_Read);
    LL_SPIM_CtrlR1(rd_len - 1);
    LL_SPIM_Enable(SSI_Enabled);
    LL_SPIM_Slave_Select(0);

    if(wr_len <= TX_FIFO_DEPTH) {
        for( i = 0; i < wr_len; i++) {
            LL_SPIM_DataReg_Set(*wr_ptr++);
        }
        LL_SPIM_Slave_Select(id);
    } else {
        for( i = 0; i < TX_FIFO_DEPTH; i++) {
            LL_SPIM_DataReg_Set(*wr_ptr++);
        }

        LL_SPIM_Slave_Select(id);

        for( i = TX_FIFO_DEPTH; i < wr_len; i++) {
            while( LL_SPIM_Is_TxFIFONotFull() == Transmit_FIFO_Full);
            LL_SPIM_DataReg_Set(*wr_ptr++);
        }
    }

    while( LL_SPIM_Is_TxFIFOEmpty() != Transmit_FIFO_Empty);

    for(i = 0; i < rd_len; i++) {
        while( LL_SPIM_Is_RxFIFONotEmpty() == Receive_FIFO_Empty );
        *rd_ptr++ = LL_SPIM_DataReg_Get();
    }
    while( LL_SPIM_Is_Busy() == SSI_Busy);
    LL_SPIM_Enable(SSI_Disabled);
}

void HAL_SPIM_Read_Only_Polling(SPIM_Slave_Id id, uint32_t * rd_ptr, uint16_t rd_len)
{
    uint16_t i;

    LL_SPIM_TransferMode_Set(Receive_Only);
    LL_SPIM_CtrlR1(rd_len - 1);
    LL_SPIM_Enable(SSI_Enabled);

    LL_SPIM_Slave_Select(0);
    LL_SPIM_Slave_Select(id);

    LL_SPIM_DataReg_Set(0xffffffff);

    for(i = 0; i < rd_len; i++) {
        while( LL_SPIM_Is_RxFIFONotEmpty() == Receive_FIFO_Empty );
        *rd_ptr++ = LL_SPIM_DataReg_Get();
    }

    while( LL_SPIM_Is_Busy() == SSI_Busy );
    LL_SPIM_Enable(SSI_Disabled);
}

void HAL_SPIM_Write_Polling(SPIM_Slave_Id id, uint32_t * wr_ptr, uint16_t wr_len)
{
    uint16_t i;

    LL_SPIM_Slave_Select(0);

    if(wr_len <= TX_FIFO_DEPTH) {
        for( i = 0; i < wr_len; i++) {
            LL_SPIM_DataReg_Set(*wr_ptr++);
        }
        LL_SPIM_Slave_Select(id);
    } else {
        for( i = 0; i < TX_FIFO_DEPTH; i++) {
            LL_SPIM_DataReg_Set(*wr_ptr++);
        }
        LL_SPIM_Slave_Select(id);
        for( i = TX_FIFO_DEPTH; i < wr_len; i++) {
            while( LL_SPIM_Is_TxFIFONotFull() == Transmit_FIFO_Full );
            LL_SPIM_DataReg_Set(*wr_ptr++);
        }
    }
    while( LL_SPIM_Is_Busy() == SSI_Busy );
    LL_SPIM_Enable(SSI_Disabled);
}


/*----------------------------------------------------------------------------*/
/*------------------------------  FIFO Operations  ---------------------------*/
/*----------------------------------------------------------------------------*/


/**
 * @brief Write one data to TX FIFO.
 *
 * @param data
 */
void HAL_SPIM_WriteOneData(uint32_t data)
{
    LL_SPIM_DataReg_Set(data);
}

/**
 * @brief Read one data from RX FIFO.
 *
 * @return uint32_t
 */
uint32_t HAL_SPIM_ReadOneData()
{
    return LL_SPIM_DataReg_Get();
}

void HAL_SPIM_Read_RxFIFO(uint32_t * rd_ptr, uint16_t rd_len)
{
    uint16_t i;
    for( i = 0; i < rd_len; i++) {
        while( LL_SPIM_Is_RxFIFONotEmpty() == Receive_FIFO_Empty );
        *rd_ptr++ = LL_SPIM_DataReg_Get();
    }
}

void HAL_SPIM_Write_TxFIFO(uint32_t * wr_ptr, uint16_t wr_len)
{
    uint16_t i;
    for( i = 0; i < wr_len; i++) {
        while( LL_SPIM_Is_TxFIFONotFull() == Transmit_FIFO_Full );
        LL_SPIM_DataReg_Set(*wr_ptr++);
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------   basic operations  ---------------------------*/
/*----------------------------------------------------------------------------*/

/**
 * @brief Enable/Disable the SPIM controller.
 *
 * @param en 1--enable; 0--disable.
 */
void HAL_SPIM_Enable(SPI_En en)
{
    LL_SPIM_Enable(en);
}

/**
 * @brief Slave Select. Each bit in this register corresponds to a slave select
 * line from the SPIM controller. When a bit in this register is set(1), the
 * corresponding slave select line is activated when a serial transfer begins.
 * NOTE: It cannot be written when SPIM is busy.
 * @param id slave id.
 * Only one bit should be set.
 */
void HAL_SPIM_Slave_Select_En(SPIM_Slave_Id id)
{
    LL_SPIM_Slave_Select(id);
}

/**
 * @brief Clock Divider.
 * NOTE: Odd div is not allowed.
 * @param div
 */
void HAL_SPIM_CLK_Div(uint16_t div)
{
    assert_param( (div >= 2) && (0 == (div % 2)) );

    LL_SPIM_CLK_Div(div);
}

/**
 * @brief Microwire Setting.
 *
 * @param handshake_en Used to enable/disable the "busy/ready" handshaking interface
 * for the Microwire protocol.
 * 0 -- handshaking disabled;
 * 1 -- handshaking enabled.
 * @param mode Microwire Control.
 * 0 -- receive data word from external serial device.
 * 1 -- Send data word to external serial device.
 * @param transfer_mode Microwrie Transfer Mode. Defines whether the Microwire transfer is
 * sequential or non-sequential.
 * 0 -- non-sequential transfer.
 * 1 -- sequential transfer.
 */
void HAL_SPIM_Microwire_Setting(uint8_t handshake_en, uint8_t mode, uint8_t transfer_mode)
{
    LL_SPIM_Microwire_Setting(handshake_en, mode, transfer_mode);
}

/**
 * @brief
 *
 * @param dfs Data Frame Size in 32-bit mode.
 * @param cfs Control Frame Size.
 * @param srl Shift Register Loop.
 * @param slv_oe NOT usued.
 * @param tmod Transfer mode. Only indicates whether the receive or transmit data are valid.
 * 00 -- Transmit & Receive
 * 01 -- Transmit Only
 * 10 -- Receive Only
 * 11 -- EEPROM Read
 * @param scpol Serial Clock Polarity.
 * 0 -- Inactive state of serial clock is low.
 * 1 -- Inactive state of serial clock is high.
 * @param scph Serial Clock Phase.
 * 0 -- Serial clock toggles in middle of first data bit.
 * 1 -- Serial clock toggles at start of first data bit.
 * @param frf Frame Format. Selects which serial protocol transfers the data.
 * 00 -- Motorola SPI
 * 01 -- Texas Instruments SSP
 * 10 -- National Semiconductors Microwire
 * 11 -- Reserved
 */
void HAL_SPIM_Ctrl0(uint8_t dfs,uint8_t cfs,uint8_t srl,uint8_t slv_oe,uint8_t tmod,uint8_t scpol,uint8_t scph,uint8_t frf)
{
    LL_SPIM_CtrlR0(dfs, cfs, srl, slv_oe, tmod, scpol, scph, frf);
}

/**
 * @brief
 *
 * @param ndf Number of Data Frames. When TMOD = 10 or TMOD = 11, this register
 * field sets the number of data frames to be continuously received by the SPIM
 * controller.
 *
 * Indicates this module continues to receive serial data until the number of
 * data frames received is equal to this register value plus 1.
 */
void HAL_SPIM_Ctrl1(uint16_t ndf)
{
    LL_SPIM_CtrlR1(ndf-1);
}

void HAL_SPIM_Mode_Set(SPI_Transmit_Mode mode)
{
    LL_SPIM_TransferMode_Set(mode);
}

/**
 * @brief Serial Clock Polarity. Valid when the FRF is set to Motorola SPI.
 *
 * @param scpol
 */
void HAL_SPIM_ClkPolarity_Set(SPI_Clock_Polarity scpol)
{
    LL_SPIM_ClkPolarity_Set(scpol);
}

/**
 * @brief Serial Clock Phase. Valid when the FRF is set to Motorola SPI.
 *
 * @param scph
 */
void HAL_SPIM_ClkPhase_Set(SPI_Clock_Phase scph)
{
    LL_SPIM_ClkPhase_Set(scph);
}

/**
 * @brief Frame Format. Selects which serial protocol transfers the data.
 *
 * @param frf
 */
void HAL_SPIM_FrameFormat_Set(SPI_Protocol_Type frf)
{
    LL_SPIM_FrameFormat_Set(frf);
}

uint32_t* HAL_SPIM_DataRegAddr_Get(void)
{
    return LL_SPIM_DataRegAddr_Get();
}

void HAL_SPIM_Set_Read_Len(uint16_t rd_len)
{
    LL_SPIM_CtrlR1(rd_len - 1);
}

void HAL_SPIM_RxSample_Dly(uint8_t dly)
{
    LL_SPIM_RxSampleDly_Set(dly);
}

/**
 * @brief Configurae TX/RX DMA and data level.
 *
 * @param tx_dma_en
 * @param rx_dma_en
 * @param tx_data_level
 * @param rx_data_level
 */
void HAL_SPIM_Set_DMA(uint8_t tx_dma_en, uint8_t rx_dma_en, uint8_t tx_data_level, uint8_t rx_data_level)
{
    LL_SPIM_DAM_DataLvl_Set(tx_data_level, rx_data_level);
    LL_SPIM_DMA_Ctrl(tx_dma_en, rx_dma_en);
}


/*----------------------------------------------------------------------------*/
/*------------------------------   Interrupt APIs  ---------------------------*/
/*----------------------------------------------------------------------------*/


/**
 * @brief Each bit represents an interrupt mask, write 0 to disable interrupt.
 * Bit 5 -- Multi-Master Contention Interrupt Mask
 * Bit 4 -- Receive FIFO Full Interrupt Mask
 * Bit 3 -- Receive FIFO Overflow Interrupt Mask
 * Bit 2 -- Receive FIFO Underflow Interrupt Mask
 * Bit 1 -- Transmit FIFO Overflow Interrupt Mask
 * Bit 0 -- Transmit FIFO Empty Interrupt Mask
 *
 * One bit for one interrupt mask:
 * 0 -- masked (disabled).
 * 1 -- not masked (enabled).
 *
 * see `SPIM_Int_Mask`
 */
void HAL_SPIM_Set_Mask(uint8_t mask)
{
    LL_SPIM_IntMask_Set(mask);
}

/**
 * @brief
 *
 * @return uint8_t see `SPIM_Int_Mask`
 */
uint8_t HAL_SPIM_Get_Mask()
{
    return LL_SPIM_IntMask_Get();
}

/**
 * @brief Raw Interrupt Status.
 * This read-only register reports the status of SPI master interrupts prior to masking.
 * Bit 5 -- Multi-Master Contension Raw Interrupt Status;
 * Bit 4 -- Receive FIFO Full Raw Interrupt Status;
 * Bit 3 -- Receive FIFO Overflow Raw Interrupt Status;
 * Bit 2 -- Receive FIFO Underflow Raw Interrupt Status;
 * Bit 1 -- Transmit FIFO Overflow Raw Interrupt Status;
 * Bit 0 -- Transmit FIFO Empty Raw Interrupt Status;
 *
 * One bit for one raw interrupt status:
 * 0 -- not active;
 * 1 -- active;
 * @return uint8_t see `SPIM_Int_Mask`
 */
uint8_t HAL_SPIM_RawInt_Status()
{
    return LL_SPIM_RawInt_Status();
}

/**
 * @brief Interrupt Status after masked.
 *
 * @return uint8_t one bit for one interrupt.
 * bit5 -- mstis, Multi-Master Contention Interrupt Status
 * bit4 -- rxfis, Receive FIFO Full Interrupt Status
 * bit3 -- rxois, Receive FIFO Overflow Interrupt Status
 * bit2 -- rxuis, Receive FIFO Underflow Interrupt Status
 * bit1 -- txois, Transmit FIFO Overflow Interrupt Status
 * bit0 -- txeis, Transmit FIFO Empty Interrupt Status
 *
 * See `SPIM_Int_Status`
 */
uint8_t HAL_SPIM_Int_Status()
{
    return LL_SPIM_Int_Status();
}

/**
 * @brief Clear Interrupts (txo, rxu, rxo, mst).
 *
 * @return uint8_t
 */
uint8_t HAL_SPIM_IntClr()
{
    return LL_SPIM_IntClr_All();
}

/**
 * @brief Clear Multi-Master Contention Interrupt and return the interrupt status.
 *
 * @return uint8_t
 */
uint8_t HAL_SPIM_IntClr_MST()
{
    return LL_SPIM_IntClr_MultiMaster();
}

/**
 * @brief Clear Receive FIFO Underflow Interrupt and return the interrupt status.
 *
 * @return uint8_t
 */
uint8_t HAL_SPIM_IntClr_RXU()
{
    return LL_SPIM_IntClr_RxFIFOUnderflow();
}

/**
 * @brief Clear Receive FIFO Overflow Interrupt and return the interrupt status.
 *
 * @return uint8_t
 */
uint8_t HAL_SPIM_IntClr_RXO()
{
    return LL_SPIM_IntClr_RxFIFOOverflow();
}

/**
 * @brief Clear Transmit FIFO Overflow Interrupt, and return the interrupt status.
 *
 * @return uint8_t
 */
uint8_t HAL_SPIM_IntClr_TXO()
{
    return LL_SPIM_IntClr_TxFIFOOverflow();
}


/*----------------------------------------------------------------------------*/
/*-------------------------------  status API  -------------------------------*/
/*----------------------------------------------------------------------------*/


/**
 * @brief SPIM status. Contains the following status:
 * Bit 0 -- Busy, indicates that a serial transfer is in progress;
 * Bit 1 -- tfnf, Transmit FIFO Not Full;
 * Bit 2 -- tfe, Transmit FIFO Empty;
 * Bit 3 -- rfne, Receive FIFO Not Empty;
 * Bit 4 -- rff, Receive FIFO Full;
 * Bit 5 -- txe, Transmission Error;
 * Bit 6 -- dcol, Data Collision Error;
 *
 * @return uint8_t see `SPIM_Normal_Status`
 */
uint8_t HAL_SPIM_Normal_Status()
{
    return LL_SPIM_Normal_Status();
}

bool HAL_SPIM_Is_Busy()
{
    return LL_SPIM_Is_Busy();
}

/**
 * @brief Transmit FIFO Level. Contains the number of valid data entries in the TX FIFO.
 *
 * @return uint8_t
 */
uint8_t HAL_SPIM_Current_TxFIFO_Level()
{
    return LL_SPIM_Current_TxFIFO_Level();
}

/**
 * @brief Receive FIFO Level. Contains the number of valid data entries in the RX FIFO.
 *
 * @return uint8_t
 */
uint8_t HAL_SPIM_Current_RxFIFO_Level()
{
    return LL_SPIM_Current_RxFIFO_Level();
}

/**
 * @brief Transmit FIFO Threshold. Controls the level of entries (or below) at
 * which the transmit FIFO controller triggers an interrupt - TX FIFO Empty INT.
 *
 * @param tx_thd valid range is 0 ~ (TX FIFO Depth-1), that is [0, 16).
 */
void HAL_SPIM_TX_FIFO_Threshold_Set(uint8_t tx_thd)
{
    LL_SPIM_TX_FIFO_Threshold_Set(tx_thd);
}

/**
 * @brief Receive FIFO Threshold. Controls the level of entries (or above) at
 * which the receive FIFO controller triggers an interrupt - RX FIFO Full INT.
 *
 * @param rx_thd valid range is 0 ~ (RX FIFO Depth-1), that is [0, 16).
 */
void HAL_SPIM_RX_FIFO_Threshold_Set(uint8_t rx_thd)
{
    LL_SPIM_RX_FIFO_Threshold_Set(rx_thd);
}

/**
 * @brief Configure Transmit/Receive FIFO threshold.
 * @param tx_thd
 * @param rx_thd
 */
void HAL_SPIM_Set_FIFO_Threshold(uint8_t tx_thd, uint8_t rx_thd)
{
    LL_SPIM_FIFO_Threshold_Set(tx_thd, rx_thd);
}
