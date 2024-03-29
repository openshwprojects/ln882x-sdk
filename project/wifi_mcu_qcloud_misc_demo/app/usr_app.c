#include "osal/osal.h"
#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"
#include "utils/art_string.h"
#include "utils/system_parameter.h"
#include "wifi/wifi.h"
#include "wifi_manager.h"
#include "lwip/tcpip.h"

#include "SEGGER_SYSVIEW.h"

#include "wifi_setup.h"
#include "ota_mqtt_example.h"

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#define MQTT_SAMPLE             1
#define DATATEMPLATE_SAMPLE     2
#define LIGHT_SCENARY_SAMPLE    3
#define OTA_SAMPLE              4
#define GATEWAY_SAMPLE          5
#define DYN_REG_SAMPLE          6

#define RUN_SAMPLE_TYPE         LIGHT_SCENARY_SAMPLE
// #define RUN_SAMPLE_TYPE         OTA_SAMPLE

#if (MQTT_SAMPLE == RUN_SAMPLE_TYPE)
#define USR_APP_TASK_STACK_SIZE      (1024 * 4) //Byte
#elif (LIGHT_SCENARY_SAMPLE == RUN_SAMPLE_TYPE)
#define USR_APP_TASK_STACK_SIZE      (1024 * 22) //Byte
#else
#define USR_APP_TASK_STACK_SIZE      (1024 * 14) //Byte
#endif

static OS_Thread_t g_usr_app_thread;
extern int mqtt_demo(bool loop_flag);
extern int light_data_template_demo(void);


void usr_app_task_entry(void *params)
{
    IOT_Log_Set_Level(eLOG_DEBUG);

    SEGGER_SYSVIEW_PrintfHost("setup network");

    // setup_wifi_via_airkiss();
    setup_wifi_via_softap(); // OK
    // setup_wifi_via_smartconfig(); // OK
    // setup_wifi_direct(); // OK

    Log_d("Tencent iot explorer example begin, BuildTime: %s %s", __DATE__, __TIME__);
	Log_d("task_stack_used %d, Available Heap %ld", USR_APP_TASK_STACK_SIZE, xPortGetFreeHeapSize());
    if(LIGHT_SCENARY_SAMPLE == RUN_SAMPLE_TYPE) {
        light_data_template_demo();
    } else if(OTA_SAMPLE == RUN_SAMPLE_TYPE) {
        ota_mqtt_example();
    } else if(GATEWAY_SAMPLE == RUN_SAMPLE_TYPE) {
        Log_d("Gateway sample to be ported");
    } else if(DYN_REG_SAMPLE == RUN_SAMPLE_TYPE) {
        Log_d("Dynamic register sample to be ported");
    } else {
        mqtt_demo(true);
    }

    uint32_t idle_cnt = 0;
    while (1) {
        OS_MsDelay(1000);
        LOG(LOG_LVL_INFO, "idle (%u)\r\n", idle_cnt++);
        SEGGER_SYSVIEW_PrintfHost("idle (%u)\r\n", idle_cnt++);
    }
}


void creat_usr_app_task(void)
{
    if(OS_OK != OS_ThreadCreate(&g_usr_app_thread, "UsrAPP", usr_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, USR_APP_TASK_STACK_SIZE)) {
        ART_ASSERT(1);
    }
}
