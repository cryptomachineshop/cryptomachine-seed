#include "board_init.h"

#include "board_pins.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/vreg.h"
#include "pico/stdlib.h"

namespace cryptomachine::hardware {
namespace {

uint g_backlight_slice = 0;
uint g_backlight_channel = 0;
bool g_backlight_ready = false;

void init_output_pin(unsigned int pin, bool initial_high) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, initial_high ? 1 : 0);
}

}  // namespace

BoardBusRates board_init(std::uint32_t requested_lcd_spi_hz) {
    // Match the Waveshare reference board configuration.
    vreg_set_voltage(VREG_VOLTAGE_1_25);
    sleep_ms(100);

    set_sys_clock_khz(200'000, true);

    clock_configure(
        clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        200'000'000,
        200'000'000
    );

    // LCD control pins.
    init_output_pin(kLcdResetPin, true);
    init_output_pin(kLcdDataCommandPin, false);
    init_output_pin(kLcdChipSelectPin, true);

    // Touch reset is explicitly configured as an output.
    // Waveshare's reference driver toggles this pin without doing so.
    init_output_pin(kTouchResetPin, true);

    // Touch interrupt.
    gpio_init(kTouchInterruptPin);
    gpio_set_dir(kTouchInterruptPin, GPIO_IN);
    gpio_pull_up(kTouchInterruptPin);

    // LCD SPI0.
    const std::uint32_t actual_spi_hz =
        spi_init(spi0, requested_lcd_spi_hz);

    gpio_set_function(kLcdClockPin, GPIO_FUNC_SPI);
    gpio_set_function(kLcdMosiPin, GPIO_FUNC_SPI);

    // Backlight PWM. Start completely off so the panel does not illuminate
    // until display initialization has completed.
    gpio_set_function(kLcdBacklightPin, GPIO_FUNC_PWM);

    g_backlight_slice = pwm_gpio_to_slice_num(kLcdBacklightPin);
    g_backlight_channel = pwm_gpio_to_channel(kLcdBacklightPin);

    pwm_set_wrap(g_backlight_slice, 100);
    pwm_set_chan_level(
        g_backlight_slice,
        g_backlight_channel,
        0
    );
    pwm_set_clkdiv(g_backlight_slice, 50.0f);
    pwm_set_enabled(g_backlight_slice, true);

    g_backlight_ready = true;

    // Shared I2C1 bus used by the FT6336U and other onboard peripherals.
    const std::uint32_t actual_i2c_hz =
        i2c_init(i2c1, kI2cBaudHz);

    gpio_set_function(kI2cSdaPin, GPIO_FUNC_I2C);
    gpio_set_function(kI2cSclPin, GPIO_FUNC_I2C);

    gpio_pull_up(kI2cSdaPin);
    gpio_pull_up(kI2cSclPin);

    return {
        actual_spi_hz,
        actual_i2c_hz,
    };
}

void set_backlight_percent(std::uint8_t percent) {
    if (!g_backlight_ready) {
        return;
    }

    if (percent > 100) {
        percent = 100;
    }

    pwm_set_chan_level(
        g_backlight_slice,
        g_backlight_channel,
        percent
    );
}

}  // namespace cryptomachine::hardware