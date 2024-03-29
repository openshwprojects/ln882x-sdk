#include "proj_config.h"
#include "osal/osal.h"
#include "atcmd/at_task.h"
#include "wifi/wifi.h"
#include "netif/ethernetif.h"
#include "usr_app.h"
#include "wifi_manager.h"
#include "utils/system_parameter.h"
#include "hal/hal_interrupt.h"
#include "utils/debug/CmBacktrace/cm_backtrace.h"
#include "ln_kv_api.h"
#include "ln_nvds.h"
#include "ota_port.h"
#include "ota_types.h"
#include "flash_partition_table.h"


/**
 * This is an example: a piece of memory that has not been initialized after power on
**/
uint8_t test_no_init_data[128] __attribute__((section("no_init_data"),zero_init));

int main (int argc, char* argv[])
{
    //1.sys clock,interrupt
    SetSysClock();
    set_interrupt_priority();
    switch_global_interrupt(true);

    //2. register os heap mem
    OS_DefineHeapRegions();

    //3.rf preprocess,img cal
    wifi_rf_preprocess();
    wifi_rf_image_cal();

    //4.init log&AT
    at_init();
    log_init();

    cm_backtrace_init("wifi_mcu_ota_demo", "HD_V2", "SW_V0.8");
    LOG(LOG_LVL_INFO, "------  wifi_mcu_ota_demo  ------\r\n");


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

    //Creat usr app task.
    creat_usr_app_task();

#if BLE_SUPPORT==ENABLE
    ble_init();
#endif
    OS_ThreadStartScheduler();

    return 0;
}

// ----------------------------------------------------------------------------
