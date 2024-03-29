#include "at_cmd_basic.h"
#include "at_parser.h"
#include "hal/hal_syscon.h"
#include "hal/syscon_types.h"
#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"


char at_at_excute(char *str)
{
    LOG(LOG_LVL_INFO,"AT excute\r\n");
    at_printf("OK\r\n");
    return AT_OK;
}

char at_ate0_excute(char *str)
{
    LOG(LOG_LVL_INFO,"ATE0 excute\r\n");
    console_echo_enable(0);
    at_printf("OK\r\n");
    return AT_OK;
}

char at_ate1_excute(char *str)
{
    LOG(LOG_LVL_INFO,"ATE1 excute\r\n");
    console_echo_enable(1);
    at_printf("OK\r\n");
    return AT_OK;
}

char at_rst_excute(char *str)
{
    LOG(LOG_LVL_INFO,"at_rst excute\r\n");
    HAL_SYSCON_SoftwareResetCore(SW_RST_CORE_ALL);
    return AT_OK;
}

char at_gmr_excute(char *str)
{
    at_printf("OK\r\n");
    return AT_OK;
}

char at_gslp_excute(char *str)
{
    at_printf("[%s, %d]\r\n", __func__, __LINE__);
    return AT_OK;
}

char at_restore_excute(char *str)
{
    at_printf("[%s, %d]\r\n", __func__, __LINE__);
    return AT_OK;
}

char at_sleep_get(char *str)
{
    at_printf("[%s, %d]\r\n", __func__, __LINE__);
    return AT_OK;
}

char at_sleep_set(char *str)
{
    at_printf("[%s, %d]\r\n", __func__, __LINE__);
    return AT_OK;
}


