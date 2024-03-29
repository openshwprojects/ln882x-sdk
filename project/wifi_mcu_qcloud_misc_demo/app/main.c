#include "proj_config.h"
#include "osal/osal.h"
#include "atcmd/at_task.h"
#include "wifi/wifi.h"
#include "netif/ethernetif.h"
#include "usr_app.h"
#include "temp_cal_app.h"
#include "wifi_manager.h"
#include "ota_port.h"
#include "utils/system_parameter.h"
#include "hal/hal_interrupt.h"
#include "utils/debug/CmBacktrace/cm_backtrace.h"
#include "ln_kv_api.h"
#include "ln_nvds.h"
#include "flash_partition_table.h"
#include "utils/reboot_trace/reboot_trace.h"
#include "utils/runtime/runtime.h"

#include "SEGGER_SYSVIEW.h"

#if BLE_SUPPORT==ENABLE
#include "ble/ble_task.h"
#include "ble/mg_api.h"
#endif


int main (int argc, char* argv[])
{
    chip_reboot_cause_t reboot_cause = CHIP_REBOOT_POWER_ON;

    //0. check reboot cause
    reboot_cause = ln_chip_get_reboot_cause();

    //1.sys clock,interrupt
    SetSysClock();
    set_interrupt_priority();
    switch_global_interrupt(true);
    ln_runtime_measure_init();

    //2. register os heap mem
    OS_DefineHeapRegions();

    //3.rf preprocess,img cal
    wifi_rf_preprocess();
    wifi_rf_image_cal();

    //4.init log&AT
    at_init();
    log_init();

    cm_backtrace_init("wifi_mcu_qcloud_misc_demo", "HW_V1.0", "SW_V1.0");
    LOG(LOG_LVL_INFO, "------  wifi_mcu_qcloud_misc_demo  ------\r\n");

    SEGGER_SYSVIEW_Conf();

    if (NVDS_ERR_OK != ln_nvds_init(NVDS_SPACE_OFFSET)) {
        LOG(LOG_LVL_ERROR, "NVDS init filed!\r\n");
    }

    if (KV_ERR_NONE != ln_kv_port_init(KV_SPACE_OFFSET, (KV_SPACE_OFFSET + KV_SPACE_SIZE))) {
        LOG(LOG_LVL_ERROR, "KV init filed!\r\n");
    }

	//init system parameter
	system_parameter_init();

    if ( OTA_ERR_NONE != ota_port_init()) {
        LOG(LOG_LVL_ERROR, "ota port failed!\r\n");
    }

    //Init wifi
    wifi_init();

    //Init lwip stack(Creat lwip tack).
    lwip_tcpip_init();

    //Init wifi manager(Creat wifi manager task).
    wifi_manager_init();

    // create temperature calibrate app task.
    creat_temp_cal_app_task();

    //Creat usr app task.
    creat_usr_app_task();

#if BLE_SUPPORT==ENABLE
    ble_init();
#endif
    OS_ThreadStartScheduler();

    return 0;
}

// ----------------------------------------------------------------------------
