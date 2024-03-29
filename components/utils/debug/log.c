#include "ln88xx.h"
#include "proj_config.h"
#include "utils/debug/log.h"
#include "serial.h"


#define LOG_PORT_BAUDRATE  CFG_UART_BAUDRATE_LOG

Serial_t m_LogSerial;
#if (defined(AT_LOG_MERGE_TO_UART0) && (AT_LOG_MERGE_TO_UART0 == 1))
extern Serial_t m_ConsoleSerial;
#endif // !AT_LOG_MERGE_TO_UART0


int log_stdio_write(char *buf, size_t size)
{
    int ret = 0;

    #if WIFI_TRACK
            ret = serial_write(&m_LogSerial, (const void *)buf, size);
    #elif (defined(AT_LOG_MERGE_TO_UART0) && (AT_LOG_MERGE_TO_UART0 == 1))
            ret = serial_write(&m_ConsoleSerial, (const void *)buf, size);
    #else
        // normal mode: log--uart0, at--uart1.
            ret = serial_write(&m_LogSerial, (const void *)buf, size);
    #endif

    return ret;
}


#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')

/**
* dump_hex
* 
* @brief dump data in hex format
* 
* @param buf: User buffer
* @param size: Dump data size
* @param number: The number of outputs per line
* 
* @return void
*/
static void dump_hex(uint8_t level, const uint8_t *buf, uint32_t size, uint32_t number)
{
    int i, j;

    for (i = 0; i < size; i += number)
    {
        LOG(level, "%08X: ", i);

        for (j = 0; j < number; j++)
        {
            if (j % 8 == 0)
            {
                LOG(level, " ");
            }
            if (i + j < size)
                LOG(level, "%02X ", buf[i + j]);
            else
                LOG(level, "   ");
        }
        LOG(level, " ");

        for (j = 0; j < number; j++)
        {
            if (i + j < size)
            {
                LOG(level, "%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        LOG(level, "\r\n");
    }
}

void hexdump(uint8_t level, uint8_t *info, void *buff, uint32_t count)
{
    LOG(level, "%s: ", info);
    dump_hex(level, (const uint8_t *)buff, count, 16);
}

/**
 * @brief It's better to call `at_init()` before `log_init()`.
 */
void log_init(void)
{
    #if WIFI_TRACK
        serial_init(&m_LogSerial, SER_PORT_UART1, LOG_PORT_BAUDRATE, NULL);
    #elif (defined(AT_LOG_MERGE_TO_UART0) && (AT_LOG_MERGE_TO_UART0 == 1))
        // in this mode, no need to reinitial UART0 again.
    #else
        // normal mode.
        serial_init(&m_LogSerial, SER_PORT_UART0, LOG_PORT_BAUDRATE, NULL);//2000000 SER_PORT_UART1
    #endif // !AT_LOG_MERGE_TO_UART0
}
