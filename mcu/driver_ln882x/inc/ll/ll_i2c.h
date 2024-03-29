#ifndef __LL_I2C_H__
#define __LL_I2C_H__

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "ln_types.h"
#include "ln88xx.h"

////////////////////////////////  Datatype and Macros  ////////////////////////
typedef void I2CInstance;
#define I2C0   ((I2CInstance *) REG_I2C0_BASE)
#define I2C1   ((I2CInstance *) REG_I2C1_BASE)


///////////////////////////////  Functioin Declarations ///////////////////////



///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CON, I2C Control Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_SlaveDisable_Set(I2CInstance * i2c_instance, uint8_t flag);

void LL_I2C_MasterMode_Set(I2CInstance * i2c_instance, uint8_t flag);

uint8_t LL_I2C_MasterMode_Get(I2CInstance * i2c_instance);

void LL_I2C_RestartMode_Enable(I2CInstance * i2c_instance, uint8_t en);

void LL_I2C_Set7_10Addressing_Master(I2CInstance * i2c_instance, uint8_t address);

void LL_I2C_Set7_10Addressing_Slave(I2CInstance * i2c_instance, uint8_t address);

void LL_I2C_SetSpeed(I2CInstance * i2c_instance, uint8_t speed_mode);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_TAR, I2C Target Address Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_SetSpecialBit(I2CInstance * i2c_instance, uint8_t special);

void LL_I2C_GeneralCall_Or_Start(I2CInstance *i2c_instance, uint8_t start);

void LL_I2C_TarAddr_Set(I2CInstance * i2c_instance, uint16_t addr);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_SAR, I2C Slave Address Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_SlaveAddr_Set(I2CInstance * i2c_instance, uint16_t addr);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_HS_MADDR, I2C High Speed Master Mode Code Address Reg
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_HighSpeed_MasterAddr_Set(I2CInstance * i2c_instance, uint8_t maddr);

uint8_t LL_I2C_HighSpeed_MasterCode_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_DATA_CMD, I2C Rx/Tx Data Buffer and Command Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_Data_Get(I2CInstance * i2c_instance);

void LL_I2C_Data_Cmd_Pack(I2CInstance * i2c_instance, uint8_t restart, uint8_t stop, uint8_t cmd, uint8_t dat);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_SS_SCL_HCNT, Standard Speed I2C Clock SCL High Count
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_StandardSpeed_SCL_HighCount_Set(I2CInstance * i2c_instance, uint16_t hcnt);

uint16_t LL_I2C_StandardSpeed_SCL_HighCount_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_SS_SCL_LCNT, Standard Speed I2C Clock SCL Low Count
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_StandardSpeed_SCL_LowCount_Set(I2CInstance * i2c_instance, uint16_t lcnt);

uint16_t LL_I2C_StandardSpeed_SCL_LowCount_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_FS_SCL_HCNT, Fast Mode or Fast Mode Plus I2C Clock
////                                 SCL High Count Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_FastSpeed_SCL_HighCount_Set(I2CInstance * i2c_instance, uint16_t hcnt);

uint16_t LL_I2C_FastSpeed_SCL_HighCount_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_FS_SCL_LCNT, Fast Mode or Fast Mode Plus I2C Clock
////                                 SCL Low Count Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_FastSpeed_SCL_LowCount_Set(I2CInstance * i2c_instance, uint16_t lcnt);

uint16_t LL_I2C_FastSpeed_SCL_LowCount_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_HS_SCL_HCNT, High Speed I2C Clock SCL High Count Reg
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_HighSpeed_SCL_HighCount_Set(I2CInstance * i2c_instance, uint16_t hcnt);

uint16_t LL_I2C_HighSpeed_SCL_HighCount_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_HS_SCL_LCNT, High Speed I2C Clock SCL Low Count Reg
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_HighSpeed_SCL_LowCount_Set(I2CInstance * i2c_instance, uint16_t lcnt);

uint16_t LL_I2C_HighSpeed_SCL_LowCount_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_INTR_STAT, I2C Interrupt Status Register
///////////////////////////////////////////////////////////////////////////////

uint16_t LL_I2C_IntStatus(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_INTR_MASK, I2C Interrupt Mask Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_IntMask_Set(I2CInstance * i2c_instance, uint16_t mask);

uint16_t LL_I2C_IntMask_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_RAW_INTR_STAT, I2C Raw Interrupt Status Register
///////////////////////////////////////////////////////////////////////////////

uint16_t LL_I2C_RawIntStatus(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_RX_TL, I2C Receive FIFO Threshold Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_Rx_Threshold_Set(I2CInstance * i2c_instance, uint8_t rx_tl);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_TX_TL, I2C Transmit FIFO Threshold Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_Tx_Threshold_Set(I2CInstance * i2c_instance, uint8_t tx_tl);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_INTR, Clear Combined and Individual Interrupt Reg
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_All(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_RX_UNDER, Clear RX_UNDER Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_RxUnder(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_RX_OVER, Clear RX_OVER Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_RxOver(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_TX_OVER, Clear TX_OVER Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_TxOver(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_RD_REQ, Clear RD_REQ Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_RdReq(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_TX_ABRT, CLear TX_ABRT Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_TxAbort(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_RX_DONE, Clear RX_DONE Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_RxDone(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_ACTIVITY, Clear ACTIVITY Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_Activity(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_STOP_DET, Clear STOP_DET Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_StopDet(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_START_DET, Clear START_DET Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_StartDet(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_GEN_CALL, Clear GEN_CALL Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_GenCall(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_ENABLE, I2C Enable Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_Enable(I2CInstance * i2c_instance, uint8_t en);

void LL_I2C_Abort(I2CInstance * i2c_instance, uint8_t en);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_STATUS, I2C Status Register
///////////////////////////////////////////////////////////////////////////////

uint32_t LL_I2C_IC_Status(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_TXFLR, I2C Transmit FIFO Level Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_Tx_FIFO_Level(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_RXFLR, I2C Receive FIFO Level Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_Rx_FIFO_Level(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_SDA_HOLD, I2C SDA Hold Time Length Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_SDA_RxHold_Set(I2CInstance * i2c_instance, uint8_t rx_hold);

uint8_t LL_I2C_SDA_RxHold_Get(I2CInstance * i2c_instance);

void LL_I2C_SDA_TxHold_Set(I2CInstance * i2c_instance, uint16_t tx_hold);

uint16_t LL_I2C_Tx_Threshold_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_TX_ABRT_SOURCE, I2C Transmit Abort Source Register
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get Tx abort source, each bit for one source, like abrt_device_slvaddr_noack.
 *
 * @param i2c_instance
 * @return uint32_t
 */
uint32_t LL_I2C_TxAbortSource_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_SLV_DATA_NACK_ONLY, Generate Slave Data NACK Register
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_DMA_CR, DMA Control Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_DMACtrl(I2CInstance * i2c_instance, uint8_t tx_dma_en, uint8_t rx_dma_en);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_DMA_TDLR, DMA Transmit Data Level Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_DMA_DataLevel_Set(I2CInstance * i2c_instance, uint8_t tx_data_level, uint8_t rx_data_level);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_DMA_RDLR, I2C Receive Data Level Register
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_SDA_SETUP, I2C SDA Setup Register
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_ACK_GENERAL_CALL, I2C ACK General Call Register
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_ENABLE_STATUS, I2C Enable Status Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IC_Enable_Status(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_FS_SPKLEN, I2C SS and FS Spike Suppression Limit Reg
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_FastSpeed_SPKLEN_Set(I2CInstance * i2c_instance, uint8_t spklen);

uint8_t LL_I2C_FastSpeed_SPKLEN_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_HS_SPKLEN, I2C HS Spike Suppression Limit Register
///////////////////////////////////////////////////////////////////////////////

void LL_I2C_HighSpeed_SPKLEN_Set(I2CInstance * i2c_instance, uint8_t spklen);

uint8_t LL_I2C_HighSpeed_SPKLEN_Get(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_CLR_RESTART_DET, Clear RESTART_DET Interrupt Register
///////////////////////////////////////////////////////////////////////////////

uint8_t LL_I2C_IntClr_RestartDet(I2CInstance * i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_COMP_PARAM_1, Component Parameter Register 1
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_COMP_VERSION, I2C Component Version Register
///////////////////////////////////////////////////////////////////////////////

uint32_t LL_I2C_CompVersion(I2CInstance *i2c_instance);

///////////////////////////////////////////////////////////////////////////////
////  Register Name: IC_COMP_TYPE, I2C Component Type Register
///////////////////////////////////////////////////////////////////////////////



#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __LL_I2C_H__