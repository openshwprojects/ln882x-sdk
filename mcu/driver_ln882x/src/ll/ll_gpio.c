#include "ln_types.h"
#include <stdbool.h>
#include "reg_gpio.h"
#include "ll/ll_gpio.h"

/**
 * @brief Controls the type of interrupt that can occur on GPIO ranging from GPIOA_0 to GPIOA_20
 * @param  num: the GPIO_Num to define which GPIO to operate. num can only choose from GPIOA_0 to GPIOA_20
 * @param  irqLvl: it configures the interrupt type to be level-sensitive or edge-sensitive.
 * @return This function has no return.
 */
void LL_GPIO_IrqLevelSet(GPIO_Num num, GPIO_IrqLvl irqLvl)
{
    uint32_t irqLevel = gpio_int_level_getf();

    if (GPIO_IRQLVL_EDGE == irqLvl) {
        irqLevel |= (1 << num);
    } else {
        irqLevel &= ~(1 << num);
    }

    gpio_int_level_setf(irqLevel);
}


/**
 * @brief Controls the polarity of edge or level sensitivity that can occur on GPIO ranging from GPIOA_0 to GPIOA_20
 * @param  num: the GPIO_Num to define which GPIO to operate. num can only choose from GPIOA_0 to GPIOA_20
 * @param  ply: choose from enum GPIO_Polarity to configures the interrupt type to falling-edge or rising-edge.
 * @return This function has no return.
 */
void LL_GPIO_PolaritySet(GPIO_Num num, GPIO_Polarity ply)
{
    uint32_t polarity = gpio_int_polarity_getf();

    if (GPIO_HIGH_RISING == ply) {
        polarity |= (1 << num);
    } else {
        polarity &= ~(1 << num);
    }

    gpio_int_polarity_setf(polarity);
}


/**
 * @brief Set the GPIO to trigger a interrupt at both edge, rising and falling.
 * @param  num: the GPIO_Num to define which GPIO to operate.
 * @param  enable: enable or disable whether a gpio trigger an interrupt at both edge.
 * @return This function has no return.
 */
void LL_GPIO_TrigBothEdge(GPIO_Num num, uint8_t enable)
{
    uint32_t reg = gpio_int_both_edge_getf();
    if (enable) {
        gpio_int_both_edge_setf(reg | (1 << num));
    } else {
        gpio_int_both_edge_setf(reg & (~(1 << num)));
    }
}


/**
 * @brief Set GPIO Direction, Input or Output
 * @param  num: the GPIO_Num to define which GPIO to operate.
 * @param  dir: choose from GPIO_Direction, GPIO_INPUT or GPIO_OUTPUT
 * @return This function has no return.
 */
void LL_GPIO_SetDir(GPIO_Num num, GPIO_Direction dir)
{
    uint32_t dirReg;

    if ( num < GPIO_PWIDTH_A) { // port A
        dirReg = gpio_portadatadirectionregister_getf();
        if (GPIO_OUTPUT == dir) {
            dirReg |= 1 << num;
        } else {
            dirReg &= ~(1<<num);
        }
        gpio_portadatadirectionregister_setf(dirReg);
    } else { // port B
        dirReg = gpio_portbdatadirectionregister_getf();
        if (GPIO_OUTPUT == dir) {
            dirReg |= 1 << (num - GPIO_PWIDTH_A);
        } else {
            dirReg &= ~(1 << (num - GPIO_PWIDTH_A));
        }
        gpio_portbdatadirectionregister_setf(dirReg);
    }
}


/**
 * @brief Get GPIO Direction, Input or Output
 * @param  num: the GPIO_Num to define which GPIO to operate.
 * @return return the direction of the specific gpio
 */
GPIO_Direction LL_GPIO_GetDir(GPIO_Num num)
{
    uint32_t dir;

    if ( num < GPIO_PWIDTH_A ) {
        dir = (gpio_portadatadirectionregister_getf() >> num) & 0x01;
    } else {
        dir = (gpio_portbdatadirectionregister_getf() >> (num - GPIO_PWIDTH_A)) & 0x01;
    }

    return (GPIO_Direction)dir;
}


/**
 * @brief Get signals value on the External Port A.
 *
 * @return uint32_t Only lowest 21 bits are valid, one bit for one gpio.
 */
uint32_t LL_GPIO_ExtPortAValue(void)
{
    return gpio_ext_porta_getf();
}


/**
 * @brief Get signals value on the External Port B.
 *
 * @return uint32_t Only lowest 14 bits are valid, one bit for one gpio.
 */
uint32_t LL_GPIO_ExtPortBValue(void)
{
    return gpio_ext_portb_getf();
}


/**
 * @brief Values written to this register are output on the I/O signals for
 * Port A if the corresponding data direction bits for Port A are set
 * to Output mode and the corresponding control bit for Port A is
 * set to Software mode. The value read back is equal to the last
 * value written to this register.
 *
 * @param value
 */
void LL_GPIO_PORTADataRegWrite(uint32_t value)
{
    gpio_portadataregister_setf(value);
}


/**
 * @brief Values written to this register are output on the I/O signals for
 * Port A if the corresponding data direction bits for Port A are set
 * to Output mode and the corresponding control bit for Port A is
 * set to Software mode. The value read back is equal to the last
 * value written to this register.
 *
 * @return uint32_t
 */
uint32_t LL_GPIO_PORTADataRegRead(void)
{
    return gpio_portadataregister_getf();
}


/**
 * @brief Values written to this register are output on the I/O signals for
 * Port B if the corresponding data direction bits for Port B are set
 * to Output mode and the corresponding control bit for Port B is
 * set to Software mode. The value read back is equal to the last
 * value written to this register.
 *
 * @param value
 */
void LL_GPIO_PORTBDataRegWrite(uint32_t value)
{
    gpio_portbdataregister_setf(value);
}

uint32_t LL_GPIO_PORTBDataRegRead(void)
{
    return gpio_portbdataregister_getf();
}


/**
 * @brief When GPIO direction is input, read current gpio level.
 * @param  num: the GPIO_Num to define which GPIO to operate.
 * @return return the result of current gpio level.
 */
GPIO_Value LL_GPIO_ReadPin(GPIO_Num num)
{
    uint32_t value;

    if ( num < GPIO_PWIDTH_A ) {
        value = (gpio_ext_porta_getf() >> num) & 0x01;
    } else {
        value = (gpio_ext_portb_getf() >> (num - GPIO_PWIDTH_A)) & 0x01;
    }
    return (GPIO_Value)value;
}

/**
 * @brief When GPIO direction is output, write value to set gpio level.
 * @param  num: the GPIO_Num to define which GPIO to operate.
 * @param  value: value can be choose from GPIO_Value, GPIO_VALUE_LOW or GPIO_VALUE_HIGH
 * @return This function has no return.
 */
void LL_GPIO_WritePin(GPIO_Num num, GPIO_Value value)
{
    uint32_t valueReg;

    if ( num < GPIO_PWIDTH_A) { // port A
        valueReg = gpio_portadataregister_getf();
        if ( GPIO_VALUE_LOW == value ) {
            valueReg &= ~(1 << num);
        } else {
            valueReg |= (1 << num);
        }
        gpio_portadataregister_setf(valueReg);
    } else { // port B
        valueReg = gpio_portbdataregister_getf();
        if ( GPIO_VALUE_LOW == value ) {
            valueReg &= ~(1 << (num - GPIO_PWIDTH_A));
        } else {
            valueReg |= (1 << (num - GPIO_PWIDTH_A));
        }
        gpio_portbdataregister_setf(valueReg);
    }
}


/**
 * @brief Toggle a gpio pin
 * @param  num: the GPIO_Num to define which GPIO to operate.
 * @return This function has no return.
 */
void LL_GPIO_TogglePin(GPIO_Num num)
{
    uint32_t value;
    uint32_t result;
    uint32_t valueReg;
    uint32_t move;

    if ( num < GPIO_PWIDTH_A ) {
        value = gpio_ext_porta_getf();
        // NOTE: do not change other bits.
        // remove 1 on the left
        result = ~value;
        move = num;
        result = result << (32 - 1 - move);
        // remove 1 on the right
        result = result >> (32 - 1);
        // go back in position
        result = result << move;
        valueReg = (gpio_portadataregister_getf() & (~(1 << move))) | (result);
        gpio_portadataregister_setf(valueReg);
    } else {
        move = num - GPIO_PWIDTH_A;
        value = gpio_ext_portb_getf();
        result = ~value;
        result = result << (32 - 1 - move);
        result = result >> (32 - 1);
        result = result << move;
        valueReg = ( gpio_portbdataregister_getf() & ( ~(1 << move) ) ) | (result);
        gpio_portbdataregister_setf(valueReg);
        value = gpio_ext_portb_getf();
        value = value;
    }
}

/**
 * @brief Controls whether en external signal that is the source of an interrupt needs to be debounced to remove any spurious glitches.
 *
 * @param num  The GPIO_Num to define which GPIO to operate.
 * @param debounce enable/disable debounce.
 */
void LL_GPIO_SetDebounce(GPIO_Num num, GPIO_Debounce debounce)
{
    uint32_t debReg = gpio_debounceenable_getf();
    if ( GPIO_DEBOUNCE_YES == debounce ) {
        debReg |= 1 << num;
    } else {
        debReg &= ~ (1 << num);
    }
    gpio_debounceenable_setf(debReg);
}


/**
 * @brief Enable the interrupt of specific GPIO
 * @param  num: the GPIO_Num to define which GPIO to operate. n can only choose from GPIOA_0 to GPIOA_20
 * @return This function has no return.
 */
void LL_GPIO_IntEnable(GPIO_Num num)
{
    uint32_t irqEnable = gpio_interruptenable_getf();

    irqEnable |= 1 << num;
    gpio_interruptenable_setf(irqEnable);
}


/**
 * @brief Disable the interrupt of specific GPIO
 * @param  num: the GPIO_Num to define which GPIO to operate. num can only choose from GPIOA_0 to GPIOA_20
 * @return This function has no return.
 */
void LL_GPIO_IntDisable(GPIO_Num num)
{
    uint32_t irqEnableReg = gpio_interruptenable_getf();

    irqEnableReg &= ~(1 << num);
    gpio_interruptenable_setf(irqEnableReg);
}


/**
 * @brief Mask the interrupt of specific GPIO, when the interrupt is masked, no interrupt will trigger to CPU
 * @param  num: the GPIO_Num to define which GPIO to operate. num can only choose from GPIOA_0 to GPIOA_20
 * @return This function has no return.
 */
void LL_GPIO_MaskIrq(GPIO_Num num)
{
    uint32_t mask = gpio_int_mask_getf();
    mask |= 1 << num;
    gpio_int_mask_setf(mask);
}

/**
 * @brief Unmask the interrupt of specific GPIO, when the interrupt is unmasked, the interrupt will trigger to CPU
 * @param  num: the GPIO_Num to define which GPIO to operate. num can only choose from GPIOA_0 to GPIOA_20
 * @return This function has no return.
 */
void LL_GPIO_UnmaskIrq(GPIO_Num num)
{
    uint32_t mask = gpio_int_mask_getf();
    mask &= ~(1 << num);
    gpio_int_mask_setf(mask);
}


/**
 * @brief Interrupt status of PORT A.
 * Only 21 bits are valid, one bit for one GPIO.
 *
 * @return uint32_t
 */
uint32_t LL_GPIO_IntStatus(void)
{
    return gpio_int_status_getf();
}


/**
 * @brief Clear the interrupt of specific gpio
 * @param  num: the GPIO_Num to define which GPIO to operate. num can only choose from GPIOA_0 to GPIOA_20
 * @return This function has no return.
 */
void LL_GPIO_IrqClear(GPIO_Num num)
{
    gpio_gpio_porta_eoi_set(1 << num);
}

