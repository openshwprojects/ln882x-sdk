#include "ln_types.h"
#include "ln88xx.h"
#include "hal/hal_gpio.h"
#include "hal/simi2c.h"

#define I2C_NOACK   1
#define I2C_ACK     0

///////////////////////////  SCL, SDA Pin Configuration  //////////////////////

// 这两个GPIO模拟I2C是OK的
// 大板子丝印34，对应 GPIOB_11，配置为 SCL
// 大板子丝印33，对应 GPIOB_10，配置为 SDA
// #define SCL_PIN     GPIOB_11
// #define SDA_PIN     GPIOB_10

// 这两个GPIO模拟I2C是OK的
// 大板子丝印22，对应 GPIOB_13，配置为 SCL
// 大板子丝印21，对应 GPIOB_12，配置为 SDA
// #define SCL_PIN     GPIOB_6
// #define SDA_PIN     GPIOB_7

// 这两个GPIO模拟I2C是OK的
#define SCL_PIN     GPIOA_9
#define SDA_PIN     GPIOA_10


// 这两个GPIO模拟I2C是OK的
// 大板子丝印16，对应 GPIOA_20，配置为 SCL
// 大板子丝印15，对应 GPIOA_14，配置为 SDA
// #define SCL_PIN     GPIOA_20
// #define SDA_PIN     GPIOA_14

// 这两个GPIO模拟I2C是OK的, 恰好可以点亮 cloud, identity 两个灯
// 大板子丝印14，对应 GPIOA_13，配置为 SCL
// 大板子丝印13，对应 GPIOA_12，配置为 SDA
// #define SCL_PIN     GPIOA_13
// #define SDA_PIN     GPIOA_12


static void __switch_sda_output(void);
static void __switch_sda_input(void);
static uint8_t __sda_read(void);

#define I2C_SCL_HIGH    HAL_GPIO_WritePin(SCL_PIN, GPIO_VALUE_HIGH)
#define I2C_SCL_LOW     HAL_GPIO_WritePin(SCL_PIN, GPIO_VALUE_LOW)
#define I2C_SDA_HIGH    HAL_GPIO_WritePin(SDA_PIN, GPIO_VALUE_HIGH)
#define I2C_SDA_LOW     HAL_GPIO_WritePin(SDA_PIN, GPIO_VALUE_LOW)
#define I2C_SDA_READ    __sda_read()

/**
 * @brief 当前的配置I2C波特率为 112Kbps
 *
 * @param dly
 */
void I2C_Delay(uint16_t dly)
{
    uint16_t i = 0;
    for (i = dly*15; i > 0; i--)
    {
        __NOP();
    }
}

/**
 * @brief 引脚初始化
 */
void I2C_Init(void)
{
    GPIO_InitTypeDef scl_config;
    GPIO_InitTypeDef sda_config;

    scl_config.dir = GPIO_OUTPUT;

    sda_config.dir = GPIO_OUTPUT;

    HAL_GPIO_Init(SCL_PIN, scl_config);
    HAL_GPIO_Init(SDA_PIN, sda_config);
    I2C_SCL_HIGH;
    I2C_SDA_HIGH;
    I2C_SDA_HIGH;
}

static void __switch_sda_output(void)
{
    GPIO_InitTypeDef sda_config;

    sda_config.dir = GPIO_OUTPUT;

    HAL_GPIO_Init(SDA_PIN, sda_config);
}

static void __switch_sda_input(void)
{
    GPIO_InitTypeDef sda_config;

    sda_config.dir = GPIO_INPUT;
    sda_config.debounce = GPIO_DEBOUNCE_YES;

    HAL_GPIO_Init(SDA_PIN, sda_config);
}

static uint8_t __sda_read(void)
{
    uint8_t value = 0;
    value = HAL_GPIO_ReadPin(SDA_PIN);
    return value;
}

/**
 * @brief 模拟起始信号
 */
void I2C_Start(void)
{
    I2C_SDA_HIGH;
    I2C_SCL_HIGH;
    I2C_Delay(10);

    I2C_Delay(10);
    I2C_SDA_LOW;
    I2C_Delay(10);

    // 钳住I2C总线，准备发送数据
    I2C_SCL_LOW;
    I2C_Delay(10);
}

/**
 * @brief 模拟结束信号
 */
void I2C_Stop(void)
{
    I2C_SCL_LOW;
    I2C_SDA_LOW;
    I2C_Delay(3);

    I2C_SCL_HIGH;
    I2C_Delay(10);

    I2C_SDA_HIGH;
    I2C_Delay(10);
}

// 仅作为 debug 用途，用来统计没有收到ACK的次数
static uint32_t ack_failed = 0;

/**
 * @brief 等待从机应答信号
 *
 * @return uint8_t
 */
uint8_t I2C_GetAck(void)
{
    uint8_t ack;

    I2C_SDA_HIGH;
    __switch_sda_input();
    I2C_Delay(10);

    I2C_SCL_HIGH;
    I2C_Delay(5);

    if (I2C_SDA_READ) {
        ack = I2C_NOACK;
    } else {
        ack = I2C_ACK;
    }
    __switch_sda_output();
    I2C_SCL_LOW;
    I2C_Delay(5);

    if (I2C_NOACK == ack) {
        ack_failed++;
    }
    return ack;
}

/**
 * @brief 主机产生应答信号
 *
 * @param ack 如果值为 I2C_ACK 则发送ACK；否则不回ACK。
 */
void I2C_PutAck(uint8_t ack)
{
    I2C_SCL_LOW;
    I2C_Delay(10);

    if (I2C_ACK == ack) {
        I2C_SDA_LOW;
    } else {
        I2C_SDA_HIGH;
    }
    I2C_Delay(10);

    I2C_SCL_HIGH;
    I2C_Delay(10);
    I2C_SCL_LOW;
    I2C_Delay(2);
}


// SCL 低电平时准备数据，在SCL为高电平时发送出去
void I2C_WriteByte(uint8_t data)
{
    uint8_t cnt = 0;

    for (cnt = 0; cnt < 8; cnt++) {
        I2C_SCL_LOW;
        I2C_Delay(5);

        if (data & 0x80) {
            I2C_SDA_HIGH;
        } else {
            I2C_SDA_LOW;
        }
        data <<= 1;
        I2C_Delay(5);

        I2C_SCL_HIGH;
        I2C_Delay(5);
    }

    I2C_SCL_LOW;
}


uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t cnt = 0;
    uint8_t data = 0;

    I2C_SCL_LOW;
    I2C_Delay(10);

    I2C_SDA_HIGH;

    for( cnt = 0; cnt < 8; cnt++) {
        I2C_SCL_HIGH;
        I2C_Delay(10);

        data <<= 1;
        if (I2C_SDA_READ) {
            data |= 0x01;
        }

        I2C_SCL_LOW;
        I2C_Delay(10);
    }

    return data;
}
