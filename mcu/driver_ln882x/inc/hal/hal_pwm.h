#ifndef __HAL_PWM_H__
#define __HAL_PWM_H__
#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "ln_types.h"

typedef enum {
    PWM_CH0    = 0,
    PWM_CH1    = 1,
    PWM_CH2    = 2,
    PWM_CH3    = 3,
    PWM_CH4    = 4,
    PWM_CH5    = 5,
    PWM_CH6    = 6,
    PWM_CH7    = 7,
    PWM_CH8    = 8,
    PWM_CH9    = 9,
    PWM_CH10   = 10,
    PWM_CH11   = 11,
    PWM_CH_MAX = 12
} pwm_channel_enum_t;

typedef enum {
    PWM_COUNT_MOD_UP         = 0,
    PWM_COUNT_MOD_DOWN       = 1,
    PWM_COUNT_MOD_UP_DOWN    = 2,
} pwm_count_mod_t;


#define IS_PWM_CHAN(chanX)  ( ( PWM_CH0 <= (chanX) ) && ( (chanX) <= PWM_CH11 ) )

/**
 * @brief Start one PWM channel.
 *
 * @param chan
 */
void HAL_PWM_Start(pwm_channel_enum_t chan);

/**
 * @brief Stop one PWM channel.
 *
 * @param chan
 */
void HAL_PWM_Stop(pwm_channel_enum_t chan);

/**
 * @brief Config frequency and duty cycle of one channel.
 *
 * @param chan
 * @param freq  100Hz <= freq <= 800KHz
 * @param duty 0~1.0
 */
void HAL_PWM_Config(pwm_channel_enum_t chan, uint32_t freq, float duty);

/**
 * @brief Invert output polarity.
 * The default duty cycle is low, after invert, the output is high.
 * @param chan
 * @param en
 */
void HAL_PWM_Invert(pwm_channel_enum_t chan, uint8_t en);

void HAL_PWM_DivSet(pwm_channel_enum_t chan, uint8_t div);

void HAL_PWM_LoadSet(uint8_t pwm_chan_x, uint16_t load);

void HAL_PWM_CompareSet(uint8_t pwm_chan_x, uint16_t cmp);

void HAL_PWM_CntModeSet(uint8_t pwm_chan_x, pwm_count_mod_t mode);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __HAL_PWM_H__
