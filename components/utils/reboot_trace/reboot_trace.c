#include "ln88xx.h"
#include "ln_types.h"
#include "utils/reboot_trace/reboot_trace.h"
#include "hal/hal_syscon.h"
#include "hal/hal_wdt.h"

static uint8_t no_init_data[128] __attribute__((section(".no_init_data"),zero_init));

static int ln_chip_set_reboot_cause(chip_reboot_cause_t cause)
{
    chip_no_init_data_t * pdata = NULL;
    pdata = (chip_no_init_data_t *) no_init_data;

    switch (cause)
    {
        case CHIP_REBOOT_WATCHDOG:
            pdata->reboot_magic = WATCHDOG_BOOT_MAGIC;
            break;
        case CHIP_REBOOT_SOFTWARE:
            pdata->reboot_magic = SOFTWARE_BOOT_MAGIC;
            break;
        default:
            return LN_FALSE;
    }
    
    return LN_TRUE;
}

// 1.just called once. 2.Call it in the first phase of the main function.
chip_reboot_cause_t ln_chip_get_reboot_cause(void)
{
    chip_reboot_cause_t cause = CHIP_REBOOT_POWER_ON;
    chip_no_init_data_t * pdata = NULL;
    pdata = (chip_no_init_data_t *) no_init_data;

    if (pdata->reboot_magic == WATCHDOG_BOOT_MAGIC) {
        cause = CHIP_REBOOT_WATCHDOG;
    }
    else if(pdata->reboot_magic == SOFTWARE_BOOT_MAGIC) {
        cause = CHIP_REBOOT_SOFTWARE;
    }
    else{
        cause = CHIP_REBOOT_POWER_ON;
    }

    ln_chip_set_reboot_cause(CHIP_REBOOT_WATCHDOG);

    return cause;
}

void ln_chip_reboot(void) 
{
    HAL_SYSCON_CPUResetReqMask(0);
    ln_chip_set_reboot_cause(CHIP_REBOOT_SOFTWARE);
    NVIC_SystemReset();
}

void ln_chip_watchdog_keepalive(void)
{
    HAL_WDT_Restart();
}

void WDT_IRQHandler()
{
    //nothing to do.
}

void ln_chip_watchdog_start(watchdog_timeout_level_t level)
{
    WDT_InitTypeDef config;

    config.mode        = WDT_RESET_DIRECT;
    config.pulse_width = WDT_RESET_PULSE_256_PCLK;

    switch (level)
    {
        case WDT_TIMEOUT_LEVEL0:
            config.period = WDT_PERIOD_FF;
            break;
        case WDT_TIMEOUT_LEVEL1:
            config.period = WDT_PERIOD_1FF;
            break;
        case WDT_TIMEOUT_LEVEL2:
            config.period = WDT_PERIOD_3FF;
            break;
        case WDT_TIMEOUT_LEVEL3:
            config.period = WDT_PERIOD_7FF;
            break;
        case WDT_TIMEOUT_LEVEL4:
            config.period = WDT_PERIOD_FFF;
            break;
        case WDT_TIMEOUT_LEVEL5:
            config.period = WDT_PERIOD_1FFF;
            break;
        case WDT_TIMEOUT_LEVEL6:
            config.period = WDT_PERIOD_3FFF;
            break;
        case WDT_TIMEOUT_LEVEL7:
            config.period = WDT_PERIOD_7FFF;
            break;
        case WDT_TIMEOUT_LEVEL8:
            config.period = WDT_PERIOD_FFFF;
            break;
        case WDT_TIMEOUT_LEVEL9:
            config.period = WDT_PERIOD_1FFFF;
            break;
        case WDT_TIMEOUT_LEVEL10:
            config.period = WDT_PERIOD_3FFFF;
            break;
        case WDT_TIMEOUT_LEVEL11:
            config.period = WDT_PERIOD_7FFFF;
            break;
        case WDT_TIMEOUT_LEVEL12:
            config.period = WDT_PERIOD_FFFFF;
            break;
        case WDT_TIMEOUT_LEVEL13:
            config.period = WDT_PERIOD_1FFFFF;
            break;
        case WDT_TIMEOUT_LEVEL14:
            config.period = WDT_PERIOD_3FFFFF;
            break;
        case WDT_TIMEOUT_LEVEL15:
            config.period = WDT_PERIOD_7FFFFF;
            break; 

        default:
            config.period = WDT_PERIOD_1FFFF;
            break;
    }

    HAL_WDT_Init(config);
    HAL_WDT_Enable();
}

